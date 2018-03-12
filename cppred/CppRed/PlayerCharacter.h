#pragma once
#include "Trainer.h"
#include "Renderer.h"
#include "utility.h"

class InputState;
class MapWarp;
enum class Map;

namespace CppRed{

class Game;
class World;

class PlayerCharacter : public Trainer{
public:
	static const Point screen_block_offset;
private:
	typedef bool (PlayerCharacter::*warp_check_f)(const World &) const;
	static const warp_check_f warp_check_functions[2];
	const MapWarp *saved_warp = nullptr;

	void coroutine_entry_point() override;
	bool handle_movement(const InputState &);
	void entered_new_map(Map old_map, Map new_map, bool warped) override;
	void initialize_sprites(const GraphicsAsset &graphics, Renderer &) override;
	bool run_warp_logic_no_collision();
	bool run_warp_logic_collision();
	bool facing_edge_of_map(const World &) const;
	bool is_in_front_of_warp_tile(const World &) const;
	bool try_moving(const InputState &input);
	void about_to_move() override;
public:
	PlayerCharacter(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &);
	void teleport(const WorldCoordinates &);
};

}
