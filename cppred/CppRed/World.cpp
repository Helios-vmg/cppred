#include "World.h"
#include "PlayerCharacter.h"
#include "Game.h"
#include "PlayerCharacter.h"
#include "../CodeGeneration/output/variables.h"
#include <sstream>
#include <iostream>

namespace CppRed{

World::World(Game &game): ScreenOwner(game){
}

World::~World(){
	for (auto &actor : this->actors)
		actor->stop();
	this->map_store.stop();
	this->player_character->stop();
	this->rival->stop();
}

void World::teleport_player(const WorldCoordinates &destination){
	this->player_character->teleport(destination);
}

void World::teleport_player(const MapWarp &warp){
	auto &destination = warp.get_destination();
	auto index = warp.get_destination_warp_index();
	const MapData *map;
	auto &vs = this->game->get_variable_store();
	if (destination.simple)
		map = destination.destination_map;
	else{
		auto map_id = vs.get(destination.variable);
		map = this->map_store.try_get_map_by_legacy_id(map_id);
		if (!map)
			map = &this->map_store.get_map_data(Map::PalletTown);
	}
	for (auto &object : *map->objects){
		auto casted = dynamic_cast<MapWarp *>(object.get());
		if (!casted)
			continue;
		if (casted->get_index() == index){
			auto &map2 = this->map_store.get_map_data(this->player_character->get_current_map());
			if (map2.tileset->type == TilesetType::Outdoor)
				vs.set(IntegerVariableId::LastOutdoorsMap, map2.legacy_id);
			this->teleport_player({map->map_id, casted->get_position()});
			return;
		}
	}
	std::stringstream stream;
	stream << "Invalid map warp (" << warp.get_name() << "). Destination not found.";
	throw std::runtime_error(stream.str());
}

void World::set_camera_position(){
	this->camera_position = this->player_character->get_map_position() - PlayerCharacter::screen_block_offset;
	this->pixel_offset = this->player_character->get_pixel_offset();
}

static bool point_in_rectangle(const Point &p, int w, int h){
	return p.x >= 0 && p.y >= 0 && p.x < w && p.y < h;
}

static bool point_in_map(const Point &p, const MapData &map_data){
	return point_in_rectangle(p, map_data.width, map_data.height);
}

int reduce_by_region(int x, int begin, int end){
	if (x < begin)
		return -1;
	if (x >= end)
		return 1;
	return 0;
}

Point reduce_by_region(const Point &p, const MapData &m){
	return {
		reduce_by_region(p.x, 0, m.width),
		reduce_by_region(p.y, 0, m.height),
	};
}

bool in_applicable_region(const Point &reduced, const Point &region){
	return (!region.x || region.x == reduced.x) && (!region.y || region.y == reduced.y);
}

std::pair<const MapData *, Point> compute_map_connections(const WorldCoordinates &position, const MapData &map_data, MapStore &map_store){
	struct SimplifiedCheck{
		Point applicable_region;
		int x_multiplier_1;
		int x_multiplier_2;
		int y_multiplier_1;
		int y_multiplier_2;
	};
	static const SimplifiedCheck checks[] = {
		{ { 0, -1}, 1,  0, 0,  1, },
		{ { 1,  0}, 0, -1, 1,  0, },
		{ { 0,  1}, 1,  0, 0, -1, },
		{ {-1,  0}, 0,  1, 1,  0, },
	};
	auto reduced = reduce_by_region(position.position, map_data);
	for (size_t i = 0; i < array_length(checks); i++){
		if (!map_data.map_connections[i])
			continue;
		auto &check = checks[i];
		if (!in_applicable_region(reduced, check.applicable_region))
			continue;
		auto &mc = map_data.map_connections[i];
		auto &map_data2 = map_store.get_map_data(mc.destination);
		auto transformed = position.position;
		transformed.x += (mc.local_position - mc.remote_position) * check.x_multiplier_1;
		if (check.x_multiplier_2 > 0)
			transformed.x += map_data2.width * check.x_multiplier_2;
		else if (check.x_multiplier_2 < 0)
			transformed.x += map_data.width * check.x_multiplier_2;
		transformed.y += (mc.local_position - mc.remote_position) * check.y_multiplier_1;
		if (check.y_multiplier_2 > 0)
			transformed.y += map_data2.height * check.y_multiplier_2;
		else if (check.y_multiplier_2 < 0)
			transformed.y += map_data.height * check.y_multiplier_2;
		return {&map_data2, transformed};
	}
	return {nullptr, position.position};
}

std::pair<TilesetData *, int> World::compute_virtual_block(const WorldCoordinates &position, bool &border_visible){
	auto transformed = this->remap_coordinates(position);
	auto &map_data = this->map_store.get_map_data(position.map);
	auto &transformed_map_data = this->map_store.get_map_data(transformed.map);
	if (point_in_map(transformed.position, transformed_map_data))
		return {transformed_map_data.tileset.get(), transformed_map_data.get_block_at_map_position(transformed.position)};
	border_visible = true;
	if (this->visible_border_block.second < 0)
		this->visible_border_block = {map_data.tileset.get(), map_data.border_block};
	return this->visible_border_block;
}

WorldCoordinates World::remap_coordinates(const WorldCoordinates &position_parameter){
	auto position = position_parameter;
	auto *map_data = &this->map_store.get_map_data(position.map);
	while (true){
		if (point_in_map(position.position, *map_data))
			return position;
		auto transformed = compute_map_connections(position, *map_data, this->map_store);
		if (!transformed.first)
			return position;
		map_data = transformed.first;
		position.map = map_data->map_id;
		position.position = transformed.second;
	}
}

MapInstance *World::try_get_map_instance(Map map){
	return this->map_store.try_get_map_instance(map, *this->game);
}

MapInstance &World::get_map_instance(Map map){
	return this->map_store.get_map_instance(map, *this->game);
}

const MapInstance &World::get_map_instance(Map map) const{
	return this->map_store.get_map_instance(map, *this->game);
}

bool World::is_passable(const WorldCoordinates &point){
	auto &map_data = this->map_store.get_map_data(point.map);
	if (!point_in_map(point.position, map_data))
		return false;
	auto tile = map_data.get_partial_tile_at_actor_position(point.position);
	auto &v = map_data.tileset->collision->data;
	auto it = find_first_true(v.begin(), v.end(), [tile](byte_t b){ return tile <= b; });
	return it != v.end() && *it == tile;
}

bool World::can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction, bool ignore_occupancy){
	//TODO: Implement full movement check logic.
	return this->can_move_to_land(current_position, next_position, direction, ignore_occupancy);
}

