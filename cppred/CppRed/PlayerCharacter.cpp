#include "PlayerCharacter.h"
#include "Game.h"
#include <cassert>
#include <complex>

static void initialize_sprite(std::shared_ptr<Sprite> &sprite, Renderer &renderer, const GraphicsAsset &graphics, int first_tile, bool flip_x = false){
	sprite = renderer.create_sprite(2, 2);

	int i = first_tile;
	if (!flip_x){
		for (auto iterators = sprite->iterate_tiles(); iterators.first != iterators.second; ++iterators.first){
			auto &tile = *iterators.first;
			tile.tile_no = graphics.first_tile + i++;
			tile.has_priority = true;
		}
	}else{
		auto iterators = sprite->iterate_tiles();
		assert(iterators.second - iterators.first == 4 && sprite->get_w() == 2 && sprite->get_h() == 2);
		for (int y = 0; y < 2; y++){
			for (int x = 0; x < 2; x++){
				auto &tile = *(iterators.first++);
				tile.tile_no = graphics.first_tile + y * 2 + (1 - x);
				tile.flipped_x = true;
				tile.has_priority = true;
			}
		}
	}
}

namespace CppRed{

void PlayerCharacter::initialize_sprites(const GraphicsAsset &graphics, Renderer &renderer){
	const int sprite_tiles = 4;
	static const int offsets[] = { 1, 2, 0, 2 };
	for (int i = 0; i < 4; i++){
		initialize_sprite(this->standing_sprites[i], renderer, graphics, offsets[i] * sprite_tiles, i == 1);
		this->walking_sprites[i * sprite_tiles + 0] = this->standing_sprites[i];
		this->walking_sprites[i * sprite_tiles + 2] = this->standing_sprites[i];
	}
	initialize_sprite(this->walking_sprites[0 * 4 + 1], renderer, graphics, 4 * sprite_tiles);
	initialize_sprite(this->walking_sprites[0 * 4 + 3], renderer, graphics, 4 * sprite_tiles, true);
	initialize_sprite(this->walking_sprites[1 * 4 + 1], renderer, graphics, 5 * sprite_tiles, true);
	this->walking_sprites[1 * 4 + 3] = this->walking_sprites[1 * 4 + 1];
	initialize_sprite(this->walking_sprites[2 * 4 + 1], renderer, graphics, 3 * sprite_tiles);
	initialize_sprite(this->walking_sprites[2 * 4 + 3], renderer, graphics, 3 * sprite_tiles, true);
	initialize_sprite(this->walking_sprites[3 * 4 + 1], renderer, graphics, 5 * sprite_tiles);
	this->walking_sprites[3 * 4 + 3] = this->walking_sprites[1 * 4 + 1];

	auto s = Renderer::tile_size;
	Point position{ screen_block_offset_x * s * 2, screen_block_offset_y * s * 2 - s / 2 };
	this->apply_to_all_sprites([&position](auto &sprite){ sprite.set_position(position); });
}

PlayerCharacter::PlayerCharacter(Game &game, const std::string &name, Renderer &renderer):
		Trainer(name),
		game(&game),
		current_map(Map::Nowhere){
	this->coroutine.reset(new Coroutine([this](Coroutine &){ this->coroutine_entry_point(); }));
	this->initialize_sprites(RedSprite, renderer);
}

void PlayerCharacter::set_visible_sprite(){
	this->apply_to_all_sprites([](auto &sprite){ sprite.set_visible(false); });

	Sprite *sprite;
	if (!this->moving)
		sprite = this->standing_sprites[(int)this->facing_direction].get();
	else
		sprite = this->walking_sprites[(int)this->facing_direction * 4 + this->walking_animation_state].get();
	sprite->set_visible(true);
}

void PlayerCharacter::teleport(Map destination, const Point &position){
	this->current_map = destination;
	this->map_position = position;
}

void PlayerCharacter::update(){
	this->coroutine->resume();
}

void PlayerCharacter::coroutine_entry_point(){
	while (true){
		auto input = this->game->get_engine().get_input_state();
		if (input.get_start()){
			//handle menu
			this->coroutine->yield();
		}else if (input.any_direction()){
			this->handle_movement(input);
		}else
			this->coroutine->yield();
	}
}

void PlayerCharacter::handle_movement(InputState &input){
	if (input.get_up())
		this->move({0, -1}, FacingDirection::Up);
	else if (input.get_right())
		this->move({1, 0}, FacingDirection::Right);
	else if (input.get_down())
		this->move({0, 1}, FacingDirection::Down);
	else{
		assert(input.get_left());
		this->move({-1, 0}, FacingDirection::Left);
	}
}

void PlayerCharacter::move(const Point &delta, FacingDirection direction){
	auto old_map = this->current_map;
	auto old_position = this->map_position;
	auto new_position = old_position + delta;
	auto new_map = old_map;
	switch (this->game->can_move_to(new_map, old_position, new_position, direction)){
		case MoveQueryResult::CantMoveThere:
			this->coroutine->yield();
			return;
		case MoveQueryResult::CanMoveThere:
			break;
		case MoveQueryResult::CanMoveButWillTeleport:
			//TODO: Figure out destination map.
			break;
		case MoveQueryResult::CanMoveButWillChangeMaps:
			//TODO: Figure out destination map.
			break;
		default:
			throw std::runtime_error("Internal error: invalid switch.");
	}
	auto &map_instance = this->game->get_map_instance(new_map);
	//Note: during movement, both source and destination blocks are occupied by the actor.
	map_instance.set_cell_occupation(new_position, true);
	this->coroutine->wait(5.0 / 60.0);
	map_instance.set_cell_occupation(old_position, false);
	this->current_map = new_map;
	if (new_map != old_map){
		//TODO: Do things that happen when the player enters a new map.
	}
	this->map_position = new_position;
}

}
