#include "generate_maps.h"
#include "Blocksets.h"
#include "Maps.h"
#include "../common/csv_parser.h"
#include "../common/base64.h"
#include "utility.h"
#include <string>
#include <stdexcept>
#include <iostream>

static const char * const maps_file = "input/maps.csv";
static const char * const tilesets_file = "input/tilesets.csv";
static const char * const map_data_file = "input/map_data.csv";
static const char * const blocksets_file = "input/blocksets.csv";
static const std::vector<std::string> input_files = {
	maps_file,
	tilesets_file,
	map_data_file,
	graphics_csv_path,
};
static const char * const hash_key = "generate_maps";
static const char * const date_string = __DATE__ __TIME__;

static void generate_maps_internal(known_hashes_t &known_hashes, GraphicsStore &gs){
	auto current_hash = hash_files(input_files, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating maps.\n";
		return;
	}
	std::cout << "Generating maps...\n";

	Blocksets blocksets(blocksets_file);
	Tilesets tilesets(tilesets_file, blocksets, gs);
	Maps maps(maps_file, map_data_file, tilesets);

	for (auto &map : maps.get_maps()){
		std::string path1 = map->get_name();
		std::string path2 = path1;
		path1 += ".png";
		path2 += ".bin";
		map->render_to_file(path1.c_str(), path2.c_str());
	}

	//known_hashes[hash_key] = current_hash;
}

void generate_maps(known_hashes_t &known_hashes, GraphicsStore &gs){
	try{
		generate_maps_internal(known_hashes, gs);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_maps(): " + e.what());
	}
}
