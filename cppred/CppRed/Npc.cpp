#include "Npc.h"
#include "Game.h"
#include "../Maps.h"
#include "World.h"
#include "Coroutine.h"
#include "HighResolutionClock.h"

namespace CppRed{

Npc::Npc(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance):
		NonPlayerActor(game, parent_coroutine, name, renderer, sprite, instance){
}

Npc::~Npc(){}

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
		if (!this->run_saved_actions())
			continue;
		auto delta = this->position.position - this->wandering_center;
		auto distance = abs(delta.x) + abs(delta.y);
		bool can_move_further = distance < this->wandering_radius;
		if (this->randomize_facing_direction || can_move_further){
			auto &rand = this->game->get_engine().get_prng();
			auto wait = rand.generate_double() * (128.0 / 60.0) + 1;
			auto initial_time = clock.get();
			while (clock.get() - initial_time < wait && (this->randomize_facing_direction || can_move_further))
				this->coroutine->yield();
			if (!this->randomize_facing_direction && !can_move_further)
				continue;
			auto direction = (FacingDirection)rand(4);
			
			this->set_facing_direction(direction);
			
			if (can_move_further){
				auto pick = geometric_distribution(rand, 3);
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
		}
		this->coroutine->yield();
	}
}

bool Npc::move_internal(FacingDirection direction){
	auto ret = Actor::move_internal(direction);
	this->object_instance->set_position(this->position.position);
	return ret;
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
