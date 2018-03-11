#include "Maps.h"
#include "CppRed/Data.h"
#include "CppRed/Pokemon.h"
#include "CppRed/Actor.h"
#include "RendererStructs.h"
#include "utility.h"
#include "Objects.h"
#include <map>
#include <limits>
#include <sstream>
#include <cassert>
#include <iostream>

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

template <typename T, size_t N>
void read_pair_list(T(&dst)[N], const byte_t *buffer, size_t &offset, size_t size){
	auto pairs = (int)read_varint(buffer, offset, size);
	int i = 0;
	for (; i < pairs; i++){
		dst[i].first = read_varint(buffer, offset, size);
		dst[i].second = read_varint(buffer, offset, size);
	}
	for (; i < N; i++){
		dst[i].first = -1;
		dst[i].second = -1;
	}
}

TilesetData::TilesetData(
	const byte_t *buffer,
	size_t &offset,
	size_t size,
	const std::map<std::string, std::shared_ptr<Blockset>> &blocksets,
	const std::map<std::string, std::shared_ptr<Collision>> &collisions,
	const std::map<std::string, const GraphicsAsset *> &graphics_map){

	this->id = (TilesetId)read_varint(buffer, offset, size);
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

	read_pair_list(this->impassability_pairs, buffer, offset, size);
	read_pair_list(this->impassability_pairs_water, buffer, offset, size);
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

template <size_t max>
void check_max_party(size_t member_count, const std::string &class_name, size_t party_index){
	if (member_count <= max)
		return;
	std::stringstream stream;
	stream << "Trainer class " << class_name << ", index " << party_index << " contains an invalid number of members (" << member_count << "). Max: " << max;
	throw std::runtime_error(stream.str());
}

MapStore::trainer_parties_t MapStore::load_trainer_parties(){
	trainer_parties_t ret;
	size_t offset = 0;
	auto buffer = trainer_parties_data;
	auto size = trainer_parties_data_size;
	constexpr auto max = CppRed::Party::max_party_size;
	while (offset < size){
		auto class_name = read_string(buffer, offset, size);
		auto party_count = read_varint(buffer, offset, size);
		for (decltype(party_count) i = 0; i < party_count; i++){
			auto party_index = read_varint(buffer, offset, size);
			auto member_count = read_varint(buffer, offset, size);
			check_max_party<max>(member_count, class_name, party_index);
			auto party = allocate_TrainerParty<0, max + 1>(member_count);
			assert(party->get_length() == member_count);
			for (decltype(member_count) j = 0; j < member_count; j++){
				auto &member = party->get_member(j);
				member.species = (SpeciesId)read_varint(buffer, offset, size);
				member.level = (int)read_varint(buffer, offset, size);
			}
			ret[std::make_pair(class_name, party_index)] = party;
		}
	}
	return ret;
}

MapStore::map_objects_t MapStore::load_objects(const graphics_map_t &graphics_map){
	std::map<std::string, ItemId> items_map;

	for (size_t i = 0; i < item_strings_size; i++)
		items_map[item_strings[i].first] = item_strings[i].second;
	auto trainer_map = this->load_trainer_parties();

	std::vector<std::pair<std::string, std::function<std::unique_ptr<MapObject>(BufferReader &)>>> constructors;
	constructors.emplace_back("event_disp", [](BufferReader &buffer){ return std::make_unique<EventDisp>(buffer); });
	constructors.emplace_back("hidden", [](BufferReader &buffer){ return std::make_unique<HiddenObject>(buffer); });
	constructors.emplace_back("item", [&graphics_map, &items_map](BufferReader &buffer){ return std::make_unique<ItemMapObject>(buffer, graphics_map, items_map); });
	constructors.emplace_back("npc", [&graphics_map](BufferReader &buffer){ return std::make_unique<NpcMapObject>(buffer, graphics_map); });
	constructors.emplace_back("pokemon", [&graphics_map](BufferReader &buffer){ return std::make_unique<PokemonMapObject>(buffer, graphics_map); });
	constructors.emplace_back("sign", [](BufferReader &buffer){ return std::make_unique<Sign>(buffer); });
	constructors.emplace_back("trainer", [&graphics_map, &trainer_map](BufferReader &buffer){ return std::make_unique<TrainerMapObject>(buffer, graphics_map, trainer_map); });
	constructors.emplace_back("warp", [this](BufferReader &buffer){ return std::make_unique<MapWarp>(buffer, *this); });

	typedef decltype(constructors)::value_type T;

	std::sort(constructors.begin(), constructors.end(), [](const T &a, const T &b){ return a.first < b.first; });

	auto begin = constructors.begin();
	auto end = constructors.end();

	std::function<std::unique_ptr<MapObject>(BufferReader &)> constructor = [&begin, &end](BufferReader &buffer){
		auto type = buffer.read_string();
		auto it = find_first_true(begin, end, [&type](const T &x){ return type <= x.first; });
		if (it == end || it->first != type)
			throw std::runtime_error("Invalid map object type: " + type);
		return it->second(buffer);
	};

	map_objects_t ret;
	BufferReader buffer(map_objects_data, map_objects_data_size);
	while (!buffer.empty()){
		auto pair = MapObject::create_vector(buffer, constructor);
		ret[pair.first] = pair.second;
	}
	return ret;
}

MapData::MapData(
	Map map_id,
	BufferReader &buffer,
	const std::map<std::string, std::shared_ptr<TilesetData>> &tilesets,
	const std::map<std::string, std::shared_ptr<BinaryMapData>> &map_data,
	std::vector<TemporaryMapConnection> &tmcs,
	std::vector<std::pair<MapData *, std::string>> &map_objects){

	this->map_id = map_id;
	this->name = buffer.read_string();
	this->tileset = find_in_constant_map(tilesets, buffer.read_string());
	this->width = buffer.read_varint();
	this->height = buffer.read_varint();
	this->map_data = find_in_constant_map(map_data, buffer.read_string());
	for (int i = 0; i < 4; i++){
		TemporaryMapConnection tmc;
		tmc.destination = buffer.read_string();
		if (!tmc.destination.size())
			continue;
		tmc.source = this;
		tmc.direction = i;
		tmc.local_pos = buffer.read_signed_varint();
		tmc.remote_pos = buffer.read_signed_varint();
		tmcs.emplace_back(std::move(tmc));
	}
	this->border_block = buffer.read_varint();
	map_objects.emplace_back(this, buffer.read_string());
	this->map_text.reserve(buffer.read_varint());
	while (this->map_text.size() < this->map_text.capacity()){
		auto text = buffer.read_signed_varint();
		auto script = buffer.read_string();
		MapTextEntry entry;
		if (text >= 0){
			entry.simple_text = true;
			entry.text = (TextResourceId)text;
		}else{
			entry.simple_text = false;
			entry.script = script;
		}
		this->map_text.push_back(entry);
	}
	this->warp_check = (int)buffer.read_varint() - 1;
	auto warp_tile_count = buffer.read_varint();
	{
		unsigned i = 0;
		for (; i < warp_tile_count; i++)
			this->warp_tiles[i] = buffer.read_varint();
		for (; i < array_length(this->warp_tiles); i++)
			this->warp_tiles[i] = -1;
	}
}

int MapData::get_block_at_map_position(const Point &point) const{
	return this->map_data->data[point.x + point.y * this->width];
}

int MapData::get_partial_tile_at_actor_position(const Point &point) const{
	return this->tileset->blockset->data[this->get_block_at_map_position(point) * 4 + 2];
}

const MapData &MapStore::get_map_by_name(const std::string &map_name) const{
	typedef decltype(this->maps_by_name)::value_type T;
	auto begin = this->maps_by_name.begin();
	auto end = this->maps_by_name.end();
	auto it = find_first_true(begin, end, [&map_name](const T &a){ return map_name <= a.first; });
	if (it == end || it->first != map_name)
		throw std::runtime_error("Map not found: " + map_name);
	return *it->second;
}

void MapStore::load_maps(const tilesets_t &tilesets, const map_data_t &map_data, const graphics_map_t &graphics_map){
	std::vector<TemporaryMapConnection> tmcs;
	std::vector<std::pair<MapData *, std::string>> object_names;
	int id = 0;
	BufferReader buffer(map_definitions, map_definitions_size);
	while (!buffer.empty()){
		this->maps.emplace_back(new MapData((Map)++id, buffer, tilesets, map_data, tmcs, object_names));
		this->maps_by_name.emplace_back(this->maps.back()->name, this->maps.back().get());
	}
	std::sort(this->maps_by_name.begin(), this->maps_by_name.end());

	for (auto &tmc : tmcs){
		auto &mc = tmc.source->map_connections[tmc.direction];
		mc.destination = this->get_map_by_name(tmc.destination).map_id;
		mc.local_position = tmc.local_pos;
		mc.remote_position = tmc.remote_pos;
	}

	auto objects = this->load_objects(graphics_map);
	for (auto &pair : object_names){
		auto it = objects.find(pair.second);
		if (it == objects.end())
			throw std::runtime_error("Internal error: Map " + pair.first->name + " references non-existing object set " + pair.second);
		for (auto &object : *it->second)
			object->set_map_data(pair.first);
		pair.first->objects = it->second;
	}
}

MapStore::MapStore(){
	auto blocksets = this->load_blocksets();
	auto collisions = this->load_collisions();
	auto graphics_map = this->load_graphics_map();
	auto tilesets = this->load_tilesets(blocksets, collisions, graphics_map);
	auto map_data = this->load_map_data();
	this->load_maps(tilesets, map_data, graphics_map);
	this->map_instances.reserve(this->maps.size());
}

const MapData &MapStore::get_map_data(Map map) const{
	return *this->maps[(int)map - 1];
}

MapInstance &MapStore::get_map_instance(Map map, CppRed::Game &game){
	auto index = (int)map - 1;
	if (index >= this->map_instances.size())
		this->map_instances.resize(index + 1);
	if (!this->map_instances[index])
		this->map_instances[index].reset(new MapInstance(map, *this, game));
	return *this->map_instances[index];
}

const MapInstance &MapStore::get_map_instance(Map map, CppRed::Game &game) const{
	auto index = (int)map - 1;
	if (index >= this->map_instances.size() || !this->map_instances[index])
		throw std::runtime_error("Internal error. Map instance not loaded.");
	return *this->map_instances[index];
}

MapInstance::MapInstance(Map map, MapStore &store, CppRed::Game &game): map(map), store(&store){
	this->data = &store.get_map_data(map);
	this->occupation_bitmap.resize(this->data->width * this->data->height * 2, false);
	this->objects.reserve(this->data->objects->size());
	auto &map_data = this->data->map_data->data;
	auto begin = this->data->warp_tiles;
	auto end = begin + array_length(this->data->warp_tiles); 
	for (auto &object : *this->data->objects)
		this->objects.emplace_back(*object, game);
	for (int y = 0; y < this->data->height; y++){
		for (int x = 0; x < this->data->width; x++){
			Point p(x, y);
			auto tile = this->data->get_partial_tile_at_actor_position(p);
			auto it = std::find(begin, end, tile);
			this->occupation_bitmap[this->get_block_number(p) * 2 + 1] = it != end;
		}
	}
}

void MapInstance::check_map_location(const Point &p) const{
	if (p.x < 0 || p.y < 0 || p.x >= this->data->width || p.y >= this->data->height){
		auto &map_data = this->store->get_map_data(map);
		std::stringstream stream;
		stream << "Invalid map coordinates: " << p.x << ", " << p.y << ". Name: \"" << map_data.name << "\" with dimensions: " << this->data->width << "x" << this->data->height;
		throw std::runtime_error(stream.str());
	}
}

int MapInstance::get_block_number(const Point &p) const{
	return p.x + p.y * this->data->width;
}

void MapInstance::set_cell_occupation(const Point &p, bool state){
	this->check_map_location(p);
	this->occupation_bitmap[this->get_block_number(p) * 2] = state;
}

bool MapInstance::get_cell_occupation(const Point &p) const{
	this->check_map_location(p);
	return this->occupation_bitmap[this->get_block_number(p) * 2];
}

bool MapInstance::is_warp_tile(const Point &p) const{
	if (p.x < 0 || p.y < 0 || p.x >= this->data->width || p.y >= this->data->height)
		return false;
	return this->occupation_bitmap[this->get_block_number(p) * 2 + 1];
}

MapObjectInstance::MapObjectInstance(MapObject &object, CppRed::Game &game): game(&game){
	this->position = object.get_position();
	this->full_object = &object;
}

void MapObjectInstance::activate(CppRed::Actor &activator){
	std::cout << activator.get_name() << " activated " << this->full_object->get_name() << std::endl;
	this->full_object->activate(*this->game, activator);
}

void MapStore::release_map_instance(Map map){
	auto index = (int)map - 1;
	if (index < 0 || index >= this->map_instances.size())
		return;
	this->map_instances[index].reset();
}
