#pragma once

#include "Maps.h"
#include "Actor.h"
#include "ScreenOwner.h"

namespace CppRed{

class PlayerCharacter;
class Trainer;
class Game;

class World : public ScreenOwner{
	actor_ptr<PlayerCharacter> player_character = null_actor_ptr<PlayerCharacter>();
	actor_ptr<Trainer> rival = null_actor_ptr<Trainer>();
	MapStore map_store;
	std::vector<actor_ptr<Actor>> actors;
	Point camera_position;
	Point pixel_offset;
	std::pair<TilesetData *, int> visible_border_block = {nullptr, -1};
	MapInstance *current_map = nullptr;

	bool is_passable(const WorldCoordinates &);
	typedef decltype(&TilesetData::impassability_pairs) pairs_t;
	bool check_jumping_and_tile_pair_collisions(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection, pairs_t pairs);
	bool check_jumping(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection);
	bool check_tile_pair_collisions(const WorldCoordinates &current_position, const WorldCoordinates &next_position, pairs_t pairs);
	bool can_move_to_land(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction);
	bool can_move_to_water(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction);
	std::pair<TilesetData *, int> compute_virtual_block(const WorldCoordinates &position, bool &border_visible);
	void set_camera_position();
	std::unique_ptr<ScreenOwner> update();
	void render(Renderer &);
public:
	World(Game &game);
	MapStore &get_map_store(){
		return this->map_store;
	}
	void teleport_player(const MapWarp &);
	void teleport_player(const WorldCoordinates &);
	MapInstance &get_map_instance(Map);
	MapInstance *try_get_map_instance(Map);
	const MapInstance &get_map_instance(Map) const;
	bool get_objects_at_location(MapObjectInstance *(&dst)[8], const WorldCoordinates &);
	std::unique_ptr<ScreenOwner> run() override;
	WorldCoordinates remap_coordinates(const WorldCoordinates &position_parameter);
	bool can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection);
	void entered_map(Map old_map, Map new_map, bool warped);
	void create_main_characters(const std::string &player_name, const std::string &rival_name);
	bool facing_edge_of_map(const WorldCoordinates &, FacingDirection) const;
	void pause() override;
	PlayerCharacter &get_pc(){
		return *this->player_character;
	}
	Actor &get_actor(const char *);

	DEFINE_GETTER(camera_position)
	DEFINE_GETTER(pixel_offset)
};

}
