#include "PlayerCharacter.h"
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

PlayerCharacter::PlayerCharacter(const std::string &name, Renderer &renderer): Trainer(name){
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

void PlayerCharacter::teleport(const MapData *destination, const Point &position){
	this->current_map = destination;
	this->map_position = position;
}

}
