#pragma once
#include "FirstLevelParser.h"
#include "../../../common/json.hpp"

namespace SecondLevelParser{

class MapObject{
protected:
	virtual nlohmann::json internal_serialize() const = 0;
	virtual const char *get_type() const = 0;
public:
	virtual ~MapObject(){}
	nlohmann::json serialize();
};

class Warp : public MapObject{
	unsigned y;
	unsigned x;
	unsigned warp_index;
	unsigned destination_warp_index;
	std::shared_ptr<FirstLevelParser::Number> fourth;
	nlohmann::json internal_serialize() const override;
	const char *get_type() const override{
		return "warp";
	}
public:
	Warp(unsigned y, unsigned x, unsigned warp_index, unsigned destination_warp_index, const std::shared_ptr<FirstLevelParser::Number> &fo):
		y(y),
		x(x),
		warp_index(warp_index),
		destination_warp_index(destination_warp_index),
		fourth(fo){}
};

class Sign : public MapObject{
	unsigned y, x, text_index;
	nlohmann::json internal_serialize() const override;
	const char *get_type() const override{
		return "sign";
	}
public:
	Sign(unsigned f, unsigned s, unsigned t):
		y(f),
		x(s),
		text_index(t){}
};

class SpritedMapObject : public MapObject{
protected:
	std::string sprite;
	unsigned x;
	unsigned y;
	std::string movement;
	std::string direction;
	int range = -1;
	unsigned text_id;
	virtual nlohmann::json internal_serialize() const override;
public:
	SpritedMapObject(const FirstLevelParser::MapObject &);
	virtual ~SpritedMapObject() = 0;
};

class Npc : public SpritedMapObject{
	const char *get_type() const override{
		return "npc";
	}
public:
	Npc(const FirstLevelParser::MapObject &);
};

class Trainer : public SpritedMapObject{
	std::string trainer_class_or_pokemon_id;
	unsigned trainer_number_or_pokemon_level;
	nlohmann::json internal_serialize() const override;
	const char *get_type() const override;
	bool is_pokemon() const;
public:
	Trainer(const FirstLevelParser::MapObject &);
};

class Item : public SpritedMapObject{
	std::shared_ptr<FirstLevelParser::Number> item_id;
	nlohmann::json internal_serialize() const override;
	const char *get_type() const override{
		return "item";
	}
public:
	Item(const FirstLevelParser::MapObject &);
};

class EventDisp : public MapObject{
	std::vector<std::shared_ptr<FirstLevelParser::Number>> elements;
	nlohmann::json internal_serialize() const override;
	const char *get_type() const override{
		return "event_disp";
	}
public:
	EventDisp(const FirstLevelParser::EventDisp &);
};

class Parser{
	std::string label;
	unsigned border_block;
	std::vector<std::unique_ptr<MapObject>> objects;
public:
	Parser(const FirstLevelParser::Parser &);
	const decltype(label) &get_label() const{
		return this->label;
	}
	const decltype(border_block) &get_border_block() const{
		return this->border_block;
	}
	const decltype(objects) &get_objects() const{
		return this->objects;
	}
	nlohmann::json serialize();
};

}
