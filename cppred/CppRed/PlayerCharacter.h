#pragma once
#include "Trainer.h"
#include "Renderer.h"
#include "utility.h"

class InputState;
enum class Map;

namespace CppRed{

class Game;

class PlayerCharacter : public Trainer{
public:
	static const Point screen_block_offset;
private:

	void coroutine_entry_point() override;
	bool handle_movement(InputState &);
	void entered_new_map(Map old_map, Map new_map) override;
	void initialize_sprites(const GraphicsAsset &graphics, Renderer &) override;
public:
	PlayerCharacter(Game &game, const std::string &name, Renderer &);
	void teleport(const WorldCoordinates &);
};

}
