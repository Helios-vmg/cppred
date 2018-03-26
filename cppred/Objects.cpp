#include "Objects.h"
#include "Maps.h"
#include "CppRed/Actor.h"
#include "CppRed/Trainer.h"
#include "CppRed/Npc.h"
#include "CppRed/Game.h"
#include "CppRed/World.h"
#include "CppRed/TextDisplay.h"
#include "../CodeGeneration/output/variables.h"
#include "CppRed/ItemActor.h"
#include <cassert>

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

CppRed::actor_ptr<CppRed::Actor> MapObject::create_actor(CppRed::Game &game, Renderer &renderer, Map map, MapObjectInstance &instance) const{
	return CppRed::null_actor_ptr<CppRed::Actor>();
}

MapObject::MapObject(BufferReader &buffer){
	this->id = buffer.read_varint();
	this->name = buffer.read_string();
	this->position.x = buffer.read_varint();
	this->position.y = buffer.read_varint();
}

EventDisp::EventDisp(BufferReader &buffer): MapObject(buffer){}

Sign::Sign(BufferReader &buffer): DialogingMapObject(buffer){
	this->text_index = buffer.read_varint();
}

HiddenObject::HiddenObject(BufferReader &buffer): MapObject(buffer){
	this->script = buffer.read_string();
	this->script_parameter = buffer.read_string();
}

void HiddenObject::activate(CppRed::Game &game, CppRed::Actor &activator, CppRed::Actor *activatee){
	game.execute(this->script.c_str(), activator, this->script_parameter.c_str());
}

MapWarp::MapWarp(BufferReader &buffer, const MapStore &map_store): MapObject(buffer){
	this->index = buffer.read_varint();
	auto temp = buffer.read_string();
	if (temp.size() >= 4 && temp[0] == 'v' && temp[1] == 'a' && temp[2] == 'r' && temp[3] == ':')
		this->destination = WarpDestination((CppRed::IntegerVariableId)buffer.read_varint());
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

ObjectWithSprite::ObjectWithSprite(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map): DialogingMapObject(buffer){
	this->legacy_id = buffer.read_varint();
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
		const std::map<std::string, ItemId> &items_map): ObjectWithSprite(buffer, graphics_map){
	this->item = find_in_constant_map(items_map, buffer.read_string());
}

TrainerMapObject::TrainerMapObject(BufferReader &buffer,
		const std::map<std::string, const GraphicsAsset *> &graphics_map,
		const std::map<std::string, std::map<int, std::shared_ptr<BaseTrainerParty>>> &parties_map): NpcMapObject(buffer, graphics_map){
	auto class_name = buffer.read_string();
	this->default_party = (int)buffer.read_varint();
	this->parties = find_in_constant_map(parties_map, class_name);
}

PokemonMapObject::PokemonMapObject(BufferReader &buffer, const std::map<std::string, const GraphicsAsset *> &graphics_map):
		ObjectWithSprite(buffer, graphics_map){
	this->species = (SpeciesId)buffer.read_varint();
	this->level = (int)buffer.read_varint();
}

void init_actor(CppRed::Actor &actor, int object_id, Map map, const Point &position, MapObjectFacingDirection facing_direction, CppRed::Game &game){
	actor.set_object_id(object_id);
	actor.set_current_map(map);
	actor.set_map_position(position);
	switch (facing_direction){
		case MapObjectFacingDirection::Undefined:
		case MapObjectFacingDirection::BoulderMovementByte2:
			break;
		case MapObjectFacingDirection::None:
			actor.set_random_facing_direction(true);
			break;
		case MapObjectFacingDirection::Up:
			actor.set_facing_direction(FacingDirection::Up);
			break;
		case MapObjectFacingDirection::Right:
			actor.set_facing_direction(FacingDirection::Right);
			break;
		case MapObjectFacingDirection::Down:
			actor.set_facing_direction(FacingDirection::Down);
			break;
		case MapObjectFacingDirection::Left:
			actor.set_facing_direction(FacingDirection::Left);
			break;
		default:
			throw std::exception();
	}
	game.get_world().get_map_store().get_map_instance(map, game).set_cell_occupation(position, true);
}

void NpcMapObject::initialize_actor(CppRed::Npc &npc, Map map, CppRed::Game &game) const{

	init_actor(npc, this->id, map, this->position, this->facing_direction, game);
	if (this->wandering)
		npc.set_wandering(this->range);
}

CppRed::actor_ptr<CppRed::Actor> NpcMapObject::create_actor(CppRed::Game &game, Renderer &renderer, Map map, MapObjectInstance &instance) const{
	auto ret = CppRed::create_actor2<CppRed::Npc>(game, game.get_coroutine(), this->name, renderer, *this->sprite, instance);
	auto npc = (CppRed::Npc *)ret.get();
	this->initialize_actor(*npc, map, game);
	return ret;
}

CppRed::actor_ptr<CppRed::Actor> ItemMapObject::create_actor(CppRed::Game &game, Renderer &renderer, Map map, MapObjectInstance &instance) const{
	auto ret = CppRed::create_actor2<CppRed::ItemActor>(game, game.get_coroutine(), this->name, renderer, *this->sprite, instance);
	auto item = (CppRed::ItemActor *)ret.get();
	init_actor(*item, this->id, map, this->position, this->facing_direction, game);
	return ret;
}

void DialogingMapObject::activate(CppRed::Game &game, CppRed::Actor &activator, CppRed::Actor *activatee){
	auto index = get_text_index();
	if (index < 0 || index >= this->map_data->map_text.size())
		return;
	const auto &text = this->map_data->map_text[index];
	if (text.simple_text)
		game.run_dialogue(text.text, true, true);
	else
		game.execute(text.script.c_str(), activator);
}

void ObjectWithSprite::activate(CppRed::Game &game, CppRed::Actor &activator, CppRed::Actor *activatee){
	assert(activatee);
	if (activatee->is_moving() || !activatee->get_visible())
		return;
	auto old_facing_direction = activatee->get_facing_direction();
	activatee->set_facing_direction(invert_direction(activator.get_facing_direction()));
	auto old_random = activatee->get_random_facing_direction();
	activatee->set_random_facing_direction(false);
	
	DialogingMapObject::activate(game, activator, activatee);
	
	activatee->set_random_facing_direction(old_random);
	activatee->set_facing_direction(old_facing_direction);
}

CppRed::actor_ptr<CppRed::Actor> TrainerMapObject::create_actor(CppRed::Game &game, Renderer &renderer, Map map, MapObjectInstance &instance) const{
	auto ret = CppRed::create_actor2<CppRed::NpcTrainer>(
		game,
		game.get_coroutine(),
		this->name,
		renderer,
		*this->sprite,
		instance,
		this->parties,
		this->default_party
	);
	auto npc = (CppRed::Npc *)ret.get();
	this->initialize_actor(*npc, map, game);
	return ret;
}
