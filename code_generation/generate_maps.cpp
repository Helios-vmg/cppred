#include "code_generators.h"
#include "../common/csv_parser.h"
#include "utility.h"
#include <string>
#include <stdexcept>
#include <iostream>

static const char * const input_file = "input/maps.csv";
static const char * const hash_key = "generate_maps";

static void generate_maps_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating maps.\n";
		return;
	}
	std::cout << "Generating maps...\n";

	static const std::vector<std::string> maps_order = { "name", "id", "width", "height", };

	std::ofstream header("output/maps.h");
	std::ofstream source("output/maps.cpp");

	header << generated_file_warning << "\n";
	source << generated_file_warning << "\n";

	CsvParser csv(input_file);
	auto rows = csv.row_count();

	header <<
		"extern const MapMetadata map_metadata[" << rows << "];\n"
		"\n"
		"enum class MapId{\n";
	source << "const MapMetadata map_metadata[" << rows << "] = {\n";

	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, maps_order);
		auto name = columns[0];
		auto id = to_unsigned(columns[1]);
		auto width = to_unsigned(columns[2]);
		auto height = to_unsigned(columns[3]);

		header << "    " << name << " = " << id << ",\n";
		source << "    { " << width << ", " << height << " },\n";
	}

	header << "};\n";
	source << "};\n";

	known_hashes[hash_key] = current_hash;
}

void generate_maps(known_hashes_t &known_hashes){
	try{
		generate_maps_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_maps(): " + e.what());
	}
}
