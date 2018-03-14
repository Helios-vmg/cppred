#pragma once
#include "Actor.h"

namespace CppRed{

class Npc : public Actor{
protected:
	Point wandering_center;
	int wandering_radius = -1;
	double special_movement_duration = Renderer::tile_size * 4;

	void coroutine_entry_point() override;
	bool can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction);
	void update_sprites() override;
public:
	Npc(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &);
	void set_wandering(int radius);
	bool move(FacingDirection) override;
	double movement_duration() const override{
		return this->special_movement_duration;
	}
	void set_special_movement_duration(double d){
		this->special_movement_duration = d;
	}
};

}
