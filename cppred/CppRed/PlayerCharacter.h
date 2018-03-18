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

class PlayerCharacter : public Actor, public Trainer{
public:
	static const Point screen_block_offset;
private:
	typedef bool (PlayerCharacter::*warp_check_f)(const World &) const;
	static const warp_check_f warp_check_functions[2];
	const MapWarp *saved_post_warp = nullptr;
	bool ignore_input = false;

	void coroutine_entry_point() override;
	void entered_new_map(Map old_map, Map new_map, bool warped) override;
	void initialize_sprites(const GraphicsAsset &graphics, Renderer &) override;
	bool run_warp_logic_no_collision();
	bool run_warp_logic_collision();
	bool facing_edge_of_map(const World &) const;
	bool is_in_front_of_warp_tile(const World &) const;
	void about_to_move() override;
	static FacingDirection input_to_direction(const InputState &input);
	bool move_internal(FacingDirection) override;
	void check_for_bookshelf_or_card_key_door();
public:
	PlayerCharacter(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &);
	void teleport(const WorldCoordinates &);
	DEFINE_GETTER_SETTER(ignore_input)
};

}
