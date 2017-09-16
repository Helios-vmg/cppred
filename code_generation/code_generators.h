#pragma once
#include "../common/csv_parser.h"
#include "utility.h"
#include <functional>

struct BitmapDeclaration{
	std::string type_name;
	std::string class_name;
};

typedef std::vector<BitmapDeclaration> bitmaps_declarations_t;

struct generate_bitmaps_result{
	std::function<bitmaps_declarations_t()> function;
	bool changed;
};

generate_bitmaps_result generate_bitmaps(known_hashes_t &known_hashes);
void generate_rams(known_hashes_t &known_hashes, const generate_bitmaps_result &bitmaps_result);
void generate_charmap(known_hashes_t &known_hashes);
void generate_graphics(known_hashes_t &known_hashes);
void generate_maps(known_hashes_t &known_hashes);
void generate_pokemon_data(known_hashes_t &known_hashes);
void generate_text(known_hashes_t &known_hashes);
