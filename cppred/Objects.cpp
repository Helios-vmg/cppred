#include "Objects.h"
#include "Maps.h"
#include "CppRed/Actor.h"
#include "CppRed/Trainer.h"
#include "CppRed/Npc.h"

MapObject::~MapObject(){}

std::pair<std::string, std::shared_ptr<std::vector<std::unique_ptr<MapObject>>>> MapObject::create_vector(
	BufferReader &buffer,
	const std::function<std::unique_ptr<MapObject>(BufferReader &)> &constructor){
	auto ret = std::make_shared<std::vector<std::unique_ptr<MapObject>>>();
	auto name = buffer.read_string();
	auto count = buffer.read_varint();
	ret->reserve(count);
	while (count--)
		ret->push_back(constructor(buffer));
	return{name, ret};
}

CppRed::actor_ptr<CppRed::Actor> MapObject::create_actor(CppRed::Game &game, Renderer &renderer) const{
	return CppRed::null_actor_ptr<CppRed::Actor>();
}

MapObject::MapObject(BufferReader &buffer){
	this->name = buffer.read_string();
	this->position.x = buffer.read_varint();
	this->position.y = buffer.read_varint();
}

EventDisp::EventDisp(BufferReader &buffer) : MapObject(buffer){}

Sign::Sign(BufferReader &buffer) : MapObject(buffer){
	this->text_index = buffer.read_varint();
}

HiddenObject::HiddenObject(BufferReader &buffer) : MapObject(buffer){
	this->script = buffer.read_string();
	this->script_parameter = buffer.read_string();
}

MapWarp::MapWarp(BufferReader &buffer, const MapStore &map_store) : MapObject(buffer){
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

ObjectWithSprite::ObjectWithSprite(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map) : MapObject(buffer){
	this->sprite = find_in_constant_map(graphics_map, buffer.read_string());
	this->facing_direction = to_MapObjectFacingDirection(buffer.read_string());
	this->wandering = !!buffer.read_varint();
	this->range = buffer.read_varint();
	this->text_index = buffer.read_varint();
}

NpcMapObject::NpcMapObject(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map) : ObjectWithSprite(buffer, graphics_map){
}

ItemMapObject::ItemMapObject(BufferReader &buffer,
	const std::map<std::string, const GraphicsAsset *> &graphics_map,
	const std::map<std::string, ItemId> &items_map) : ObjectWithSprite(buffer, graphics_map){
	this->item = find_in_constant_map(items_map, buffer.read_string());
}

TrainerMapObject::TrainerMapObject(BufferReader &buffer,
	const std::map<std::string, const GraphicsAsset *> &graphics_map,
	const std::map<std::pair<std::string, int>, std::shared_ptr<BaseTrainerParty>> &parties_map) : ObjectWithSprite(buffer, graphics_map){
	auto class_name = buffer.read_string();
	auto party_index = (int)buffer.read_varint();
	this->party = find_in_constant_map(parties_map, std::make_pair(class_name, party_index));
}

PokemonMapObject::PokemonMapObject(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map) :
	ObjectWithSprite(buffer, graphics_map){
	this->species = (SpeciesId)buffer.read_varint();
	this->level = (int)buffer.read_varint();
}

CppRed::actor_ptr<CppRed::Actor> NpcMapObject::create_actor(CppRed::Game &game, Renderer &renderer) const{
	auto ret = CppRed::create_actor2<CppRed::Npc>(game, this->name, renderer, *this->sprite);
	ret->set_map_position(this->position);
	return ret;
}
