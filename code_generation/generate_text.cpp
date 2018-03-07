#include "generate_text.h"
#include "TextStore.h"
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

static void generate_text_internal(known_hashes_t &known_hashes, TextStore &text_store){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating text.\n";
		return;
	}
	std::cout << "Generating text...\n";

	auto &sections = text_store.get_sections();
	auto &binary_data = text_store.get_binary_data();
	
	std::ofstream text_inl("output/text.inl");
	text_inl << generated_file_warning <<
		"\n"
		"extern const byte_t packed_text_data[] = ";
	write_buffer_to_stream(text_inl, binary_data);
	text_inl << ";\n";

	std::ofstream text_h("output/text.h");
	text_h << "#pragma once\n"
		<< generated_file_warning
		<< "\n"
		"extern const byte_t packed_text_data[];\n"
		"static const size_t packed_text_data_size = "<< binary_data.size() << ";\n"
		"enum class TextResourceId{\n";
	for (auto &kv : text_store.get_text_by_id())
		text_h << "    " << kv.first << " = " << kv.second << ",\n";
	text_h << "};\n";

	known_hashes[hash_key] = current_hash;
}

void generate_text(known_hashes_t &known_hashes, TextStore &text_store){
	try{
		generate_text_internal(known_hashes, text_store);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_text(): " + e.what());
	}
}
