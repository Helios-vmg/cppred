#include "SecondLevelParser.h"
#include "data.h"
#include <iostream>

namespace SecondLevelParser{

Parser::Parser(const FirstLevelParser::Parser &parser){
	this->label = parser.get_label();
	auto &objects = parser.get_objects();
	size_t index = 0;

	assume(objects.size() >= index + 1);

	this->border_block = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]).get_value();

	assume(objects.size() >= index + 1);
	unsigned warp_count = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]).get_value();
	for (unsigned i = 0; i < warp_count; i++){
		assume(objects.size() >= index + 4);
		auto &first = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]);
		auto &second = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]);
		auto &third = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]);
		auto fourth = std::dynamic_pointer_cast<FirstLevelParser::Number>(objects[index++]);
		assume(fourth);
		this->objects.emplace_back(std::make_unique<Warp>(first.get_value(), second.get_value(), i, third.get_value(), fourth));
	}

	assume(objects.size() >= index + 1);
	unsigned sign_count = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]).get_value();
	for (unsigned i = 0; i < sign_count; i++){
		assume(objects.size() >= index + 3);
		auto &first = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]);
		auto &second = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]);
		auto &third = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]);
		this->objects.emplace_back(std::make_unique<Sign>(first.get_value(), second.get_value(), third.get_value()));
	}

	assume(objects.size() >= index + 1);
	unsigned object_count = dynamic_cast<FirstLevelParser::Number &>(*objects[index++]).get_value();
	for (unsigned i = 0; i < object_count; i++){
		assume(objects.size() >= index + 1);
		auto &obj = dynamic_cast<FirstLevelParser::MapObject &>(*objects[index++]);
		switch (obj.get_elements().size()){
			case 6:
				this->objects.emplace_back(std::make_unique<Npc>(obj));
				break;
			case 7:
				this->objects.emplace_back(std::make_unique<Item>(obj));
				break;
			case 8:
				this->objects.emplace_back(std::make_unique<Trainer>(obj));
				break;
			default:
				assume(false);
		}
	}

	for (size_t i = index; i < objects.size(); i++){
		auto ed = std::dynamic_pointer_cast<FirstLevelParser::EventDisp>(objects[i]);
		if (!ed){
			std::cerr << "Warning: " << this->label << " contains unparseable EVENT_DISP elements.\n";
			continue;
		}
		this->objects.emplace_back(std::make_unique<EventDisp>(*ed));
	}
}

SpritedMapObject::SpritedMapObject(const FirstLevelParser::MapObject &mo){
	auto &elements = mo.get_elements();
	this->sprite = dynamic_cast<FirstLevelParser::Identifier &>(*elements[0]).get_identifier();
	this->x = elements[1]->get_value();
	this->y = elements[2]->get_value();
	this->movement = dynamic_cast<FirstLevelParser::Identifier &>(*elements[3]).get_identifier();
	auto direction = std::dynamic_pointer_cast<FirstLevelParser::Identifier>(elements[4]);
	if (direction)
		this->direction = direction->get_identifier();
	else
		this->range = elements[4]->get_value();
	this->text_id = elements[5]->get_value();
}

SpritedMapObject::~SpritedMapObject(){}

Npc::Npc(const FirstLevelParser::MapObject &mo): SpritedMapObject(mo){}

Trainer::Trainer(const FirstLevelParser::MapObject &mo): SpritedMapObject(mo){
	auto &elements = mo.get_elements();
	this->trainer_class_or_pokemon_id = dynamic_cast<FirstLevelParser::Identifier &>(*elements[6]).get_identifier();
	this->trainer_number_or_pokemon_level = elements[7]->get_value();
}

Item::Item(const FirstLevelParser::MapObject &mo): SpritedMapObject(mo){
	auto &elements = mo.get_elements();
	this->item_id = std::dynamic_pointer_cast<FirstLevelParser::Number>(elements[6]);
}

EventDisp::EventDisp(const FirstLevelParser::EventDisp &ed){
	this->elements = ed.get_elements();
}

nlohmann::json Parser::serialize(){
	nlohmann::json ret;
	ret["name"] = this->label;
	ret["border_block"] = this->border_block;
	nlohmann::json::array_t objects;
	for (auto &o : this->objects)
		objects.push_back(o->serialize());
	ret["objects"] = objects;
	return ret;
}