bool World::can_move_to_land(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction, bool ignore_occupancy){
	//TODO: Implement full movement check logic.
	auto &map_data = this->map_store.get_map_data(next_position.map);
	if (!point_in_map(next_position.position, map_data))
		return false;
	if (!ignore_occupancy && this->map_store.get_map_instance(next_position.map, *this->game).get_cell_occupation(next_position.position))
		return false;
	if (!this->check_jumping_and_tile_pair_collisions(current_position, next_position, direction, &TilesetData::impassability_pairs))
		return false;
	if (!this->is_passable(next_position))
		return false;
	return true;
}

bool World::check_jumping_and_tile_pair_collisions(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction, pairs_t pairs){
	if (!this->check_jumping(current_position, next_position, direction))
		return false;
	return this->check_tile_pair_collisions(current_position, next_position, pairs);
}

bool World::check_jumping(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction){
	auto &map_data = this->map_store.get_map_data(next_position.map);
	if (!point_in_map(next_position.position, map_data))
		return false;
	//TODO: See HandleLedges
	return true;
}

bool World::check_tile_pair_collisions(const WorldCoordinates &pos0, const WorldCoordinates &pos1, pairs_t pairs){
	auto &map_data0 = this->map_store.get_map_data(pos0.map);
	auto &map_data1 = this->map_store.get_map_data(pos1.map);
	if (map_data0.tileset != map_data1.tileset || !point_in_map(pos0.position, map_data0) || !point_in_map(pos1.position, map_data1))
		return true;
	auto tile0 = map_data0.get_partial_tile_at_actor_position(pos0.position);
	auto tile1 = map_data1.get_partial_tile_at_actor_position(pos1.position);
	for (auto &pair : (*map_data0.tileset).*pairs){
		if (pair.first < 0)
			break;
		if (pair.first == tile0 && pair.second == tile1 || pair.first == tile1 && pair.second == tile0)
			return false;
	}
	return true;
}

void World::entered_map(Map old_map, Map new_map, bool warped){
	if (warped)
		this->visible_border_block = {nullptr, -1};
	this->map_store.release_map_instance(old_map);
	auto &instance = this->map_store.get_map_instance(new_map, *this->game);
	this->current_map = &instance;
	this->actors.clear();
	auto &map_data = this->map_store.get_map_data(new_map);
	auto &engine = this->game->get_engine();
	auto &renderer = engine.get_renderer();
	auto &array = map_data.sprite_visibility_flags;
	for (MapObjectInstance &object_instance : instance.get_objects()){
		auto &object = object_instance.get_object();
		if (object.requires_actor()){
			auto actor = object.create_actor(*this->game, renderer, new_map, object_instance);
			if (!actor)
				continue;
			auto legacy = object.get_legacy_id();
			if (legacy >= 0 && legacy < array_length(array) && array[legacy] != VisibilityFlagId::None){
				auto visible = this->game->get_variable_store().get(array[legacy]);
				actor->set_visible(visible);
			}
			object_instance.set_actor(*actor);
			this->actors.emplace_back(std::move(actor));
		}
	}
	game->get_audio_interface().play_sound(map_data.music == AudioResourceId::None ? AudioResourceId::Stop : map_data.music);
	instance.loaded(*this->game);
}

