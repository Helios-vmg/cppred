#include "Maps.h"
#include "CppRed/Data.h"
#include "CppRed/Pokemon.h"
#include "RendererStructs.h"
#include "utility.h"
#include <map>
#include <limits>
#include <sstream>
#include <cassert>

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
void read_pair_list(T (&dst)[N], const byte_t *buffer, size_t &offset, size_t size){
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
	std::map<std::string, const MapData *> map_maps;
	std::map<std::string, ItemId> items_map;

	for (auto &map : this->maps)
		map_maps[map->name] = map.get();
	for (size_t i = 0; i < item_strings_size; i++)
		items_map[item_strings[i].first] = item_strings[i].second;
	auto trainer_parties = this->load_trainer_parties();

	map_objects_t ret;
	size_t offset = 0;
	while (offset < map_objects_data_size){
		auto pair = MapObject::create_vector(map_objects_data, offset, map_objects_data_size, map_maps, graphics_map, items_map, trainer_parties);
		ret[pair.first] = pair.second;
	}
	return ret;
}

MapData::MapData(
		Map map_id,
		const byte_t *buffer,
		size_t &offset,
		size_t size,
		const std::map<std::string, std::shared_ptr<TilesetData>> &tilesets,
		const std::map<std::string, std::shared_ptr<BinaryMapData>> &map_data,
		std::vector<TemporaryMapConnection> &tmcs){
	
	this->map_id = map_id;
	this->name = read_string(buffer, offset, size);
	this->tileset = find_in_constant_map(tilesets, read_string(buffer, offset, size));
	this->width = read_varint(buffer, offset, size);
	this->height = read_varint(buffer, offset, size);
	this->map_data = find_in_constant_map(map_data, read_string(buffer, offset, size));
	for (int i = 0; i < 4; i++){
		TemporaryMapConnection tmc;
		tmc.destination = read_string(buffer, offset, size);
		if (!tmc.destination.size())
			continue;
		tmc.source = this;
		tmc.direction = i;
		tmc.local_pos = read_signed_varint(buffer, offset, size);
		tmc.remote_pos = read_signed_varint(buffer, offset, size);
		tmcs.emplace_back(std::move(tmc));
	}
	this->border_block = read_varint(buffer, offset, size);
}

int MapData::get_block_at_map_position(const Point &point){
	return this->map_data->data[point.x + point.y * this->width];
}

int MapData::get_partial_tile_at_actor_position(const Point &point){
	return this->tileset->blockset->data[this->get_block_at_map_position(point) * 4 + 2];
}

void MapStore::load_maps(const tilesets_t &tilesets, const map_data_t &map_data){
	size_t offset = 0;
	std::vector<TemporaryMapConnection> tmcs;
	int id = 0;
	while (offset < map_definitions_size)
		this->maps.emplace_back(new MapData((Map)++id, map_definitions, offset, map_definitions_size, tilesets, map_data, tmcs));
	
	for (auto &tmc : tmcs){
		auto &mc = tmc.source->map_connections[tmc.direction];
		int found = -1;
		for (size_t i = 0; i < this->maps.size(); i++){
			if (this->maps[i]->name == tmc.destination){
				found = (int)i;
				break;
			}
		}
		if (found < 0)
			throw std::runtime_error("Internal error: Static data error.");
		mc.destination = (Map)(found + 1);
		mc.local_position = tmc.local_pos;
		mc.remote_position = tmc.remote_pos;
	}
}

MapStore::MapStore(){
	auto blocksets = this->load_blocksets();
	auto collisions = this->load_collisions();
	auto graphics_map = this->load_graphics_map();
	auto tilesets = this->load_tilesets(blocksets, collisions, graphics_map);
	auto map_data = this->load_map_data();
	this->load_maps(tilesets, map_data);
	auto objects = this->load_objects(graphics_map);
	this->map_instances.resize(this->maps.size());
}

MapData &MapStore::get_map_data(Map map){
	return *this->maps[(int)map - 1];
}

MapInstance &MapStore::get_map_instance(Map map){
	auto index = (int)map - 1;
	if (!this->map_instances[index])
		this->map_instances[index].reset(new MapInstance(map, *this));
	return *this->map_instances[index];
}

std::pair<std::string, std::shared_ptr<std::vector<std::unique_ptr<MapObject>>>> MapObject::create_vector(
		const byte_t *buffer,
		size_t &offset,
		const size_t size,
		const std::map<std::string, const MapData *> &maps,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::string, ItemId> &items_map,
		const std::map<std::pair<std::string, int>, std::shared_ptr<BaseTrainerParty>> &trainer_map){
	auto ret = std::make_shared<std::vector<std::unique_ptr<MapObject>>>();
	auto name = read_string(buffer, offset, size);
	auto count = read_varint(buffer, offset, size);
	for (decltype(count) i = 0; i < count; i++)
		ret->push_back(create_object(buffer, offset, size, maps, graphics_map, items_map, trainer_map));
	return std::make_pair(name, ret);
}

std::unique_ptr<MapObject> MapObject::create_object(
		const byte_t *buffer,
		size_t &offset,
		const size_t size,
		const std::map<std::string, const MapData *> &maps,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::string, ItemId> &items_map,
		const std::map<std::pair<std::string, int>, std::shared_ptr<BaseTrainerParty>> &trainer_map){

	auto object_type = read_string(buffer, offset, size);

	if (object_type == "event_disp")
		return std::make_unique<EventDisp>(buffer, offset, size);
	if (object_type == "hidden")
		return std::make_unique<HiddenObject>(buffer, offset, size);
	if (object_type == "item")
		return std::make_unique<ItemMapObject>(buffer, offset, size, graphics_map, items_map);
	if (object_type == "npc")
		return std::make_unique<NpcMapObject>(buffer, offset, size, graphics_map);
	if (object_type == "pokemon")
		return std::make_unique<PokemonMapObject>(buffer, offset, size, graphics_map);
	if (object_type == "sign")
		return std::make_unique<Sign>(buffer, offset, size);
	if (object_type == "trainer")
		return std::make_unique<TrainerMapObject>(buffer, offset, size, graphics_map, trainer_map);
	if (object_type == "warp")
		return std::make_unique<MapWarp>(buffer, offset, size, maps);
	
	throw std::runtime_error("Internal error: Static data error.");
}

