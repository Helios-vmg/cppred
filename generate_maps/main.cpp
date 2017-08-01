#include "../common/csv_parser.h"
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

unsigned to_unsigned(const std::string &s){
	std::stringstream stream(s);
	unsigned ret;
	if (!(stream >> ret))
		throw std::runtime_error("Can't convert \"" + s + "\" to integer.");
	return ret;
}

void generate_maps(){
	const std::vector<std::string> maps_order = { "name", "id", "width", "height", };

	std::ofstream header("output/maps.h");
	std::ofstream source("output/maps.cpp");

	CsvParser csv("input/maps.csv");
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
}

int main(){
	try{
		generate_maps();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}
	return 0;
}
