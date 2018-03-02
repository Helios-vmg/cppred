#pragma once

#include "common_types.h"
#include "GraphicsAsset.h"
#include "../CodeGeneration/output/maps.h"
#include "../CodeGeneration/output/items.h"
#include "../common/TilesetType.h"
#include "TrainerData.h"
#include "utility.h"
#include <vector>
#include <memory>
#include <map>

struct Blockset{
	std::string name;
	std::vector<byte_t> data;

	Blockset(const byte_t *, size_t &, size_t);
};

struct Collision{
	std::string name;
	std::vector<byte_t> data;

	Collision(const byte_t *, size_t &, size_t);
};

enum class MapObjectFacingDirection{
	Undefined,
	None,
	Up,
	Right,
	Down,
	Left,
	BoulderMovementByte2,
};

struct TilesetData{
	std::string name;
	std::shared_ptr<Blockset> blockset;
	const GraphicsAsset *tiles;
	std::shared_ptr<Collision> collision;
	short counters[16];
	int grass_tile;
	TilesetType type;
	std::pair<short, short> impassability_pairs[16];
	std::pair<short, short> impassability_pairs_water[16];

	TilesetData(
		const byte_t *,
		size_t &,
		size_t,
		const std::map<std::string, std::shared_ptr<Blockset>> &,
		const std::map<std::string, std::shared_ptr<Collision>> &,
		const std::map<std::string, const GraphicsAsset *> &
	);
};

class MapStore;

class MapObject{
protected:
	std::string name;
	Point position = {-1, -1};

	MapObject(BufferReader &);
public:
	MapObject(const std::string &name, const Point &position): name(name), position(position){}
	virtual ~MapObject() = 0;
	static std::pair<std::string, std::shared_ptr<std::vector<std::unique_ptr<MapObject>>>> create_vector(BufferReader &buffer, const std::function<std::unique_ptr<MapObject>(BufferReader &)> &);
	virtual const char *get_type_string() const = 0;
	DEFINE_GETTER(position)
	DEFINE_GETTER(name)
};

inline MapObject::~MapObject(){}

class EventDisp : public MapObject{
public:
	EventDisp(BufferReader &);
	EventDisp(const std::string &name, const Point &position): MapObject(name, position){}
	const char *get_type_string() const override{
		return "event_disp";
	}
};

class Sign : public MapObject{
protected:
	int text_index;
public:
	Sign(BufferReader &);
	Sign(const std::string &name, const Point &position, int text_index): MapObject(name, position), text_index(text_index){}
	const char *get_type_string() const override{
		return "sign";
	}
};

class HiddenObject : public MapObject{
protected:
	std::string script;
	std::string script_parameter;

public:
	HiddenObject(BufferReader &);
	HiddenObject(const std::string &name, const Point &position, const std::string &script, const std::string &script_parameter):
		MapObject(name, position),
		script(script),
		script_parameter(script_parameter){}
	const char *get_type_string() const override{
		return "hidden_object";
	}
};

struct MapData;

struct WarpDestination{
	bool simple;
	const MapData *destination_map = nullptr;
	std::string variable_name;
	WarpDestination() = default;
	WarpDestination(const MapData &destination_map): simple(true), destination_map(&destination_map){}
	WarpDestination(const std::string &variable_name): simple(false), variable_name(variable_name){}
};

class MapWarp : public MapObject{
protected:
	int index;
	WarpDestination destination;
	int destination_warp_index;

public:
	MapWarp(BufferReader &, const MapStore &);
	MapWarp(const std::string &name, const Point &position, int index, const WarpDestination &destination, int destination_warp_index):
		MapObject(name, position),
		index(index),
		destination(destination),
		destination_warp_index(destination_warp_index){}
	const char *get_type_string() const override{
		return "map_warp";
	}
};

class ObjectWithSprite : public MapObject{
protected:
	const GraphicsAsset *sprite;
	MapObjectFacingDirection facing_direction;
	bool wandering;
	int range;
	int text_index;

public:
	ObjectWithSprite(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map);
	ObjectWithSprite(const std::string &name, const Point &position, const GraphicsAsset &sprite, MapObjectFacingDirection facing_direction, bool wandering, int range, int text_index):
		MapObject(name, position),
		sprite(&sprite),
		facing_direction(facing_direction),
		wandering(wandering),
		range(range),
		text_index(text_index){}
	virtual ~ObjectWithSprite() = 0;
};

inline ObjectWithSprite::~ObjectWithSprite(){}

class NpcMapObject : public ObjectWithSprite{
public:
	NpcMapObject(BufferReader &, const std::map<std::string, const GraphicsAsset *> &graphics_map);
	NpcMapObject(const char *name, const Point &position, const GraphicsAsset &sprite, MapObjectFacingDirection facing_direction, bool wandering, int range, int text_id):
		ObjectWithSprite(name, position, sprite, facing_direction, wandering, range, text_id){}
	const char *get_type_string() const override{
		return "npc";
	}
};

