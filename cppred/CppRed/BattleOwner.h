#pragma once

#include "ScreenOwner.h"
#include "../TrainerData.h"
#include "../utility.h"

class FullTrainerClass;

namespace CppRed{
class Trainer;
class Pokemon;

enum class BattleResult{
	None,
	PlayerVictory,
	PlayerDefeat,
};

class BattleOwner : public ScreenOwner{
	FullTrainerClass trainer_class;
	BattleResult result = BattleResult::None;

	void coroutine_entry_point();
	void battle_transition_animation();
	void trainers_animation();
	void opponent_moves_back(std::vector<Point> &&);
	void player_moves_back(std::vector<Point> &&);
	void display_opponent_balls(Trainer &trainer);
	void display_player_balls(Trainer &);
	void display_balls(Trainer &, const Point &placement, int increment, const GraphicsAsset &);
	void display_opponent_pokemon_status(Pokemon &);
	void display_player_pokemon_status(Pokemon &);
	bool run_one_round(Pokemon &own, Pokemon &opponent);
public:
	BattleOwner(Game &game, FullTrainerClass &&);
	void pause() override{}
	DEFINE_GETTER(result)
};

}
