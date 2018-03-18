#include "PlayerCharacter.h"
#include "Game.h"
#include "Actor.h"
#include "../Maps.h"
#include "World.h"
#include <cassert>
#include <iostream>

namespace CppRed{

const Point PlayerCharacter::screen_block_offset(4, 4);
const PlayerCharacter::warp_check_f PlayerCharacter::warp_check_functions[2] = {
	&PlayerCharacter::facing_edge_of_map,
	&PlayerCharacter::is_in_front_of_warp_tile,
};

PlayerCharacter::PlayerCharacter(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer):
		Actor(game, parent_coroutine, name, renderer, RedSprite){
}

void PlayerCharacter::initialize_sprites(const GraphicsAsset &graphics, Renderer &renderer){
	Actor::initialize_sprites(graphics, renderer);
	auto s = Renderer::tile_size;
	Point position = screen_block_offset * (s * 2);
	position.y -= s / 2;
	this->apply_to_all_sprites([&position](auto &sprite){ sprite.set_position(position); });
}

void PlayerCharacter::teleport(const WorldCoordinates &destination){
	auto old_position = this->position;
	this->position = destination;
	auto &map_instance = this->game->get_world().get_map_instance(destination.map);
	if (old_position.map != destination.map)
		this->entered_new_map(old_position.map, destination.map, true);
	else
		map_instance.set_cell_occupation(old_position.position, false);
	map_instance.set_cell_occupation(destination.position, true);
}

void PlayerCharacter::coroutine_entry_point(){
	while (!this->quit_coroutine){
		if (!this->run_saved_actions())
			continue;
		auto &engine = this->game->get_engine();
		if (this->ignore_input){
			this->coroutine->yield();
			continue;
		}
		auto input = engine.get_input_state();
		if (input.any_direction()){
			if (!this->move_internal(this->input_to_direction(input)))
				this->coroutine->yield();
			continue;
		}else{
			input = this->game->joypad_only_newly_pressed();
			if (input.get_start()){
				//handle menu
			}else if (input.get_a()){
				MapObjectInstance *instances[8];
				auto &world = this->game->get_world();
				world.get_objects_at_location(instances, world.remap_coordinates(this->position + direction_to_vector(this->facing_direction)));
				bool activated = false;
				for (auto instance : instances){
					if (!instance)
						break;
					instance->activate(*this);
					activated = true;
				}
				if (!activated)
					this->check_for_bookshelf_or_card_key_door();
			}
			this->coroutine->yield();
		}
	}
}

bool PlayerCharacter::move_internal(FacingDirection direction){
	if (Actor::move_internal(direction)){
		if (this->saved_post_warp){
			auto warp = this->saved_post_warp;
			this->saved_actions.push_back([this, warp](){
				this->game->get_world().teleport_player(*warp);
			});
			this->saved_post_warp = nullptr;
		}
		return true;
	}
	if (!this->run_warp_logic_collision())
		return false;
	return Actor::move_internal(direction);
}

bool PlayerCharacter::run_warp_logic_collision(){
	auto &world = this->game->get_world();

	auto &ms = world.get_map_store();
	auto &map_data = ms.get_map_data(this->position.map);
	if (!(map_data.warp_check >= 0 && map_data.warp_check < array_length(this->warp_check_functions)))
		return false;
	if (!(this->*this->warp_check_functions[map_data.warp_check])(world))
		return false;

	MapObjectInstance *instances[8];
	world.get_objects_at_location(instances, this->position);
	const MapWarp *warp = nullptr;
	for (auto instance : instances){
		if (!instance)
			break;
		auto &o = instance->get_object();
		if (o.get_type() != MapObjectType::Warp)
			continue;
		warp = static_cast<const MapWarp *>(&o);
		break;
	}
	if (!warp)
		return false;
	//auto c = Coroutine::get_current_coroutine_ptr();
	world.teleport_player(*warp);
	return true;
}

bool PlayerCharacter::run_warp_logic_no_collision(){
	auto &world = this->game->get_world();
	auto &ms = world.get_map_store();
	auto &map_data = ms.get_map_data(this->position.map);
	auto &map_instance = world.get_map_instance(this->position.map);
	auto pos2 = this->position + direction_to_vector(this->facing_direction);
	
	MapObjectInstance *instances[8];
	world.get_objects_at_location(instances, pos2);
	const MapWarp *warp = nullptr;
	for (auto instance : instances){
		if (!instance)
			break;
		auto &o = instance->get_object();
		if (o.get_type() != MapObjectType::Warp)
			continue;
		if (!map_instance.is_warp_tile(pos2.position)){
			if (!(map_data.warp_check >= 0 && map_data.warp_check < array_length(this->warp_check_functions)))
				continue;
			if (!(this->*this->warp_check_functions[map_data.warp_check])(world))
				continue;
		}
		warp = static_cast<const MapWarp *>(&o);
		break;
	}
	if (!warp)
		return false;
	this->saved_post_warp = warp;
	return true;
}

FacingDirection PlayerCharacter::input_to_direction(const InputState &input){
	if (input.get_up())
		return FacingDirection::Up;
	if (input.get_right())
		return FacingDirection::Right;
	if (input.get_down())
		return FacingDirection::Down;
	assert(input.get_left());
	return FacingDirection::Left;
}

void PlayerCharacter::entered_new_map(Map old_map, Map new_map, bool warped){
	this->game->entered_map(old_map, new_map, warped);
}

bool PlayerCharacter::facing_edge_of_map(const World &world) const{
	return world.facing_edge_of_map(this->position, this->facing_direction);
}

bool PlayerCharacter::is_in_front_of_warp_tile(const World &world) const{
	auto pos2 = this->position + direction_to_vector(this->facing_direction);
	return world.get_map_instance(pos2.map).is_warp_tile(pos2.position);
}

void PlayerCharacter::about_to_move(){
	this->run_warp_logic_no_collision();
}

void PlayerCharacter::check_for_bookshelf_or_card_key_door(){
	auto pos2 = this->position + direction_to_vector(this->facing_direction);
	auto &map_data = this->game->get_world().get_map_store().get_map_data(pos2.map);
	auto tile = map_data.get_partial_tile_at_actor_position(pos2.position);
	auto info = map_data.tileset->get_bookcase_info(tile);
	if (!info)
		return;
	if (info->is_script)
		this->game->execute(info->script_name.c_str(), *this);
	else
		this->game->run_dialog_from_world(info->text_id, *this);
}

}
