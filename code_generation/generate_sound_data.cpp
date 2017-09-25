#include "code_generators.h"
#include <iostream>

static const char * const input_file = "input/sound.csv";
static const char * const hash_key = "generate_sound_data";
static const char * const date_string = __DATE__ __TIME__;

static void generate_sound_data_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating sound data.\n";
		return;
	}
	std::cout << "Generating sound data...\n";

	static const std::vector<std::string> data_order = {
		"name",
		"id",
	};

	std::ofstream file("output/sounds.h");

	file << generated_file_warning <<
		"#pragma once\n"
		"\n"
		"enum class SoundId{\n";

	CsvParser csv(input_file);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, data_order);
		auto name = row[0];
		auto id = to_unsigned(row[1]);
		file << "    " << name << " = " << id << ",\n";
	}

	file << "};\n";

	known_hashes[hash_key] = current_hash;
}

void generate_sound_data(known_hashes_t &known_hashes){
	try{
		generate_sound_data_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_sound_data(): " + e.what());
	}
}