MapObject::MapObject(const byte_t *buffer, size_t &offset, const size_t size){
	this->name = read_string(buffer, offset, size);
	this->x = read_varint(buffer, offset, size);
	this->y = read_varint(buffer, offset, size);
}

EventDisp::EventDisp(const byte_t *buffer, size_t &offset, const size_t size): MapObject(buffer, offset, size){}

Sign::Sign(const byte_t *buffer, size_t &offset, const size_t size) : MapObject(buffer, offset, size){
	this->text_index = read_varint(buffer, offset, size);
}

HiddenObject::HiddenObject(const byte_t *buffer, size_t &offset, const size_t size) : MapObject(buffer, offset, size){
	this->script = read_string(buffer, offset, size);
	this->script_parameter = read_string(buffer, offset, size);
}

MapWarp::MapWarp(const byte_t *buffer, size_t &offset, const size_t size, const std::map<std::string, const MapData *> &maps): MapObject(buffer, offset, size){
	this->index = read_varint(buffer, offset, size);
	auto temp = read_string(buffer, offset, size);
	if (temp.size() >= 4 && temp[0] == 'v' && temp[1] == 'a' && temp[2] == 'r' && temp[3] == ':')
		this->destination = WarpDestination(temp.substr(4));
	else
		this->destination = WarpDestination(*find_in_constant_map(maps, temp));
	this->destination_warp_index = read_varint(buffer, offset, size);
}

#define CASE_to_MapObjectFacingDirection(x) if (s == #x) return MapObjectFacingDirection::x

MapObjectFacingDirection to_MapObjectFacingDirection(const std::string &s){
	CASE_to_MapObjectFacingDirection(Undefined);
	CASE_to_MapObjectFacingDirection(None);
	CASE_to_MapObjectFacingDirection(Up);
	CASE_to_MapObjectFacingDirection(Right);
	CASE_to_MapObjectFacingDirection(Down);
	CASE_to_MapObjectFacingDirection(Left);
	CASE_to_MapObjectFacingDirection(BoulderMovementByte2);
	throw std::runtime_error("Invalid MapObjectFacingDirection value: " + s);
}

ObjectWithSprite::ObjectWithSprite(
		const byte_t *buffer,
		size_t &offset,
		const size_t size,
		const std::map<std::string, const GraphicsAsset *> &graphics_map): MapObject(buffer, offset, size){
	this->sprite = find_in_constant_map(graphics_map, read_string(buffer, offset, size));
	this->facing_direction = to_MapObjectFacingDirection(read_string(buffer, offset, size));
	this->wandering = !!read_varint(buffer, offset, size);
	this->range = read_varint(buffer, offset, size);
	this->text_index = read_varint(buffer, offset, size);
}

NpcMapObject::NpcMapObject(const byte_t *buffer, size_t &offset, const size_t size,
		const std::map<std::string, const GraphicsAsset *> &graphics_map): ObjectWithSprite(buffer, offset, size, graphics_map){
}

ItemMapObject::ItemMapObject(const byte_t *buffer, size_t &offset, const size_t size,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::string, ItemId> &items_map) : ObjectWithSprite(buffer, offset, size, graphics_map){
	this->item = find_in_constant_map(items_map, read_string(buffer, offset, size));
}

TrainerMapObject::TrainerMapObject(const byte_t *buffer, size_t &offset, const size_t size,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::pair<std::string, int>, std::shared_ptr<BaseTrainerParty>> &parties_map): ObjectWithSprite(buffer, offset, size, graphics_map){
	auto class_name = read_string(buffer, offset, size);
	auto party_index = (int)read_varint(buffer, offset, size);
	this->party = find_in_constant_map(parties_map, std::make_pair(class_name, party_index));
}

PokemonMapObject::PokemonMapObject(const byte_t *buffer, size_t &offset, const size_t size, const std::map<std::string, const GraphicsAsset *> &graphics_map):
		ObjectWithSprite(buffer, offset, size, graphics_map){
	this->species = (SpeciesId)read_varint(buffer, offset, size);
	this->level = (int)read_varint(buffer, offset, size);
}

MapInstance::MapInstance(Map map, MapStore &store): map(map), store(&store){
	auto &map_data = store.get_map_data(map);
	this->w = map_data.width;
	this->h = map_data.height;
	this->occupation_bitmap.resize(this->w * this->h, false);
}

void MapInstance::check_map_location(const Point &p){
	if (p.x < 0 || p.y < 0 || p.x >= this->w || p.y >= this->h){
		auto &map_data = this->store->get_map_data(map);
		std::stringstream stream;
		stream << "Invalid map coordinates: " << p.x << ", " << p.y << ". Name: \"" << map_data.name << "\" with dimensions: " << this->w << "x" << this->h;
		throw std::runtime_error(stream.str());
	}
}

void MapInstance::set_cell_occupation(const Point &p, bool state){
	this->check_map_location(p);
	this->occupation_bitmap[p.x + p.y * this->w] = state;
}

bool MapInstance::get_cell_occupation(const Point &p){
	this->check_map_location(p);
	return this->occupation_bitmap[p.x + p.y * this->w];
}
