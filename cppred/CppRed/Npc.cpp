#include "Npc.h"
#include "Game.h"
#include "../Maps.h"
#include "World.h"
#include <iostream>

namespace CppRed{

Npc::Npc(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance):
		Actor(game, parent_coroutine, name, renderer, sprite){
	this->object_instance = &instance;
}

static int geometric_distribution(XorShift128 &rand, std::uint32_t n){
	int ret = n;
	n = rand(1 << n);
	for (int i = 0; i < 32 && n; i++){
		n >>= 1;
		ret--;
	}
	return ret;
}

void Npc::coroutine_entry_point(){
	this->standing_sprites[(int)this->facing_direction]->set_visible(this->visible);
	HighResolutionClock real_time;
	auto &clock = this->coroutine->get_clock();
	while (!this->quit_coroutine){
		auto delta = this->position.position - this->wandering_center;
		auto distance = abs(delta.x) + abs(delta.y);
		if (distance < this->wandering_radius){
			auto &rand = this->game->get_engine().get_prng();
			this->coroutine->wait(rand.generate_double() * (128.0 / 60.0));
			auto pick = geometric_distribution(rand, 3);
			auto direction = (FacingDirection)rand(4);
			
			this->set_facing_direction(direction);
			
			for (int i = pick; i--;){
				delta = this->position.position + direction_to_vector(direction) - this->wandering_center;
				distance = abs(delta.x) + abs(delta.y);
				if (distance >= this->wandering_radius){
					do
						direction = (FacingDirection)rand(4);
					while (direction != this->facing_direction);
					this->set_facing_direction(direction);
					break;
				}
				this->move(direction);
			}
		}
		this->coroutine->yield();
	}
}

bool Npc::move(FacingDirection direction){
	auto ret = Actor::move(direction);
	this->object_instance->set_position(this->position.position);
	return ret;
}

void Npc::update_sprites(){
	auto &world = this->game->get_world();
	auto cam = world.get_camera_position();
	auto po = world.get_pixel_offset();
	auto position = (this->position.position - cam) * (Renderer::tile_size * 2) - po + this->pixel_offset;
	position.y -= Renderer::tile_size / 2;
	this->apply_to_all_sprites([&position](Sprite &sprite){ sprite.set_position(position); });
}

void Npc::set_wandering(int radius){
	this->wandering_center = this->position.position;
	this->wandering_radius = radius;
}

bool Npc::can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction){
	if (!Actor::can_move_to(current_position, next_position, direction))
		return false;
	MapObjectInstance *instances[8];
	auto &world = this->game->get_world();
	world.get_objects_at_location(instances, this->position);
	for (auto instance : instances){
		if (!instance)
			break;
		if (!instance->get_object().npcs_can_walk_over())
			return false;
	}
	return true;
}

}
