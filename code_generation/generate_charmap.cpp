#include "code_generators.h"
#include "../common/csv_parser.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cstdint>

static const char * const charmap_file = "input/charmap.csv";
static const char * const hash_key = "generate_charmap";
static const char * const date_string = __DATE__ __TIME__;

static std::uint8_t convert_string(const std::string &s){
	std::stringstream stream(s);
	int temp;
	if (!(stream >> temp) || temp < 0 || temp > 0xFF)
		throw std::runtime_error("Can't convert to byte: " + s);
	return (std::uint8_t)temp;
}

void generate_charmap_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(charmap_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating charmap.\n";
		return;
	}
	std::cout << "Generating charmap...\n";

	static const std::vector<std::string> data_order = { "ascii", "mapped" };
	CsvParser csv(charmap_file);
	auto rows = csv.row_count();
	std::vector<std::uint8_t> map(0x100);
	std::vector<std::uint8_t> reverse_map(0x100);
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, data_order);
		auto x = convert_string(row[0]);
		auto y = convert_string(row[1]);
		if (!x || !y)
			continue;
		if (map[x]){
			std::stringstream stream;
			stream << "ASCII value " << (int)x << " is mapped more than once.";
			throw std::runtime_error(stream.str());
		}
		map[x] = y;
		if (reverse_map[y]){
			std::stringstream stream;
			stream << "More than one ASCII value maps to " << (int)y << ".";
			throw std::runtime_error(stream.str());
		}
		reverse_map[y] = x;
	}

	std::ofstream output("output/charmap.inl");
	output << "const byte_t character_map_forward[] = {";
	for (auto i : map)
		output << (int)i << ",";
	output << "};\nconst byte_t character_map_reverse[] = {";
	for (auto i : reverse_map)
		output << (int)i << ",";
	output << "};\n";

	known_hashes[hash_key] = current_hash;
}

void generate_charmap(known_hashes_t &known_hashes){
	try{
		generate_charmap_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_charmap(): " + e.what());
	}
}
