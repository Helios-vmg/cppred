#include "generate_maps.h"
#include "Maps.h"
#include "ReorderedBlockset.h"
#include "../common/csv_parser.h"
#include "../common/base64.h"
#include "utility.h"
#include <string>
#include <stdexcept>
#include <iostream>
#include "Tilesets2.h"
#include "Maps2.h"

static const char * const maps_file = "input/maps.csv";
static const char * const tilesets_file = "input/tilesets.csv";
static const char * const map_data_file = "input/map_data.csv";
static const char * const map_data2_file = "input/map_data2.csv";
static const char * const blocksets_file = "input/blocksets.csv";
static const char * const blocksets2_file = "input/blocksets2.csv";
static const char * const collision_file = "input/collision.csv";
static const std::vector<std::string> input_files = {
	maps_file,
	tilesets_file,
	map_data2_file,
	blocksets2_file,
	graphics_csv_path,
};
static const char * const hash_key = "generate_maps";
static const char * const date_string = __DATE__ __TIME__;

std::shared_ptr<std::vector<byte_t>> serialize_blocksets(const std::vector<Block> &blockset){
	auto ret = std::make_shared<std::vector<byte_t>>();
	ret->reserve(blockset.size() * Block::size);
	for (auto &block : blockset)
		for (size_t i = 0; i < Block::size; i++)
			ret->push_back(block.tiles[i]);
	return ret;
}

std::map<std::string, std::shared_ptr<std::vector<byte_t>>> serialize_blocksets(const std::map<std::string, std::shared_ptr<std::vector<Block>>> &blocksets){
	std::map<std::string, std::shared_ptr<std::vector<byte_t>>> ret;
	for (auto &kv : blocksets)
		ret[kv.first] = serialize_blocksets(*kv.second);
	return ret;
}

template <typename T>
void write_blocksets(std::ostream &header, std::ostream &source, const T &blocksets){
	std::vector<byte_t> blocksets_data;
	std::map<std::string, std::pair<size_t, size_t>> blocksets_data_offsets;
	for (auto &kv : blocksets){
		auto n = blocksets_data.size();
		auto m = kv.second->size();
		blocksets_data_offsets[kv.first] = { n, m };
		if (!kv.second->size())
			continue;
		blocksets_data.resize(n + m);
		memcpy(&blocksets_data[n], &(*kv.second)[0], m);
	}
	header << "namespace Blocksets{\n"
		"extern const byte_t data[" << blocksets_data.size() << "];\n"
		"typedef std::pair<const byte_t *, size_t> pair_t;\n";
	source << "namespace Blocksets{\n"
		"extern const byte_t data[" << blocksets_data.size() << "] = ";
	write_buffer_to_stream(source, blocksets_data);
	source << ";\n";
	for (auto &kv : blocksets_data_offsets){
		auto s = "const pair_t " + kv.first;
		header << "extern " << s << ";\n";
		source << s << "(data + " << kv.second.first << ", " << kv.second.second << ");\n";
	}
	header << "}\n\n";
	source << "}\n\n";
}

template <typename T>
void write_collision(std::ostream &header, std::ostream &source, const T &collision){
	std::vector<byte_t> collision_data;
	std::map<std::string, std::pair<size_t, size_t>> collision_data_offsets;
	for (auto &kv : collision){
		auto n = collision_data.size();
		auto m = kv.second->size();
		collision_data_offsets[kv.first] = { n, m };
		if (!kv.second->size())
			continue;
		collision_data.resize(n + m);
		memcpy(&collision_data[n], &(*kv.second)[0], m);
	}
	header << "namespace Collision{\n"
		"extern const byte_t data[" << collision_data.size() << "];\n"
		"typedef std::pair<const byte_t *, size_t> pair_t;\n";
	source << "namespace Collision{\n"
		"extern const byte_t data[" << collision_data.size() << "] = ";
	write_buffer_to_stream(source, collision_data);
	source << ";\n";
	for (auto &kv : collision_data_offsets){
		auto s = "const pair_t " + kv.first;
		header << "extern " << s << ";\n";
		source << s << "(data + " << kv.second.first << ", " << kv.second.second << ");\n";
	}
	header << "}\n\n";
	source << "}\n\n";
}

