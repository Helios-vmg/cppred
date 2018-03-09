#include "PlayerCharacter.h"
#include "Game.h"
#include "Actor.h"
#include "../Maps.h"
#include "World.h"
#include <cassert>

namespace CppRed{

const Point PlayerCharacter::screen_block_offset(4, 4);

PlayerCharacter::PlayerCharacter(Game &game, const std::string &name, Renderer &renderer):
		Trainer(game, name, renderer, RedSprite){
}

void PlayerCharacter::initialize_sprites(const GraphicsAsset &graphics, Renderer &renderer){
	Trainer::initialize_sprites(graphics, renderer);
	auto s = Renderer::tile_size;
	Point position = screen_block_offset * (s * 2);
	position.y -= s / 2;
	this->apply_to_all_sprites([&position](auto &sprite){ sprite.set_position(position); });
}

void PlayerCharacter::teleport(const WorldCoordinates &destination){
	auto old_map = this->position.map;
	this->position = destination;
	if (old_map != destination.map)
		this->entered_new_map(old_map, destination.map);
}

void PlayerCharacter::coroutine_entry_point(){
	while (!this->quit_coroutine){
		auto input = this->game->get_engine().get_input_state();
		auto &world = this->game->get_world();
		if (input.get_start()){
			//handle menu
			this->coroutine->yield();
		}else if (input.get_a()){
			MapObjectInstance *instances[8];
			world.get_objects_at_location(instances, world.remap_coordinates(this->position + direction_to_vector(this->facing_direction)));
			for (auto instance : instances){
				if (!instance)
					break;
				instance->activate(*this);
			}
			this->coroutine->yield();
		}else if (input.any_direction()){
			if (!this->handle_movement(input))
				this->coroutine->yield();
			MapObjectInstance *instances[8];
			world.get_objects_at_location(instances, this->position);
			for (auto instance : instances){
				if (!instance)
					break;
				auto object = dynamic_cast<const MapWarp *>(&instance->get_object());
				if (!object)
					continue;
				world.teleport_player(*object);
				break;
			}
		}else
			this->coroutine->yield();
	}
}

bool PlayerCharacter::handle_movement(InputState &input){
	if (input.get_up())
		return this->move({0, -1}, FacingDirection::Up);
	if (input.get_right())
		return this->move({1, 0}, FacingDirection::Right);
	if (input.get_down())
		return this->move({0, 1}, FacingDirection::Down);
	assert(input.get_left());
	return this->move({-1, 0}, FacingDirection::Left);
}

void PlayerCharacter::entered_new_map(Map old_map, Map new_map){
	this->game->entered_map(old_map, new_map);
}

}
