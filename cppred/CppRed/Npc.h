#pragma once
#include "Actor.h"

namespace CppRed{

class Npc : public NonPlayerActor{
protected:
	Point wandering_center;
	int wandering_radius = -1;
	double special_movement_duration = Renderer::tile_size * 4;
	bool randomize_facing_direction = false;

	void coroutine_entry_point() override;
	bool can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction);
	bool move_internal(FacingDirection) override;
public:
	Npc(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &);
	void set_wandering(int radius);
	double movement_duration() const override{
		return this->special_movement_duration;
	}
	void set_special_movement_duration(double d){
		this->special_movement_duration = d;
	}
	void set_random_facing_direction(bool value) override{
		this->randomize_facing_direction = value;
	}
};

}
