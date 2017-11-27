#include "generate_items.h"
#include <iostream>
#include <algorithm>

static const char * const input_file = "input/items.csv";
static const char * const hash_key = "generate_items";
static const char * const date_string = __DATE__ __TIME__;

static void generate_items_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating items.\n";
		return;
	}
	std::cout << "Generating items...\n";

	static const std::vector<std::string> data_order = {
		"id",
		"name",
	};

	std::ofstream header("output/items.h");
	std::ofstream source("output/items.inl");

	header << "#pragma once\n"
		<< generated_file_warning <<
		"\n"
		"enum class ItemId{\n";
	source << generated_file_warning <<
		"\n"
		"extern const std::pair<const char *, ItemId> item_strings[] = {\n";

	CsvParser csv(input_file);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, data_order);
		auto id = to_unsigned(row[0]);
		auto name = row[1];

		header << "    " << name << " = " << id << ",\n";
		source << "\t{ \"" << name << "\", ItemId::" << name << " },\n";
	}
	header << "};\n"
		"extern const std::pair<const char *, ItemId> item_strings[];\n"
		"static const size_t item_strings_size = " << rows << ";\n";
	source << "};\n";

	known_hashes[hash_key] = current_hash;
}

void generate_items(known_hashes_t &known_hashes){
	try{
		generate_items_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_items(): " + e.what());
	}
}
