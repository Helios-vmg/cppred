#pragma once
#include "utility.h"
#include "GraphicsAsset.h"
#include <string>
#include "CppRed/Actor.h"

namespace CppRed{
class Actor;
class Game;
}
struct MapData;
class MapStore;
class BaseTrainerParty;
enum class MapObjectFacingDirection;
enum class ItemId;
enum class SpeciesId;

class MapObject{
protected:
	std::string name;
	Point position = {-1, -1};
	const MapData *map_data = nullptr;

	MapObject(BufferReader &);
public:
	MapObject(const std::string &name, const Point &position): name(name), position(position){}
	virtual ~MapObject() = 0;
	static std::pair<std::string, std::shared_ptr<std::vector<std::unique_ptr<MapObject>>>> create_vector(BufferReader &buffer, const std::function<std::unique_ptr<MapObject>(BufferReader &)> &);
	virtual const char *get_type_string() const = 0;
	virtual bool requires_actor() const = 0;
	virtual CppRed::actor_ptr<CppRed::Actor> create_actor(CppRed::Game &game, Renderer &renderer, Map map, MapObjectInstance &instance) const;
	virtual bool npcs_can_walk_over() const{
		return true;
	}
	virtual void activate(CppRed::Game &, const CppRed::Actor &activator){}
	DEFINE_GETTER(position)
	DEFINE_GETTER(name)
	DEFINE_GETTER_SETTER(map_data)
};

class EventDisp : public MapObject{
public:
	EventDisp(BufferReader &);
	EventDisp(const std::string &name, const Point &position): MapObject(name, position){}
	const char *get_type_string() const override{
		return "event_disp";
	}
	bool requires_actor() const override{
		return false;
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
	bool requires_actor() const override{
		return false;
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
	bool requires_actor() const override{
		return false;
	}
};

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
	bool requires_actor() const override{
		return false;
	}
	bool npcs_can_walk_over() const override{
		return false;
	}
	DEFINE_GETTER(index)
	DEFINE_GETTER(destination)
	DEFINE_GETTER(destination_warp_index)
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
	bool requires_actor() const override{
		return true;
	}
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
	CppRed::actor_ptr<CppRed::Actor> create_actor(CppRed::Game &game, Renderer &renderer, Map map, MapObjectInstance &instance) const override;
	void activate(CppRed::Game &, const CppRed::Actor &activator) override;
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