bool World::get_objects_at_location(MapObjectInstance *(&dst)[8], const WorldCoordinates &location){
	auto &instance = this->map_store.get_map_instance(location.map, *this->game);
	size_t count = 0;
	for (MapObjectInstance &object : instance.get_objects()){
		if (object.get_position() == location.position){
			if (count == array_length(dst))
				return true;
			dst[count++] = &object;
		}
	}
	std::fill(dst + count, dst + array_length(dst), nullptr);
	return false;
}

std::unique_ptr<ScreenOwner> World::update(){
	this->current_map->update(*this->game);
	this->player_character->update();
	auto ret = this->player_character->get_new_screen_owner();
	if (ret)
		return ret;
	this->set_camera_position();
	for (auto &actor : this->actors){
		actor->update();
		ret = actor->get_new_screen_owner();
		if (ret)
			return ret;
	}
	return nullptr;
}

std::unique_ptr<ScreenOwner> World::run(){
	auto &renderer = this->game->get_engine().get_renderer();
	renderer.set_enable_bg(true);
	renderer.set_enable_sprites(true);
	renderer.set_palette(PaletteRegion::Background, default_palette);
	renderer.set_palette(PaletteRegion::Sprites0, default_world_sprite_palette);
	auto &coroutine = Coroutine::get_current_coroutine();
	while (true){
		auto ret = this->update();
		if (ret)
			return ret;
		this->render(renderer);
		coroutine.yield();
	}
	assert(false);
	return nullptr;
}

void World::create_main_characters(const std::string &player_name, const std::string &rival_name){
	auto &engine = this->game->get_engine();
	auto &co = Coroutine::get_current_coroutine();
	this->player_character = create_actor<PlayerCharacter>(*this->game, co, player_name, engine.get_renderer());
	this->rival = create_actor<Trainer>(*this->game, co, rival_name, engine.get_renderer(), BlueSprite);
}

bool World::facing_edge_of_map(const WorldCoordinates &pos, FacingDirection dir) const{
	auto &map = this->map_store.get_map_data(pos.map);
	if (!(!pos.position.x || !pos.position.y || pos.position.x == map.width - 1 || pos.position.y == map.height - 1))
		return false;
	auto pos2 = pos.position + direction_to_vector(dir);
	return !point_in_map(pos2, map);
}

void World::render(Renderer &renderer){
	auto &bg = renderer.get_tilemap(TileRegion::Background);
	renderer.set_bg_global_offset(Point(Renderer::tile_size * 2, Renderer::tile_size * 2) + this->pixel_offset);
	auto current_map = this->player_character->get_current_map();
	this->player_character->set_visible_sprite();
	if (current_map == Map::Nowhere){
		renderer.fill_rectangle(TileRegion::Background, { 0, 0 }, { Tilemap::w, Tilemap::h }, Tile());
	}else{
		auto &map_data = this->map_store.get_map_data(current_map);
		auto pos = this->player_character->get_map_position();
		const int k = 2;
		bool border_visible = false;
		for (int y = -k; y < Renderer::logical_screen_tile_height + k; y++){
			for (int x = -k; x < Renderer::logical_screen_tile_width + k; x++){
				auto &tile = bg.tiles[(x + k) + (y + k) * Tilemap::w];
				int x2 = (x + k) / 2 - (k / 2) - PlayerCharacter::screen_block_offset.x + pos.x;
				int y2 = (y + k) / 2 - (k / 2) - PlayerCharacter::screen_block_offset.y + pos.y;
				Point map_position(x2, y2);
				auto computed_block = this->compute_virtual_block({current_map, map_position}, border_visible);
				auto &blockset = computed_block.first->blockset->data;
				auto tileset = computed_block.first->tiles;
				auto block = computed_block.second;
				auto offset = euclidean_modulo_u(x, 2) + euclidean_modulo_u(y, 2) * 2;
				tile.tile_no = tileset->first_tile + blockset[block * 4 + offset];
				tile.flipped_x = false;
				tile.flipped_y = false;
				tile.palette = null_palette;
			}
		}
		if (!border_visible)
			this->visible_border_block = {nullptr, -1};
	}
}

void World::pause(){
	for (auto &actor : this->actors)
		actor->pause();
	if (this->current_map)
		this->current_map->pause();
}

Actor &World::get_actor(const char *name){
	for (auto &actor : this->actors)
		if (actor->get_name() == name)
			return *actor;
	throw std::runtime_error((std::string)"Actor not found: " + name);
}

}
