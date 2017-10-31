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
