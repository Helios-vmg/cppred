#include "stdafx.h"
#include "BattleOwner.h"
#include "Game.h"
#include "Trainer.h"
#include "World.h"
#include "PlayerCharacter.h"
#include "../Coroutine.h"
#include "../../CodeGeneration/output/audio.h"
#include "../../CodeGeneration/output/variables.h"

namespace CppRed{

BattleOwner::BattleOwner(Game &game, FullTrainerClass &&ftc) :
	ScreenOwner(game),
	trainer_class(std::move(ftc)){

	this->done = false;
	this->coroutine.reset(new Coroutine("BattleOwner coroutine", game.get_coroutine().get_clock(), [this](Coroutine &){
		this->coroutine_entry_point();
	}));

}

static const Point opponent_balls_display_corner(1, 2);
static const Point player_balls_display_corner(9, 10);
static const Point balls_display_size(10, 2);

void BattleOwner::coroutine_entry_point(){
	auto &audio = this->game->get_audio_interface();
	auto &engine = this->game->get_engine();
	auto &renderer = engine.get_renderer();
	auto &vs = this->game->get_variable_store();
	auto &coroutine = Coroutine::get_current_coroutine();
	auto &trainer_class = this->trainer_class;
	auto &player = this->game->get_world().get_pc();
	ComputerTrainer ct(trainer_class, engine.get_prng());

	audio.play_sound(AudioResourceId::Music_TrainerBattle);
	this->battle_transition_animation();

	AutoRendererPusher pusher(renderer);
	renderer.clear_screen();
	renderer.clear_sprites();

	this->game->draw_box(standard_dialogue_box_position, standard_dialogue_box_size, TileRegion::Background);

	this->trainers_animation();

	const Point red_position(1, 5);
	const Point opponent_position(12, 0);

	auto red_tiles = renderer.draw_image_to_tilemap(red_position, RedPicBack);
	this->game->draw_box(standard_dialogue_box_position, standard_dialogue_box_size, TileRegion::Background);
	std::vector<Point> opponent_tiles = renderer.draw_image_to_tilemap(opponent_position, trainer_class.get_graphics());
	this->display_opponent_balls(ct);
	this->display_player_balls(player);
	audio.play_sound(AudioResourceId::SFX_Silph_Scope);

	vs.set(StringVariableId::wTrainerName, trainer_class.get_display_name());
	this->game->run_dialogue(TextResourceId::TrainerWantsToFightText, false, true);

	bool first_round = true;
	while (true){
		bool current_is_first_rount = first_round;
		first_round = false;
		if (!ct.get_party().get_first_usable_pokemon()){
			this->result = BattleResult::PlayerVictory;
			break;
		}
		if (!ct.get_party().get_first_usable_pokemon()){
			this->result = BattleResult::PlayerDefeat;
			break;
		}
		
		if (current_is_first_rount){
			renderer.fill_rectangle(TileRegion::Background, opponent_balls_display_corner, balls_display_size, Tile());
			renderer.fill_rectangle(TileRegion::Background, player_balls_display_corner, balls_display_size, Tile());
			this->opponent_moves_back(std::move(opponent_tiles));
		}

		auto &opponent_pokemon = *ct.get_party().get_first_usable_pokemon();
		auto &opponent_pokemon_data = opponent_pokemon.get_data();
		vs.set(StringVariableId::wEnemyMonNick, opponent_pokemon.get_display_name());
		this->game->run_dialogue(TextResourceId::TrainerSentOutText, false, false);
		this->game->draw_portrait(*opponent_pokemon_data.front, TileRegion::Background, opponent_position);
		audio.play_cry(opponent_pokemon_data.species_id);
		audio.wait_for_sfx_to_end();
		this->display_opponent_pokemon_status(opponent_pokemon);

		auto &red_pokemon = *ct.get_party().get_first_usable_pokemon();
		auto &red_pokemon_data = red_pokemon.get_data();

		if (current_is_first_rount){
			this->player_moves_back(std::move(red_tiles));
		}

		auto &reds_pokemon = *player.get_party().get_first_usable_pokemon();
		auto &reds_pokemon_data = reds_pokemon.get_data();
		this->display_player_pokemon_status(reds_pokemon);
		vs.set(StringVariableId::wBattleMonNick, reds_pokemon.get_display_name());
		auto enemy_percentage_health = opponent_pokemon.get_current_hp() * 25 / (opponent_pokemon.get_max_hp() / 4);
		TextResourceId go_text;
		if (enemy_percentage_health >= 70)
			go_text = TextResourceId::GoText;
		else if (enemy_percentage_health >= 40)
			go_text = TextResourceId::DoItText;
		else if (enemy_percentage_health >= 10)
			go_text = TextResourceId::GetmText;
		else
			go_text = TextResourceId::EnemysWeakText;
		this->game->reset_dialogue_state(false);
		this->game->run_dialogue(go_text, false, false);
		this->game->draw_portrait(*reds_pokemon_data.back, TileRegion::Background, red_position);
		this->game->draw_box(standard_dialogue_box_position, standard_dialogue_box_size, TileRegion::Background);
		audio.play_cry(reds_pokemon_data.species_id);
		audio.wait_for_sfx_to_end();
		this->game->reset_dialogue_state(true);
		
		while (this->run_one_round(reds_pokemon, opponent_pokemon));
	}
}

void BattleOwner::battle_transition_animation(){
	//TODO
	Coroutine::get_current_coroutine().wait(3);
}

void BattleOwner::trainers_animation(){
	//TODO
	Coroutine::get_current_coroutine().wait(2.5);
}

void BattleOwner::opponent_moves_back(std::vector<Point> &&opponent_tiles){
	//TODO
	auto &engine = this->game->get_engine();
	auto &renderer = engine.get_renderer();
	renderer.mass_set_tiles(opponent_tiles, Tile());
}

void BattleOwner::player_moves_back(std::vector<Point> &&red_tiles){
	//TODO
	auto &engine = this->game->get_engine();
	auto &renderer = engine.get_renderer();
	renderer.mass_set_tiles(red_tiles, Tile());
}

void BattleOwner::display_balls(Trainer &trainer, const Point &placement, int increment, const GraphicsAsset &layout){
	auto &engine = this->game->get_engine();
	auto &renderer = engine.get_renderer();
	renderer.draw_image_to_tilemap(placement, layout);
	Point ball_position = placement + Point(increment > 0 ? 2 : 7, 0);
	for (Pokemon &p : trainer.get_party().iterate()){
		int offset = 1;
		auto status = p.get_status();
		if (status == StatusCondition2::Normal)
			offset = 0;
		else if (status == StatusCondition2::Fainted)
			offset = 2;
		renderer.get_tile(TileRegion::Background, ball_position).tile_no = PokeballTileGraphics.first_tile + offset;
		ball_position.x += increment;
	}
}

void BattleOwner::display_opponent_balls(Trainer &trainer){
	this->display_balls(trainer, opponent_balls_display_corner, -1, OpponentBallsBattleLayout);
}

void BattleOwner::display_player_balls(Trainer &trainer){
	this->display_balls(trainer, player_balls_display_corner, 1, PlayerBallsBattleLayout);
}

void BattleOwner::display_opponent_pokemon_status(Pokemon &pokemon){
	auto &game = *this->game;
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	renderer.put_string({1, 0}, TileRegion::Background, pokemon.get_display_name(), max_pokemon_name_size);
	renderer.draw_image_to_tilemap({1, 1}, OpponentPokemonBattleLayout);
	renderer.put_string({5, 1}, TileRegion::Background, number_to_decimal_string(pokemon.get_level()).data(), 3);
	renderer.draw_bar({4, 2}, TileRegion::Background, 6, pokemon.get_max_hp(), pokemon.get_current_hp());
}

void BattleOwner::display_player_pokemon_status(Pokemon &pokemon){
	auto &game = *this->game;
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	renderer.put_string({10, 7}, TileRegion::Background, pokemon.get_display_name(), max_pokemon_name_size);
	renderer.draw_image_to_tilemap({9, 8}, PlayerPokemonBattleLayout);
	renderer.put_string({15, 8}, TileRegion::Background, number_to_decimal_string(pokemon.get_level()).data(), 3);
	renderer.draw_bar({12, 9}, TileRegion::Background, 6, pokemon.get_max_hp(), pokemon.get_current_hp());
	renderer.put_string({11, 10}, TileRegion::Background, number_to_decimal_string(pokemon.get_current_hp(), 3).data());
	renderer.put_string({15, 10}, TileRegion::Background, number_to_decimal_string(pokemon.get_max_hp(), 3).data());
}
	
bool BattleOwner::run_one_round(Pokemon &own, Pokemon &opponent){
	this->display_battle_menu();
	return true;
}

void BattleOwner::display_battle_menu(){
	auto &game = *this->game;
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	auto &audio = game.get_audio_interface();
	AutoRendererWindowPusher pusher(renderer);
	
	auto &tilemap = renderer.get_tilemap(TileRegion::Window).tiles;
	fill(tilemap, Tile());

	static const Point dialog_corner(8, 12);
	static const Point dialog_size(10, 4);

	this->game->draw_box(dialog_corner, dialog_size, TileRegion::Window);
	renderer.set_window_origin({0, 0});
	renderer.set_window_region_start(dialog_corner * renderer.tile_size);
	renderer.set_window_region_size((dialog_size + Point(2, 2)) * renderer.tile_size);
	renderer.set_enable_window(true);

	renderer.put_string({10, 14}, TileRegion::Window, "FIGHT");
	renderer.put_string({16, 14}, TileRegion::Window, pkmn_string);
	renderer.put_string({10, 16}, TileRegion::Window, "ITEM");
	renderer.put_string({16, 16}, TileRegion::Window, "RUN");

	static const Point arrow_location_lo(9, 14);
	static const Point arrow_location_hi(15, 16);

	int selection = 0;
	auto &coroutine = Coroutine::get_current_coroutine();
	
	game.reset_joypad_state();
	while (true){
		Point arrow_position = arrow_location_lo;
		if (selection % 2)
			arrow_position.x = arrow_location_hi.x;
		if (selection / 2)
			arrow_position.y = arrow_location_hi.y;
		
		auto &tile = tilemap[arrow_position.x + arrow_position.y * Tilemap::w];
		tile.tile_no = black_arrow;

		while (true){
			coroutine.yield();
			auto state = game.joypad_auto_repeat();
			if (state.get_a()){
				audio.play_sound(AudioResourceId::SFX_Press_AB);
				game.reset_joypad_state();
				switch (selection){
					case 0:
						//Show move selection.
					case 1:
						//Show pokemon selection menu.
					case 2:
						//Show inventory.
					case 3:
						//Try to run away.
						break;
				}
				continue;
			}
			if (state.get_right() || state.get_left()){
				selection += selection % 2 * 2 + 1;
				break;
			}
			if (state.get_up() || state.get_down()){
				selection += 2;
				break;
			}
		}

		tile.tile_no = ' ';
		selection = euclidean_modulo_u(selection, 4);
	}

}

}
