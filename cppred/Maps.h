#pragma once

#include "common_types.h"
#include "GraphicsAsset.h"
#include "../CodeGeneration/output/maps.h"
#include "../CodeGeneration/output/items.h"
#include "../common/TilesetType.h"
#include "TrainerData.h"
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

struct TilesetData{
	std::string name;
	std::shared_ptr<Blockset> blockset;
	const GraphicsAsset *tiles;
	std::shared_ptr<Collision> collision;
	short counters[16];
	int grass_tile;
	TilesetType type;

	TilesetData(
		const byte_t *,
		size_t &,
		size_t,
		const std::map<std::string, std::shared_ptr<Blockset>> &,
		const std::map<std::string, std::shared_ptr<Collision>> &,
		const std::map<std::string, const GraphicsAsset *> &
	);
};

class MapObject{
protected:
	const char *name;
	int x, y;
public:
	MapObject(const char *name, int x, int y): name(name), x(x), y(y){}
	virtual ~MapObject() = 0;
};

inline MapObject::~MapObject(){}

class EventDisp : public MapObject{
public:
	EventDisp(const char *name, int x, int y): MapObject(name, x, y){}
};

class Sign : public MapObject{
protected:
	int text_index;
public:
	Sign(const char *name, int x, int y, int text_index): MapObject(name, x, y), text_index(text_index){}
};

class HiddenObject : public MapObject{
protected:
	const char *script;
	const char *script_parameter;
public:
	HiddenObject(const char *name, int x, int y, const char *script, const char *script_parameter):
		MapObject(name, x, y),
		script(script),
		script_parameter(script_parameter){}
};

struct MapData;

struct WarpDestination{
	bool simple;
	const MapData *destination_map = nullptr;
	std::string variable_name;
	WarpDestination(const MapData &destination_map): simple(true), destination_map(&destination_map){}
	WarpDestination(const char *variable_name): simple(false), variable_name(variable_name){}
	WarpDestination(const WarpDestination &) = default;
};

class MapWarp : public MapObject{
protected:
	int index;
	WarpDestination destination;
	int destination_warp_index;
public:
	MapWarp(const char *name, int x, int y, int index, const WarpDestination &destination, int destination_warp_index):
		MapObject(name, x, y),
		index(index),
		destination(destination),
		destination_warp_index(destination_warp_index){}
};

class ObjectWithSprite : public MapObject{
public:
	enum class FacingDirection{
		Undefined,
		None,
		Up,
		Right,
		Down,
		Left,
		BoulderMovementByte2,
	};
protected:
	const GraphicsAsset *sprite;
	FacingDirection facing_direction;
	bool wandering;
	int range;
	int text_index;
public:
	ObjectWithSprite(const char *name, int x, int y, const GraphicsAsset &sprite, FacingDirection facing_direction, bool wandering, int range, int text_index):
		MapObject(name, x, y),
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
	NpcMapObject(const char *name, int x, int y, const GraphicsAsset &sprite, FacingDirection facing_direction, bool wandering, int range, int text_id):
		ObjectWithSprite(name, x, y, sprite, facing_direction, wandering, range, text_id){}
};

class ItemMapObject : public ObjectWithSprite{
protected:
	ItemId item;
public:
	ItemMapObject(const char *name, int x, int y, const GraphicsAsset &sprite, FacingDirection facing_direction, bool wandering, int range, int text_id, ItemId item):
		ObjectWithSprite(name, x, y, sprite, facing_direction, wandering, range, text_id),
		item(item){}
};

class TrainerMapObject : public ObjectWithSprite{
protected:
	const BaseTrainerParty *party;
public:
	TrainerMapObject(const char *name, int x, int y, const GraphicsAsset &sprite, FacingDirection facing_direction, bool wandering, int range, int text_id, const BaseTrainerParty *party):
		ObjectWithSprite(name, x, y, sprite, facing_direction, wandering, range, text_id),
		party(party){}
};

class PokemonMapObject : public ObjectWithSprite{
protected:
	SpeciesId species;
	int level;
public:
	PokemonMapObject(const char *name, int x, int y, const GraphicsAsset &sprite, FacingDirection facing_direction, bool wandering, int range, int text_id, SpeciesId species, int level):
		ObjectWithSprite(name, x, y, sprite, facing_direction, wandering, range, text_id),
		species(species),
		level(level){}
};

struct BinaryMapData{
	std::string name;
	std::vector<byte_t> data;

	BinaryMapData(const byte_t *, size_t &, size_t);
};

struct MapData{
	std::string name;
	std::shared_ptr<TilesetData> tileset;
	int width, height;
	std::shared_ptr<BinaryMapData> map_data;
	std::string script;

	MapData(const byte_t *, size_t &, size_t, const std::map<std::string, std::shared_ptr<TilesetData>> &tilesets, const std::map<std::string, std::shared_ptr<BinaryMapData>> &map_data);
};

class MapStore{
	std::vector<std::unique_ptr<MapData>> maps;
	
	typedef std::map<std::string, std::shared_ptr<Blockset>> blocksets_t;
	typedef std::map<std::string, std::shared_ptr<Collision>> collisions_t;
	typedef std::map<std::string, const GraphicsAsset *> graphics_map_t;
	typedef std::map<std::string, std::shared_ptr<TilesetData>> tilesets_t;
	typedef std::map<std::string, std::shared_ptr<BinaryMapData>> map_data_t;
	static blocksets_t load_blocksets();
	static collisions_t load_collisions();
	static graphics_map_t load_graphics_map();
	static tilesets_t load_tilesets(const blocksets_t &, const collisions_t &, const graphics_map_t &);
	static map_data_t load_map_data();
	void load_maps(const tilesets_t &, const map_data_t &);
public:
	MapStore();
	MapData &get_map(Map map);
};
