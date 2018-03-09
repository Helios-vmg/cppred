#include "TextStore.h"
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "../common/csv_parser.h"
#include "utility.h"

const std::set<char> apostrophed_letters = { 'd', 'l', 's', 't', 'v', 'r', 'm', };

const unsigned char e_with_acute = 0xE9;

static void write_u32(std::vector<byte_t> &dst, std::uint32_t n){
	for (int i = 4; i--;){
		dst.push_back(n & 0xFF);
		n >>= 8;
	}
}

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
	if (first_four == "cry,"){
		auto species = command.substr(4);
		last_command = CommandType::Cry;
		dst.push_back((byte_t)last_command);
		for (auto c : species)
			dst.push_back(c);
		dst.push_back(0);
		return;
	}
	throw std::runtime_error("Unrecognized command: " + command);
}

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

void TextStore::load_data(){
	if (this->initialized)
		return;
	this->initialized = true;
	std::ifstream input(this->path, std::ios::binary);
	if (!input)
		throw std::runtime_error(this->path + " not found!");
	this->binary_data = parse_text_format(input, this->sections);
	for (auto &kv : sections)
		this->text_by_id.push_back(kv);
	std::sort(this->text_by_id.begin(), this->text_by_id.end(), [](const auto &a, const auto &b){ return a.second < b.second; });
}

const std::vector<byte_t> &TextStore::get_binary_data(){
	this->load_data();
	return this->binary_data;
}

const std::map<std::string, int> &TextStore::get_sections(){
	this->load_data();
	return this->sections;
}

const std::vector<std::pair<std::string, int>> &TextStore::get_text_by_id(){
	this->load_data();
	return this->text_by_id;
}

int TextStore::get_text_id_by_name(const std::string &name){
	this->load_data();
	auto it = this->sections.find(name);
	if (it == this->sections.end())
		throw std::runtime_error("text " + name + " not found");
	return it->second;
}