class ItemMapObject : public ObjectWithSprite{
protected:
	ItemId item;

public:
	ItemMapObject(BufferReader &,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::string, ItemId> &items_map);
	ItemMapObject(const char *name, const Point &position, const GraphicsAsset &sprite, MapObjectFacingDirection facing_direction, bool wandering, int range, int text_id, ItemId item):
		ObjectWithSprite(name, position, sprite, facing_direction, wandering, range, text_id),
		item(item){}
	const char *get_type_string() const override{
		return "item";
	}
};

class TrainerMapObject : public ObjectWithSprite{
protected:
	std::shared_ptr<BaseTrainerParty> party;

public:
	TrainerMapObject(BufferReader &,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::pair<std::string, int>, std::shared_ptr<BaseTrainerParty>> &parties_map);
	TrainerMapObject(const char *name, const Point &position, const GraphicsAsset &sprite, MapObjectFacingDirection facing_direction, bool wandering, int range, int text_id, const std::shared_ptr<BaseTrainerParty> &party):
		ObjectWithSprite(name, position, sprite, facing_direction, wandering, range, text_id),
		party(party){}
	const char *get_type_string() const override{
		return "trainer";
	}
};

class PokemonMapObject : public ObjectWithSprite{
protected:
	SpeciesId species;
	int level;

public:
	PokemonMapObject(BufferReader &, const std::map<std::string, const GraphicsAsset *> &graphics_map);
	PokemonMapObject(const char *name, const Point &position, const GraphicsAsset &sprite, MapObjectFacingDirection facing_direction, bool wandering, int range, int text_id, SpeciesId species, int level):
		ObjectWithSprite(name, position, sprite, facing_direction, wandering, range, text_id),
		species(species),
		level(level){}
	const char *get_type_string() const override{
		return "pokemon";
	}
};

struct BinaryMapData{
	std::string name;
	std::vector<byte_t> data;

	BinaryMapData(const byte_t *, size_t &, size_t);
};

struct Point;

struct TemporaryMapConnection{
	MapData *source;
	std::string destination;
	int direction;
	int local_pos, remote_pos;
	TemporaryMapConnection() = default;
	TemporaryMapConnection(const TemporaryMapConnection &) = delete;
	TemporaryMapConnection(TemporaryMapConnection &&other): destination(std::move(other.destination)){
		this->source = other.source;
		this->direction = other.direction;
		this->local_pos = other.local_pos;
		this->remote_pos = other.remote_pos;
	}
};

struct MapConnection{
	Map destination = Map::Nowhere;
	int local_position;
	int remote_position;
	operator bool() const{
		return this->destination != Map::Nowhere;
	}
	bool operator!() const{
		return !(bool)*this;
	}
};

struct MapData{
	Map map_id;
	std::string name;
	int width, height;
	std::shared_ptr<TilesetData> tileset;
	std::shared_ptr<BinaryMapData> map_data;
	std::string script_name;
	MapConnection map_connections[4];
	int border_block;
	std::shared_ptr<std::vector<std::unique_ptr<MapObject>>> objects;

	MapData(
		Map map_id,
		BufferReader &buffer,
		const std::map<std::string, std::shared_ptr<TilesetData>> &tilesets,
		const std::map<std::string, std::shared_ptr<BinaryMapData>> &map_data,
		std::vector<TemporaryMapConnection> &tmcs,
		std::vector<std::pair<MapData *, std::string>> &map_objects
	);
	int get_block_at_map_position(const Point &);
	int get_partial_tile_at_actor_position(const Point &);
};

class MapStore;

class MapInstance{
	Map map;
	MapStore *store;
	int w, h;
	std::vector<bool> occupation_bitmap;

	void check_map_location(const Point &);
public:
	MapInstance(Map, MapStore &);
	void set_cell_occupation(const Point &, bool);
	bool get_cell_occupation(const Point &);
};

class MapStore{
	std::vector<std::unique_ptr<MapData>> maps;
	std::vector<std::pair<std::string, MapData *>> maps_by_name;
	std::vector<std::unique_ptr<MapInstance>> map_instances;
	
	typedef std::map<std::string, std::shared_ptr<Blockset>> blocksets_t;
	typedef std::map<std::string, std::shared_ptr<Collision>> collisions_t;
	typedef std::map<std::string, const GraphicsAsset *> graphics_map_t;
	typedef std::map<std::string, std::shared_ptr<TilesetData>> tilesets_t;
	typedef std::map<std::string, std::shared_ptr<BinaryMapData>> map_data_t;
	typedef std::map<std::string, std::shared_ptr<std::vector<std::unique_ptr<MapObject>>>> map_objects_t;
	typedef std::map<std::pair<std::string, int>, std::shared_ptr<BaseTrainerParty>> trainer_parties_t;
	static blocksets_t load_blocksets();
	static collisions_t load_collisions();
	static graphics_map_t load_graphics_map();
	static tilesets_t load_tilesets(const blocksets_t &, const collisions_t &, const graphics_map_t &);
	static map_data_t load_map_data();
	static trainer_parties_t load_trainer_parties();
	map_objects_t load_objects(const graphics_map_t &graphics_map);
	void load_maps(const tilesets_t &, const map_data_t &, const graphics_map_t &);
	//void load_map_objects();
public:
	MapStore();
	MapData &get_map_data(Map map);
	MapInstance &get_map_instance(Map map);
	MapData &get_map_by_name(const std::string &) const;
};