const char *to_string(TilesetType type){
	switch (type){
		case TilesetType::Indoor:
			return "TilesetType::Indoor";
		case TilesetType::Cave:
			return "TilesetType::Cave";
		case TilesetType::Outdoor:
			return "TilesetType::Outdoor";
		default:
			throw std::runtime_error("Internal error in to_string(TilesetType).");
	}
}

void write_tilesets(std::ostream &header, std::ostream &source, const Tilesets2 &tilesets){
	header << "namespace Tilesets{\n";
	source << "namespace Tilesets{\n";
	for (auto &tileset : tilesets.get_tilesets()){
		header << "extern const TilesetData " << tileset->get_name() << ";\n";
		source << "const TilesetData " << tileset->get_name() << " = { \""
			<< tileset->get_name() << "\", Blockset::" << tileset->get_blockset_name() << ", "
			<< tileset->get_tiles().name << ", Collision::" << tileset->get_collision_name() << ", { ";
		for (auto i : tileset->get_counters())
			source << i << ", ";
		source << "}, " << tileset->get_grass() << ", " << to_string(tileset->get_type()) << "};\n";
	}
	header << "}\n\n";
	source << "}\n\n";
}

template <typename T>
void write_map_data(std::ostream &header, std::ostream &source, const T &maps){
	std::vector<byte_t> map_data;
	std::map<std::string, std::pair<size_t, size_t>> collision_data_offsets;
	for (auto &kv : maps){
		auto n = map_data.size();
		auto m = kv.second->size();
		collision_data_offsets[kv.first] = { n, m };
		if (!kv.second->size())
			continue;
		map_data.resize(n + m);
		memcpy(&map_data[n], &(*kv.second)[0], m);
	}
	header << "namespace BinaryMapData{\n"
		"extern const byte_t data[" << map_data.size() << "];\n"
		"typedef std::pair<const byte_t *, size_t> pair_t;\n";
	source << "namespace BinaryMapData{\n"
		"const byte_t data[" << map_data.size() << "] = ";
	write_buffer_to_stream(source, map_data);
	source << ";\n";
	for (auto &kv : collision_data_offsets){
		auto s = "const pair_t " + kv.first;
		header << "extern " << s << ";\n";
		source << s << "(data + " << kv.second.first << ", " << kv.second.second << ");\n";
	}
	header << "}\n\n";
	source << "}\n\n";
}

void write_maps(std::ostream &header, std::ostream &source, const Maps2 &maps){
	header << "namespace Maps{\n";
	source << "namespace Maps{\n";
	for (auto &map : maps.get_maps()){
		header << "extern const MapData " << map->get_name() << ";\n";
		source << "const MapData " << map->get_name() << " = { \""
			<< map->get_name() << "\", Tilesets::" << map->get_tileset().get_name() << ", "
			<< map->get_width() << ", " << map->get_height() << ", BinaryMapData::" << map->get_map_data_name() << " };\n";
	}
	header << "}\n\n";
	source << "}\n\n";
}

static void generate_maps_internal(known_hashes_t &known_hashes, GraphicsStore &gs){
	auto current_hash = hash_files(input_files, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating maps.\n";
		return;
	}
	std::cout << "Generating maps...\n";

	auto blocksets = read_data_csv(blocksets2_file);
	auto collision = read_data_csv(collision_file);
	auto map_data = read_data_csv(map_data2_file);
	
	Tilesets2 tilesets2(tilesets_file, blocksets, collision, gs);
	Maps2 maps2(maps_file, map_data, tilesets2);

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

void generate_maps(known_hashes_t &known_hashes, GraphicsStore &gs){
	try{
		generate_maps_internal(known_hashes, gs);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_maps(): " + e.what());
	}
}
