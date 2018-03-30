#include "generate_items.h"
#include <iostream>
#include <algorithm>

static const char * const items_file = "input/items.csv";
static const char * const hash_key = "generate_items";
static const char * const date_string = __DATE__ __TIME__;

static void generate_items_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(items_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating items.\n";
		return;
	}
	std::cout << "Generating items...\n";

	static const std::vector<std::string> data_order = {
		"id",
		"name",
		"display_name",
		"price",
		"is_key",
		"use_function",
	};

	std::ofstream header("output/items.h");
	std::ofstream source("output/items.inl");

	header << 
		generated_file_warning <<
		"#pragma once\n"
		"\n"
		"#include <CppRed/ItemData.h>\n"
		"\n"
		"enum class ItemId{\n";
	
	source <<
		generated_file_warning <<
		"\n"
		"#include <CppRed/ItemFunctions.h>\n"
		"\n"
		"extern const CppRed::ItemData item_data[] = {\n";

	CsvParser csv(items_file);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		try{
			auto row = csv.get_ordered_row(i, data_order);
			auto id = to_unsigned(row[0]);
			auto name = row[1];
			auto display_name = row[2];
			auto price = to_unsigned(row[3]);
			auto is_key = to_bool(row[4]);
			auto use_function = row[5];

			header << "    " << name << " = " << id << ",\n";
			source <<
				"    {"
				" ItemId::" << name << ", ";
			if (display_name.size()){
				source <<
					"\"" << name << "\", "
					"\"" << filter_text(display_name) << "\", ";
			}else
				source << "nullptr, nullptr, ";
			
			source <<
				price << ", " <<
				bool_to_string(is_key) << ", ";
			if (use_function.size())
				source << "CppRed::" << use_function;
			else
				source << "nullptr";
			source << ", },\n";
		}catch (std::exception &e){
			std::stringstream stream;
			stream << "Error while parsing CSV row " << i + 1 << ": " << e.what();
			throw std::runtime_error(stream.str());
		}
	}

	header <<
		"};\n"
		"extern const CppRed::ItemData item_data[" << rows << "];\n"
		"static const size_t item_data_size = " << rows << ";\n";
	
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
