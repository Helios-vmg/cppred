#include "generate_text.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include <stdexcept>
#include <sstream>
#include <utility>
#include <set>
#include <iomanip>
#include <algorithm>

static const char * const input_file = "input/text.txt";
static const char * const hash_key = "generate_text";
static const char * const date_string = __DATE__ __TIME__;

typedef std::uint8_t byte_t;

const unsigned char e_with_acute = 0xE9;

static void write_u32(std::vector<byte_t> &dst, std::uint32_t n){
	for (int i = 4; i--;){
		dst.push_back(n & 0xFF);
		n >>= 8;
	}
}

enum class CommandType{
	None = 0,
	Text,
	Line,
	Next,
	Cont,
	Para,
	Page,
	Prompt,
	Done,
	Dex,
	Autocont,
	Mem,
	Num,
	End,
};

static void process_command(std::vector<byte_t> &dst, const std::string &command, CommandType &last_command){
	if (command == "cont"){
		last_command = CommandType::Cont;
		dst.push_back((byte_t)last_command);
		return;
	}
	if (command == "para"){
		last_command = CommandType::Para;
		dst.push_back((byte_t)last_command);
		return;
	}
	if (command == "page"){
		last_command = CommandType::Page;
		dst.push_back((byte_t)last_command);
		return;
	}
	if (command == "prompt"){
		last_command = CommandType::Prompt;
		dst.push_back((byte_t)last_command);
		return;
	}
	if (command == "done"){
		last_command = CommandType::Done;
		dst.push_back((byte_t)last_command);
		return;
	}
	if (command == "dex"){
		last_command = CommandType::Dex;
		dst.push_back((byte_t)last_command);
		return;
	}
	if (command == "autocont"){
		last_command = CommandType::Autocont;
		dst.push_back((byte_t)last_command);
		return;
	}
	if (command == "end"){
		last_command = CommandType::End;
		dst.push_back(0);
		return;
	}
	auto first_four = command.substr(0, 4);
	if (first_four == "mem,"){
		auto variable_name = command.substr(4);
		last_command = CommandType::Mem;
		dst.push_back((byte_t)last_command);
		for (auto c : variable_name)
			dst.push_back(c);
		dst.push_back(0);
		return;
	}
	if (first_four == "num,"){
		size_t first_comma = 3;
		size_t second_comma = command.find(',', first_comma + 1);
		if (second_comma == command.npos)
			throw std::runtime_error("Can't parse: " + command);
		first_comma++;
		auto variable_name = command.substr(first_comma, second_comma - first_comma);
		second_comma++;
		auto digits = to_unsigned(command.substr(second_comma));
		last_command = CommandType::Num;
		dst.push_back((byte_t)last_command);
		for (auto c : variable_name)
			dst.push_back(c);
		dst.push_back(0);
		write_u32(dst, digits);
		return;
	}
	throw std::runtime_error("Unrecognized command: " + command);
}

const std::set<char> apostrophed_letters = { 'd', 'l', 's', 't', 'v', 'r', 'm', };

static void set_text_command(std::vector<byte_t> &dst, CommandType &last_command){
	if (/*last_command != CommandType::None &&*/ last_command != CommandType::Text)
		dst.push_back((byte_t)CommandType::Text);
	last_command = CommandType::Text;
}

static std::vector<byte_t> parse_text_format(std::ifstream &stream, std::map<std::string, int> &section_names){
	std::vector<byte_t> ret;
	bool in_section = false;
	CommandType last_command = CommandType::None;
	auto lines = file_splitter(stream);
	for (int line_no = 1; lines.size(); line_no++){
		auto line = move_pop_front(lines);
		if (!in_section){
			if (!line.size())
				continue;
			if (line[0] == '.'){
				auto label_name = line.substr(1);
				if (section_names.find(label_name) != section_names.end())
					throw std::runtime_error("Duplicate label: " + label_name);
				auto id = section_names.size();
				section_names[label_name] = id;
				write_u32(ret, id);
				in_section = true;
				last_command = CommandType::None;
				continue;
			}
			std::stringstream sstream;
			sstream << "Syntax error: line number " << line_no << ": \"" << line << "\"";
			throw std::runtime_error(sstream.str());
		}

		std::string accum;
		bool in_command = false;
		bool apostrophe_seen = false;
		for (char c : line){
			unsigned char uc = c;
			if (in_command){
				if (c == '>'){
					process_command(ret, accum, last_command);
					accum.clear();
					in_command = false;
				}else
					accum.push_back(c);
				continue;
			}
			if (apostrophe_seen){
				apostrophe_seen = false;
				set_text_command(ret, last_command);
				if (apostrophed_letters.find(c) != apostrophed_letters.end()){ 
					ret.push_back((byte_t)c + 128);
					continue;
				}
				ret.push_back('\'');
			}
			if (c == '<'){
				if (last_command == CommandType::Text)
					ret.push_back(0);
				in_command = true;
				continue;
			}
			if ((byte_t)c == e_with_acute){
				set_text_command(ret, last_command);
				ret.push_back((byte_t)'e' + 128);
				continue;
			}
			if (c == '\''){
				apostrophe_seen = true;
				continue;
			}
			set_text_command(ret, last_command);
			ret.push_back(c);
		}
		if (last_command == CommandType::End || last_command == CommandType::Done || last_command == CommandType::Dex){
			in_section = false;
			continue;
		}
		if (last_command == CommandType::Text)
			ret.push_back(0);
		if (!(last_command == CommandType::Cont || last_command == CommandType::Para || last_command == CommandType::Page || last_command == CommandType::Prompt)){
			last_command = CommandType::Line;
			ret.push_back((byte_t)last_command);
		}
	}
	return ret;
}

static void generate_text_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating text.\n";
		return;
	}
	std::cout << "Generating text...\n";

	std::ifstream input(input_file, std::ios::binary);
	if (!input)
		throw std::runtime_error("input/text.txt not found!");
	std::map<std::string, int> sections;
	auto binary_data = parse_text_format(input, sections);
	
	std::ofstream text_inl("output/text.inl");
	text_inl << generated_file_warning <<
		"\n"
		"const byte_t packed_text_data[] = ";
	write_buffer_to_stream(text_inl, binary_data);
	text_inl << ";\n";

	std::ofstream text_h("output/text.h");
	text_h << "#pragma once\n"
		<< generated_file_warning
		<< "\n"
		"extern const byte_t packed_text_data[];\n"
		"static const size_t packed_text_data_size = "<< binary_data.size() << ";\n"
		"enum class TextResourceId{\n";
	{
		std::vector<std::pair<std::string, int>> temp;
		for (auto &kv : sections)
			temp.push_back(kv);
		std::sort(temp.begin(), temp.end(), [](const auto &a, const auto &b){ return a.second < b.second; });
		for (auto &kv : temp)
			text_h << "    " << kv.first << " = " << kv.second << ",\n";
	}
	text_h << "};\n";

	known_hashes[hash_key] = current_hash;
}

void generate_text(known_hashes_t &known_hashes){
	try{
		generate_text_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_text(): " + e.what());
	}
}
