#pragma once
#include "Actor.h"

namespace CppRed{

class Npc : public Actor{
protected:
	Point wandering_center;
	int wandering_radius = -1;

	void coroutine_entry_point() override;
	double movement_duration() const override{
		return Renderer::tile_size * 4;
	}
	bool can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction);
	void update_sprites() override;
public:
	Npc(Game &game, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &);
	void set_wandering(int radius);
};

}
