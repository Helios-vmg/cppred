#pragma once
#include "Trainer.h"
#include "Renderer.h"
#include "utility.h"

struct MapData;

namespace CppRed{

enum class FacingDirection{
	Up = 0,
	Right,
	Down,
	Left,
};

class PlayerCharacter : public Trainer{
public:
	static const int screen_block_offset_x = 4;
	static const int screen_block_offset_y = 4;
private:
	const MapData *current_map = nullptr;
	Point map_position;
	std::shared_ptr<Sprite> standing_sprites[4];
	std::shared_ptr<Sprite> walking_sprites[4 * 4];
	FacingDirection facing_direction = FacingDirection::Down;
	bool moving = false;
	int walking_animation_state = 0;

	void initialize_sprites(const GraphicsAsset &graphics, Renderer &);
	template <typename T>
	void apply_to_all_sprites(const T &f){
		for (int i = 0; i < array_length(this->standing_sprites); i++)
			f(*this->standing_sprites[i]);
		for (int i = 0; i < array_length(this->walking_sprites); i++)
			f(*this->walking_sprites[i]);
	}
public:
	PlayerCharacter(const std::string &name, Renderer &);
	void set_visible_sprite();
	void teleport(const MapData *destination, const Point &position);
	DEFINE_GETTER_SETTER(facing_direction)
	DEFINE_GETTER(current_map)
	DEFINE_GETTER(map_position)
};

}
