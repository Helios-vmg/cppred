#include "Npc.h"
#include "Game.h"

namespace CppRed{

Npc::Npc(Game &game, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance):
		Actor(game, name, renderer, sprite){
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
	this->standing_sprites[(int)FacingDirection::Down]->set_visible(true);
	while (!this->quit_coroutine){
		auto delta = this->position.position - this->wandering_center;
		auto distance = abs(delta.x) + abs(delta.y);
		if (distance < this->wandering_radius){
			auto &rand = this->game->get_engine().get_prng();
			this->coroutine->wait(rand.generate_double() * (128.0 / 60.0));
			auto pick = geometric_distribution(rand, 3);
			auto direction = (FacingDirection)rand(4);
			
			this->standing_sprites[(int)this->facing_direction]->set_visible(false);
			this->facing_direction = direction;
			this->standing_sprites[(int)this->facing_direction]->set_visible(true);
			
			for (int i = pick; i--;){
				delta = this->position.position + direction_to_vector(direction) - this->wandering_center;
				distance = abs(delta.x) + abs(delta.y);
				if (distance >= this->wandering_radius){
					do
						direction = (FacingDirection)rand(4);
					while (direction != this->facing_direction);
					this->standing_sprites[(int)this->facing_direction]->set_visible(false);
					this->facing_direction = direction;
					this->standing_sprites[(int)this->facing_direction]->set_visible(true);
					break;
				}
				this->move(direction);
				this->object_instance->set_position(this->position.position);
			}
		}
		this->coroutine->yield();
	}
}

void Npc::update_sprites(){
	auto cam = this->game->get_camera_position();
	auto po = this->game->get_pixel_offset();
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
	this->game->get_objects_at_location(instances, this->position);
	for (auto instance : instances){
		if (!instance)
			break;
		if (!instance->get_object().npcs_can_walk_over())
			return false;
	}
	return true;
}

}