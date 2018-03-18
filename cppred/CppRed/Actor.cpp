#include "Actor.h"
#include "Maps.h"
#include "Game.h"
#include "World.h"
#include <set>
#include <deque>
#include <utility>

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
	if (this->coroutine){
		this->coroutine->get_clock().step();
		this->coroutine->resume();
	}
	this->update_sprites();
}

bool Actor::move(FacingDirection direction){
	auto coroutine = Coroutine::get_current_coroutine_ptr();
	if (coroutine == this->coroutine.get())
		return this->move_internal(direction);
	int result = -1;
	this->saved_actions.push_back([this, &result, direction](){
		result = this->move_internal(direction);
	});
	do
		coroutine->yield();
	while (result < 0);
	return !!result;
}

bool Actor::move_internal(FacingDirection direction){
	return this->move(direction_to_vector(direction), direction);
}

bool Actor::can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction){
	auto &world = this->game->get_world();
	return world.can_move_to(current_position, next_position, direction, this->ignore_occupancy);
}

bool Actor::move(const Point &delta, FacingDirection direction){
	auto pos0 = this->position;
	auto &world = this->game->get_world();
	auto pos1 = world.remap_coordinates(pos0 + delta);
	this->set_facing_direction(direction);
	if (!this->can_move_to(pos0, pos1, direction))
		return false;
	this->about_to_move();
	this->moving = true;
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
	this->moving = false;
	return true;
}

void Actor::run_walking_animation(const Point &delta, FacingDirection direction){
	//TODO
	auto &E = this->game->get_engine();
	auto &coroutine = Coroutine::get_current_coroutine();
	auto &c = coroutine.get_clock();
	auto t0 = c.get();
	double t;
	const double frames = this->movement_duration();
	const double movement_per_frame = Renderer::tile_size * 2 / frames;
	while ((t = c.get() - t0) < frames / 60){
		this->pixel_offset = delta * (movement_per_frame * (t * 60));
		coroutine.yield();
	}
	this->pixel_offset = Point();
}


void Actor::set_new_screen_owner(std::unique_ptr<ScreenOwner> &&owner){
	this->screen_owner = std::move(owner);
}

std::unique_ptr<ScreenOwner> Actor::get_new_screen_owner(){
	return std::move(this->screen_owner);
}

void Actor::set_facing_direction(FacingDirection direction){
	this->standing_sprites[(int)this->facing_direction]->set_visible(false);
	this->facing_direction = direction;
	this->standing_sprites[(int)this->facing_direction]->set_visible(this->visible);
}

void Actor::set_visible(bool visible){
	if (visible == this->visible)
		return;
	this->visible = visible;
	this->standing_sprites[(int)this->facing_direction]->set_visible(visible);
	this->game->get_world()
		.get_map_instance(this->position.map)
		.set_cell_occupation(this->position.position, visible);
}

void Actor::pause(){
	this->coroutine->get_clock().pause();
}

std::vector<PathStep> Actor::find_path(const Point &destination){
	std::set<Point> seen;
	std::deque<std::pair<Point, std::vector<PathStep>>> queue;
	queue.emplace_back(this->position.position, std::vector<PathStep>());
	seen.insert(this->position.position);
	while (queue.size()){
		auto point = std::move(queue.front());
		queue.pop_front();
		for (int i = 0; i < 4; i++){
			auto dir = (FacingDirection)i;
			auto delta = direction_to_vector(dir);
			auto new_position = point.first + delta;
			if (seen.find(new_position) != seen.end())
				continue;
			if (!this->can_move_to({this->position.map, point.first}, {this->position.map, new_position}, dir))
				continue;
			seen.insert(new_position);
			auto copy = point.second;
			copy.push_back({dir, new_position});
			if (new_position == destination)
				return copy;
			queue.emplace_back(new_position, std::move(copy));
		}
	}
	throw std::runtime_error("Could not find path.");
}

void Actor::follow_path(const std::vector<PathStep> &steps){
	this->ignore_occupancy = true;
	auto &world = this->game->get_world();
	auto map = world.try_get_map_instance(this->position.map);
	for (auto &step : steps)
		map->set_cell_occupation(step.after_state, true);
	for (auto &step : steps)
		this->move(step.movement_direction);
	this->ignore_occupancy = false;
}

bool Actor::run_saved_actions(){
	if (!this->saved_actions.size())
		return true;
	auto f = std::move(this->saved_actions.front());
	this->saved_actions.pop_front();
	f();
	return false;
}

NonPlayerActor::NonPlayerActor(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &instance):
		Actor(game, parent_coroutine, name, renderer, sprite){
	this->object_instance = &instance;
}

void NonPlayerActor::coroutine_entry_point(){
	this->standing_sprites[(int)this->facing_direction]->set_visible(this->visible);
	HighResolutionClock real_time;
	auto &clock = this->coroutine->get_clock();
	while (!this->quit_coroutine){
		if (!this->run_saved_actions())
			continue;
		this->coroutine->yield();
	}
}

void NonPlayerActor::update_sprites(){
	auto &world = this->game->get_world();
	auto cam = world.get_camera_position();
	auto po = world.get_pixel_offset();
	auto position = (this->position.position - cam) * (Renderer::tile_size * 2) - po + this->pixel_offset;
	position.y -= Renderer::tile_size / 2;
	this->apply_to_all_sprites([&position](Sprite &sprite){ sprite.set_position(position); });
}

}
