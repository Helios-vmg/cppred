#include "generate_maps.h"
#include "Maps.h"
#include "ReorderedBlockset.h"
#include "../common/csv_parser.h"
#include "../common/base64.h"
#include "utility.h"
#include "Tilesets2.h"
#include "Maps2.h"
#include "TextStore.h"
#include <string>
#include <stdexcept>
#include <iostream>

static const char * const maps_file = "input/maps.csv";
static const char * const tilesets_file = "input/tilesets.csv";
static const char * const map_data_file = "input/map_data.csv";
static const char * const map_data2_file = "input/map_data2.csv";
static const char * const blocksets_file = "input/blocksets.csv";
static const char * const blocksets2_file = "input/blocksets2.csv";
static const char * const collision_file = "input/collision.csv";
static const char * const map_connections_file = "input/map_connections.csv";
static const char * const map_text_file = "input/map_text.csv";
static const char * const text_file = "input/text.txt";
static const std::vector<std::string> input_files = {
	maps_file,
	tilesets_file,
	map_data2_file,
	blocksets2_file,
	graphics_csv_path,
	map_connections_file,
	map_text_file,
	text_file,
};
static const char * const hash_key = "generate_maps";
static const char * const date_string = __DATE__ __TIME__;

static std::shared_ptr<std::vector<byte_t>> serialize_blocksets(const std::vector<Block> &blockset){
	auto ret = std::make_shared<std::vector<byte_t>>();
	ret->reserve(blockset.size() * Block::size);
	for (auto &block : blockset)
		for (size_t i = 0; i < Block::size; i++)
			ret->push_back(block.tiles[i]);
	return ret;
}

static std::map<std::string, std::shared_ptr<std::vector<byte_t>>> serialize_blocksets(const std::map<std::string, std::shared_ptr<std::vector<Block>>> &blocksets){
	std::map<std::string, std::shared_ptr<std::vector<byte_t>>> ret;
	for (auto &kv : blocksets)
		ret[kv.first] = serialize_blocksets(*kv.second);
	return ret;
}

template <typename T>
void write_blocksets(std::ostream &header, std::ostream &source, const T &blocksets){
	std::vector<byte_t> blocksets_data;
	for (auto &kv : blocksets){
		auto m = kv.second->size();
		if (!m)
			continue;
		write_ascii_string(blocksets_data, kv.first);
		write_varint(blocksets_data, m);
		auto n = blocksets_data.size();
		blocksets_data.resize(n + m);
		memcpy(&blocksets_data[n], &(*kv.second)[0], m);
	}
	write_buffer_to_header_and_source(header, source, blocksets_data, "blocksets_data");
}

template <typename T>
void write_collision(std::ostream &header, std::ostream &source, const T &collision){
	std::vector<byte_t> collision_data;
	for (auto &kv : collision){
		auto m = kv.second->size();
		if (!m)
			continue;
		write_ascii_string(collision_data, kv.first);
		write_varint(collision_data, m);
		auto n = collision_data.size();
		collision_data.resize(n + m);
		memcpy(&collision_data[n], &(*kv.second)[0], m);
	}
	write_buffer_to_header_and_source(header, source, collision_data, "collision_data");
}

static void write_tilesets(std::ostream &header, std::ostream &source, const Tilesets2 &tilesets){
	std::vector<byte_t> tileset_data;

	for (auto &tileset : tilesets.get_tilesets())
		tileset->serialize(tileset_data);

	write_buffer_to_header_and_source(header, source, tileset_data, "tileset_data");

	header << "enum class TilesetId{\n";
	for (auto &tileset : tilesets.get_tilesets())
		header << "\t" << tileset->get_name() << " = " << tileset->get_id() << ",\n";
	header << "};\n";
}

template <typename T>
void write_map_data(std::ostream &header, std::ostream &source, const T &maps){
	std::vector<byte_t> map_data;
	for (auto &kv : maps){
		auto m = kv.second->size();
		if (!m)
			continue;
		write_ascii_string(map_data, kv.first);
		write_varint(map_data, m);
		auto n = map_data.size();
		map_data.resize(n + m);
		memcpy(&map_data[n], &(*kv.second)[0], m);
	}

	write_buffer_to_header_and_source(header, source, map_data, "map_data");
}

static void write_maps(std::ostream &header, std::ostream &source, const Maps2 &maps){
	std::vector<byte_t> map_definitions;
	
	header << "enum class Map{\n"
		"\tNowhere = 0,\n";
	int index = 1;
	for (auto &map : maps.get_maps()){
		header << "\t" << map->get_name() << " = " << index++ << ",\n";
		map->serialize(map_definitions);
	}
	header << "};\n";
	write_buffer_to_header_and_source(header, source, map_definitions, "map_definitions");
}

static std::map<std::string, std::vector<MapTextEntry>> read_string_map(const char *path){
	static const std::vector<std::string> order = { "map_name", "text", "script", };
	
	CsvParser csv(path);
	auto rows = csv.row_count();

	std::map<std::string, std::vector<MapTextEntry>> ret;
	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
		MapTextEntry entry;
		entry.text = columns[1];
		entry.script = columns[2];
		ret[columns[0]].push_back(entry);
	}
	return ret;
}

static void generate_maps_internal(known_hashes_t &known_hashes, GraphicsStore &gs, TextStore &text_store){
	auto current_hash = hash_files(input_files, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating maps.\n";
		return;
	}
	std::cout << "Generating maps...\n";

	auto blocksets = read_data_csv(blocksets2_file);
	auto collision = read_data_csv(collision_file);
	for (auto &kv : collision)
		std::sort(kv.second->begin(), kv.second->end());
	auto map_data = read_data_csv(map_data2_file);
	auto map_text = read_string_map(map_text_file);
	
	Tilesets2 tilesets2(tilesets_file, blocksets, collision, gs);
	Maps2 maps2(maps_file, map_data, tilesets2);
	maps2.load_map_connections(map_connections_file);
	maps2.load_map_text(map_text, text_store);

	//Do consistency check.
	for (auto &map : maps2.get_maps())
		map->render_to_file();

	std::ofstream header("output/maps.h");
	std::ofstream source("output/maps.inl");
	
	header << generated_file_warning <<
		"\n"
		"#pragma once\n"
		"#include <utility>\n"
		"struct TilesetData;\n"
		"struct MapData;\n"
	;
	source << generated_file_warning <<
		"\n"
		"#include \"maps.h\"\n"
	;

	write_blocksets(header, source, blocksets);
	write_collision(header, source, collision);
	write_tilesets(header, source, tilesets2);
	write_map_data(header, source, map_data);
	write_maps(header, source, maps2);

	known_hashes[hash_key] = current_hash;
}

void generate_maps(known_hashes_t &known_hashes, GraphicsStore &gs, TextStore &text_store){
	try{
		generate_maps_internal(known_hashes, gs, text_store);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_maps(): " + e.what());
	}
}
