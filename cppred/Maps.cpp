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
}

int MapData::get_block_at_map_position(const Point &point){
	return this->map_data->data[point.x + point.y * this->width];
}

int MapData::get_partial_tile_at_actor_position(const Point &point){
	return this->tileset->blockset->data[this->get_block_at_map_position(point) * 4 + 2];
}

MapData &MapStore::get_map_by_name(const std::string &map_name) const{
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
		BufferReader &buffer,
		const std::function<std::unique_ptr<MapObject>(BufferReader &)> &constructor){
	auto ret = std::make_shared<std::vector<std::unique_ptr<MapObject>>>();
	auto name = buffer.read_string();
	auto count = buffer.read_varint();
	ret->reserve(count);
	while (count--)
		ret->push_back(constructor(buffer));
	return {name, ret};
}

MapObject::MapObject(BufferReader &buffer){
	this->name = buffer.read_string();
	this->position.x = buffer.read_varint();
	this->position.y = buffer.read_varint();
}

EventDisp::EventDisp(BufferReader &buffer): MapObject(buffer){}

Sign::Sign(BufferReader &buffer) : MapObject(buffer){
	this->text_index = buffer.read_varint();
}

HiddenObject::HiddenObject(BufferReader &buffer) : MapObject(buffer){
	this->script = buffer.read_string();
	this->script_parameter = buffer.read_string();
}

MapWarp::MapWarp(BufferReader &buffer, const MapStore &map_store): MapObject(buffer){
	this->index = buffer.read_varint();
	auto temp = buffer.read_string();
	if (temp.size() >= 4 && temp[0] == 'v' && temp[1] == 'a' && temp[2] == 'r' && temp[3] == ':')
		this->destination = WarpDestination(temp.substr(4));
	else
		this->destination = WarpDestination(map_store.get_map_by_name(temp));
	this->destination_warp_index = buffer.read_varint();
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

ObjectWithSprite::ObjectWithSprite(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map): MapObject(buffer){
	this->sprite = find_in_constant_map(graphics_map, buffer.read_string());
	this->facing_direction = to_MapObjectFacingDirection(buffer.read_string());
	this->wandering = !!buffer.read_varint();
	this->range = buffer.read_varint();
	this->text_index = buffer.read_varint();
}

NpcMapObject::NpcMapObject(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map): ObjectWithSprite(buffer, graphics_map){
}

ItemMapObject::ItemMapObject(BufferReader &buffer,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::string, ItemId> &items_map) : ObjectWithSprite(buffer, graphics_map){
	this->item = find_in_constant_map(items_map, buffer.read_string());
}

TrainerMapObject::TrainerMapObject(BufferReader &buffer,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::pair<std::string, int>, std::shared_ptr<BaseTrainerParty>> &parties_map): ObjectWithSprite(buffer, graphics_map){
	auto class_name = buffer.read_string();
	auto party_index = (int)buffer.read_varint();
	this->party = find_in_constant_map(parties_map, std::make_pair(class_name, party_index));
}

PokemonMapObject::PokemonMapObject(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map):
		ObjectWithSprite(buffer, graphics_map){
	this->species = (SpeciesId)buffer.read_varint();
	this->level = (int)buffer.read_varint();
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

/*
void MapStore::load_map_objects(){
	std::vector<std::pair<std::string, std::function<std::unique_ptr<MapObject>(BufferReader *)>>> constructors;
	constructors.emplace_back("event_disp", [](BufferReader *buffer){ return std::make_unique<EventDisp>(*buffer); });
	
	BufferReader buffer(map_objects_data, map_objects_data_size);

	typedef decltype(constructors)::value_type T;
	auto begin = constructors.begin();
	auto end = constructors.end();
	while (!buffer.empty()){
		auto map_name = buffer.read_string();
		auto &map = this->get_map_by_name(map_name);
		auto object_count = buffer.read_varint();
		map.objects.reserve(object_count);
		while (object_count--){
			auto object_type = buffer.read_string();
			auto it = find_first_true(begin, end, [&object_type](const T &x){ return object_type <= x.first; });
			if (it == end || it->first != object_type)
				throw std::runtime_error("Unknown map object type: " + object_type);
			map.objects.emplace_back(it->second(&buffer));
		}
	}
}
*/
