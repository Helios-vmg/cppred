#pragma once
#include "Trainer.h"
#include "Renderer.h"
#include "utility.h"
#include <boost/coroutine2/all.hpp>

class InputState;
enum class Map;

namespace CppRed{

enum class FacingDirection{
	Up = 0,
	Right,
	Down,
	Left,
};

class Game;

class PlayerCharacter : public Trainer{
public:
	static const int screen_block_offset_x = 4;
	static const int screen_block_offset_y = 4;
private:
	Game *game;
	Map current_map;
	Point map_position;
	std::shared_ptr<Sprite> standing_sprites[4];
	std::shared_ptr<Sprite> walking_sprites[4 * 4];
	FacingDirection facing_direction = FacingDirection::Down;
	bool moving = false;
	int walking_animation_state = 0;
	std::unique_ptr<Coroutine> coroutine;

	void initialize_sprites(const GraphicsAsset &graphics, Renderer &);
	template <typename T>
	void apply_to_all_sprites(const T &f){
		for (int i = 0; i < array_length(this->standing_sprites); i++)
			f(*this->standing_sprites[i]);
		for (int i = 0; i < array_length(this->walking_sprites); i++)
			f(*this->walking_sprites[i]);
	}
	void coroutine_entry_point();
	void handle_movement(InputState &);
	void move(const Point &delta, FacingDirection);
public:
	PlayerCharacter(Game &game, const std::string &name, Renderer &);
	void set_visible_sprite();
	void teleport(Map destination, const Point &position);
	void update();
	DEFINE_GETTER_SETTER(facing_direction)
	DEFINE_GETTER(current_map)
	DEFINE_GETTER(map_position)
};

}
