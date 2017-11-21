#include "Maps.h"
#include "CppRed/Data.h"
#include "utility.h"
#include <map>
#include <limits>

Blockset::Blockset(const byte_t *buffer, size_t &offset, size_t size){
	this->name = read_string(buffer, offset, size);
	this->data = read_buffer(buffer, offset, size);
}

Collision::Collision(const byte_t *buffer, size_t &offset, size_t size){
	this->name = read_string(buffer, offset, size);
	this->data = read_buffer(buffer, offset, size);
}

BinaryMapData::BinaryMapData(const byte_t *buffer, size_t &offset, size_t size){
	this->name = read_string(buffer, offset, size);
	this->data = read_buffer(buffer, offset, size);
}

TilesetData::TilesetData(
		const byte_t *buffer,
		size_t &offset,
		size_t size,
		const std::map<std::string, std::shared_ptr<Blockset>> &blocksets,
		const std::map<std::string, std::shared_ptr<Collision>> &collisions,
		const std::map<std::string, const GraphicsAsset *> &graphics_map){

	this->name = read_string(buffer, offset, size);
	this->blockset = find_in_constant_map(blocksets, read_string(buffer, offset, size));
	this->tiles = find_in_constant_map(graphics_map, read_string(buffer, offset, size));
	this->collision = find_in_constant_map(collisions, read_string(buffer, offset, size));
	auto counters = (int)read_varint(buffer, offset, size);
	int i = 0;
	for (; i < counters; i++)
		this->counters[i] = read_varint(buffer, offset, size);
	for (; i < array_length(this->counters); i++)
		this->counters[i] = -1;
	auto temp_grass = read_varint(buffer, offset, size);
	if (temp_grass == std::numeric_limits<std::uint32_t>::max())
		this->grass_tile = -1;
	else
		this->grass_tile = (int)temp_grass;

	auto type = read_string(buffer, offset, size);
	if (type == "indoor")
		this->type = TilesetType::Indoor;
	else if (type == "cave")
		this->type = TilesetType::Cave;
	else if (type == "outdoor")
		this->type = TilesetType::Outdoor;
	else
		throw std::exception();
}

MapStore::blocksets_t MapStore::load_blocksets(){
	blocksets_t ret;
	size_t offset = 0;
	while (offset < blocksets_data_size){
		auto blockset = std::make_shared<Blockset>(blocksets_data, offset, blocksets_data_size);
		ret[blockset->name] = blockset;
	}
	return ret;
}

MapStore::collisions_t MapStore::load_collisions(){
	collisions_t ret;
	size_t offset = 0;
	while (offset < collision_data_size){
		auto collision = std::make_shared<Collision>(collision_data, offset, collision_data_size);
		ret[collision->name] = collision;
	}
	return ret;
}

MapStore::graphics_map_t MapStore::load_graphics_map(){
	graphics_map_t ret;
	for (auto &kv : graphics_assets_map)
		ret[kv.first] = kv.second;
	return ret;
}

MapStore::tilesets_t MapStore::load_tilesets(const blocksets_t &blocksets, const collisions_t &collisions, const graphics_map_t &graphics_map){
	tilesets_t ret;
	size_t offset = 0;
	while (offset < tileset_data_size){
		auto tileset = std::make_shared<TilesetData>(tileset_data, offset, tileset_data_size, blocksets, collisions, graphics_map);
		ret[tileset->name] = tileset;
	}
	return ret;
}

MapStore::map_data_t MapStore::load_map_data(){
	map_data_t ret;
	size_t offset = 0;
	while (offset < map_data_size){
		auto binary_map_data = std::make_shared<BinaryMapData>(::map_data, offset, map_data_size);
		ret[binary_map_data->name] = binary_map_data;
	}
	return ret;
}

MapData::MapData(
		const byte_t *buffer,
		size_t &offset,
		size_t size,
		const std::map<std::string, std::shared_ptr<TilesetData>> &tilesets,
		const std::map<std::string, std::shared_ptr<BinaryMapData>> &map_data){
	
	this->name = read_string(buffer, offset, size);
	this->tileset = find_in_constant_map(tilesets, read_string(buffer, offset, size));
	this->width = read_varint(buffer, offset, size);
	this->height = read_varint(buffer, offset, size);
	this->map_data = find_in_constant_map(map_data, read_string(buffer, offset, size));
}

void MapStore::load_maps(const tilesets_t &tilesets, const map_data_t &map_data){
	size_t offset = 0;
	while (offset < map_definitions_size)
		this->maps.emplace_back(new MapData(map_definitions, offset, map_definitions_size, tilesets, map_data));
}

MapStore::MapStore(){
	auto blocksets = this->load_blocksets();
	auto collisions = this->load_collisions();
	auto graphics_map = this->load_graphics_map();
	auto tilesets = this->load_tilesets(blocksets, collisions, graphics_map);
	auto map_data = this->load_map_data();
	this->load_maps(tilesets, map_data);
}

MapData &MapStore::get_map(Map map){
	return *this->maps[(int)map - 1];
}
