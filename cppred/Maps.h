#pragma once

#include "common_types.h"
#include "GraphicsAsset.h"
#include "../CodeGeneration/output/maps.h"
#include "../common/TilesetType.h"
#include "TrainerData.h"
#include <vector>

struct TilesetData{
	const char *name;
	Blocksets::pair_t blockset;
	const GraphicsAsset *tiles;
	Collision::pair_t collision;
	std::vector<int> counters;
	int grass_tile;
	TilesetType type;
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

struct MapData{
	const char *name;
	const TilesetData *tileset;
	int width, height;
	BinaryMapData::pair_t map_data;
	const char *script;
};
