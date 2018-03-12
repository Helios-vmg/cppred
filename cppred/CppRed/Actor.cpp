#include "Actor.h"
#include "Maps.h"
#include "Game.h"
#include "World.h"
#include <iostream>

static void initialize_sprite(std::shared_ptr<Sprite> &sprite, Renderer &renderer, const GraphicsAsset &graphics, int first_tile, bool flip_x = false){
	sprite = renderer.create_sprite(2, 2);

	if (!flip_x){
		int i = first_tile;
		for (auto &tile : sprite->iterate_tiles()){
			tile.tile_no = graphics.first_tile + i++;
			tile.has_priority = true;
		}
	}else{
		auto iterators = sprite->iterate_tiles();
		assert(iterators.end() - iterators.begin() == 4 && sprite->get_w() == 2 && sprite->get_h() == 2);
		auto i = iterators.begin();
		first_tile += graphics.first_tile;
		for (int y = 0; y < 2; y++){
			for (int x = 0; x < 2; x++){
				auto &tile = *(i++);
				tile.tile_no = first_tile + y * 2 + (1 - x);
				tile.flipped_x = true;
				tile.has_priority = true;
			}
		}
	}
}

namespace CppRed{

Actor::Actor(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite):
		game(&game),
		name(name),
		position(Map::Nowhere),
		sprite(&sprite),
		renderer(&renderer){
	this->coroutine.reset(new Coroutine(
		this->name + " coroutine",
		parent_coroutine.get_clock(),
		[this](Coroutine &){ this->coroutine_entry_point(); }
	));
}

Actor::~Actor(){}

void Actor::init(){
	this->initialize_sprites(*this->sprite, *this->renderer);
}

void Actor::uninit(){
	this->quit_coroutine = true;
	this->update();
	this->coroutine.reset();
}

void Actor::initialize_sprites(const GraphicsAsset &graphics, Renderer &renderer){
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
}

void Actor::coroutine_entry_point(){
	while (!this->quit_coroutine)
		this->coroutine->yield();
}

void Actor::set_visible_sprite(){
	this->apply_to_all_sprites([](auto &sprite){ sprite.set_visible(false); });

	Sprite *sprite;
	if (!this->moving)
		sprite = this->standing_sprites[(int)this->facing_direction].get();
	else
		sprite = this->walking_sprites[(int)this->facing_direction * 4 + this->walking_animation_state].get();
	sprite->set_visible(true);
}

void Actor::update(){
	this->coroutine->resume();
	this->update_sprites();
}

bool Actor::move(FacingDirection direction){
	static const Point deltas[] = {
		{ 0, -1},
		{ 1,  0},
		{ 0,  1},
		{-1,  0},
	};
	return this->move(direction_to_vector(direction), direction);
}

bool Actor::can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction){
	auto &world = this->game->get_world();
	return world.can_move_to(current_position, next_position, direction);
}

bool Actor::move(const Point &delta, FacingDirection direction){
	auto pos0 = this->position;
	auto &world = this->game->get_world();
	auto pos1 = world.remap_coordinates(pos0 + delta);
	this->standing_sprites[(int)this->facing_direction]->set_visible(false);
	this->facing_direction = direction;
	this->standing_sprites[(int)this->facing_direction]->set_visible(true);
	if (!this->can_move_to(pos0, pos1, direction))
		return false;
	this->about_to_move();
	auto &map1 = world.get_map_instance(pos1.map);
	//Note: during movement, both source and destination blocks are occupied by the actor.
	map1.set_cell_occupation(pos1.position, true);
	this->run_walking_animation(delta, direction);
	auto map0 = world.try_get_map_instance(pos0.map);
	if (map0)
		map0->set_cell_occupation(pos0.position, false);
	this->position = pos1;
	if (pos0.map != pos1.map)
		this->entered_new_map(pos0.map, pos1.map, false);
	return true;
}

void Actor::run_walking_animation(const Point &delta, FacingDirection direction){
	//TODO
	auto &E = this->game->get_engine();
	auto &c = this->coroutine->get_clock();
	auto t0 = c.get();
	double t;
	const double frames = this->movement_duration();
	const double movement_per_frame = Renderer::tile_size * 2 / frames;
	while ((t = c.get() - t0) < frames / 60){
		this->pixel_offset = delta * (movement_per_frame * (t * 60));
		this->coroutine->yield();
		c.step();
	}
	this->pixel_offset = Point();
}


void Actor::set_new_screen_owner(std::unique_ptr<ScreenOwner> &&owner){
	this->screen_owner = std::move(owner);
}

std::unique_ptr<ScreenOwner> Actor::get_new_screen_owner(){
	return std::move(this->screen_owner);
}

}