nlohmann::json MapObject::serialize(){
	auto ret = this->internal_serialize();
	ret["type"] = this->get_type();
	return ret;
}

nlohmann::json Warp::internal_serialize() const{
	nlohmann::json ret;
	ret["y"] = this->y;
	ret["x"] = this->x;
	ret["warp_index"] = this->warp_index;
	ret["destination_warp_index"] = this->destination_warp_index;
	auto s = this->fourth->to_string();
	auto it = map_map.find(s);
	if (it != map_map.end())
		s = it->second;
	else{
		if (s == "237")
			s = "var:SilphCoElevatorCurrentFloor";
		else{
			assume(s == "255");
			s = "var:LastOutdoorsMap";
		}
	}
	ret["destination"] = s;
	return ret;
}

nlohmann::json Sign::internal_serialize() const{
	nlohmann::json ret;
	ret["y"] = this->y;
	ret["x"] = this->x;
	ret["text_index"] = this->text_index;
	return ret;
}

static std::string SNAKE_CASE_to_CamelCase(const std::string &s){
	std::string ret;
	bool upper = true;
	for (auto c0 : s){
		auto c = c0;
		if (c == '_'){
			upper = true;
			continue;
		}
		c = upper ? toupper(c) : tolower(c);
		upper = false;
		ret.push_back(c);
	}
	return ret;
}

static std::string transform_sprite_string(const std::string &s){
	static const char * const sprite_string = "SPRITE_";
	const size_t n = 7;
	if (s.size() < n)
		return s;
	if (s.substr(0, n) != sprite_string)
		return s;

	std::string ret = SNAKE_CASE_to_CamelCase(s.substr(n));
	if (ret == "GameboyKidSpriteCopy")
		ret = "GameboyKidSprite";
	ret += "Sprite";
	return ret;
}

nlohmann::json SpritedMapObject::internal_serialize() const{
	nlohmann::json ret;
	ret["sprite"] = transform_sprite_string(this->sprite);
	ret["x"] = this->x;
	ret["y"] = this->y;
	assume(this->movement == "STAY" || this->movement == "WALK");
	bool move = this->movement == "WALK";
	ret["move"] = move;
	if (!move){
		ret["range"] = 0;
		ret["direction"] = SNAKE_CASE_to_CamelCase(this->direction);
	}else{
		assume(this->range >= 0 && !this->direction.size());
		ret["range"] = this->range;
		ret["direction"] = "";
	}
	ret["text_id"] = this->text_id;
	return ret;
}

bool Trainer::is_pokemon() const{
	return this->trainer_class_or_pokemon_id.substr(0, 4) != "OPP_";
}

const char *Trainer::get_type() const{
	return !this->is_pokemon() ?  "trainer" : "pokemon";
}

nlohmann::json Trainer::internal_serialize() const{
	auto ret = SpritedMapObject::internal_serialize();
	bool is_pokemon = this->is_pokemon();
	if (!is_pokemon){
		ret["trainer_class"] = SNAKE_CASE_to_CamelCase(this->trainer_class_or_pokemon_id);
		ret["trainer_number"] = this->trainer_number_or_pokemon_level;
	}else{
		ret["species"] = SNAKE_CASE_to_CamelCase(this->trainer_class_or_pokemon_id);
		ret["level"] = this->trainer_number_or_pokemon_level;
	}
	return ret;
}

nlohmann::json Item::internal_serialize() const{
	auto ret = SpritedMapObject::internal_serialize();
	auto s = this->item_id->to_string();
	auto it = item_constants.find(s);
	if (it == item_constants.end())
		s.clear();
	else
		s = it->second;
	ret["item_id"] = s;
	return ret;
}

nlohmann::json EventDisp::internal_serialize() const{
	nlohmann::json ret;
	assume(this->elements.size() == 3);
	ret["x"] = this->elements[1]->get_value();
	ret["y"] = this->elements[2]->get_value();
	//nlohmann::json::array_t elements;
	//for (auto &e : this->elements)
	//	elements.push_back(e->to_string());
	//ret["elements"] = elements;
	return ret;
}

}
