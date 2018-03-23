#include "MainMenu.h"
#include "Game.h"
#include "MiscClasses.h"
#include "Engine.h"
#include "Renderer.h"
#include "../CodeGeneration/output/audio.h"

namespace CppRed{
namespace Scripts{

void show_options(CppRed::Game &game){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	auto options = game.get_options();
	AutoRendererPusher pusher(renderer);
	renderer.clear_screen();
	renderer.clear_sprites();

	for (int i = 0; i < 3; i++)
		game.draw_box({ 0, i * 5 }, { Renderer::logical_screen_tile_width - 2, 3 }, TileRegion::Background);

	game.put_string({ 1,  1 }, TileRegion::Background, "TEXT SPEED");
	game.put_string({ 1,  6 }, TileRegion::Background, "BATTLE ANIMATION");
	game.put_string({ 1, 11 }, TileRegion::Background, "BATTLE STYLE");
	game.put_string({ 2,  3 }, TileRegion::Background, "FAST  MEDIUM SLOW");
	game.put_string({ 2,  8 }, TileRegion::Background, "ON       OFF");
	game.put_string({ 2, 13 }, TileRegion::Background, "SHIFT    SET");
	game.put_string({ 2, 16 }, TileRegion::Background, "CANCEL");

	const int speed_positions[] = { 1, 7, 14, -1 };
	const int animation_positions[] = { 1, 10, -1 };
	const int style_positions[] = { 1, 10, -1 };
	const int cancel_positions[] = { 1, -1 };
	const std::pair<int, const int *> cursor_positions[] = {
		std::pair<int, const int *>{3, speed_positions},
		std::pair<int, const int *>{8, animation_positions},
		std::pair<int, const int *>{13, style_positions},
		std::pair<int, const int *>{16, cancel_positions},
	};

	int horizontal_cursor_positions[] = {
		(int)options.text_speed / 2,
		!options.battle_animations_enabled,
		(int)options.battle_style,
		0,
	};
	int vertical_cursor_position = 0;
	auto tilemap = renderer.get_tilemap(TileRegion::Background).tiles;
	bool done = false;
	auto &coroutine = Coroutine::get_current_coroutine();
	while (!done){
		int i;
		for (auto &vpos : cursor_positions){
			auto y = vpos.first;
			for (i = 0; ; i++){
				auto x = vpos.second[i];
				if (x < 0)
					break;
				tilemap[x + y * Tilemap::w].tile_no = ' ';
			}
		}
		i = 0;
		for (auto pos : horizontal_cursor_positions){
			auto y = cursor_positions[i].first;
			auto x = cursor_positions[i].second[horizontal_cursor_positions[i]];
			i++;
			tilemap[x + y * Tilemap::w].tile_no = white_arrow;
		}
		{
			auto y = cursor_positions[vertical_cursor_position].first;
			auto x = cursor_positions[vertical_cursor_position].second[horizontal_cursor_positions[vertical_cursor_position]];
			tilemap[x + y * Tilemap::w].tile_no = black_arrow;
		}

		while (true){
			coroutine.yield();
			auto input = game.joypad_auto_repeat();
			if (input.get_left()){
				if (!horizontal_cursor_positions[vertical_cursor_position])
					while (cursor_positions[vertical_cursor_position].second[horizontal_cursor_positions[vertical_cursor_position]] >= 0)
						horizontal_cursor_positions[vertical_cursor_position]++;
				horizontal_cursor_positions[vertical_cursor_position]--;
				break;
			}
			if (input.get_right()){
				horizontal_cursor_positions[vertical_cursor_position]++;
				if (cursor_positions[vertical_cursor_position].second[horizontal_cursor_positions[vertical_cursor_position]] < 0)
					horizontal_cursor_positions[vertical_cursor_position] = 0;
				break;
			}
			if (input.get_up()){
				vertical_cursor_position--;
				vertical_cursor_position = euclidean_modulo(vertical_cursor_position, (int)array_length(horizontal_cursor_positions));
				break;
			}
			if (input.get_down()){
				vertical_cursor_position++;
				vertical_cursor_position = euclidean_modulo(vertical_cursor_position, (int)array_length(horizontal_cursor_positions));
				break;
			}
			if (input.get_b() || input.get_a() && vertical_cursor_position == array_length(horizontal_cursor_positions) - 1){
				done = true;
				break;
			}
		}
	}

	options.text_speed = (CppRed::TextSpeed)(horizontal_cursor_positions[0] * 2 + 1);
	options.battle_animations_enabled = !horizontal_cursor_positions[1];
	options.battle_style = (CppRed::BattleStyle)horizontal_cursor_positions[2];
	game.set_options(options);
	game.set_options_initialized(true);
	game.get_audio_interface().play_sound(AudioResourceId::SFX_Press_AB);
}

MainMenuResult main_menu(Game &game){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	auto save = game.load_save();

	renderer.set_default_palettes();

	int selection = -1;
	if (save && !save->valid){
		//TODO: Report corruption
		save.reset();
	}

	int delta = 0;
	std::vector<std::string> items;
	if (save)
		items.push_back("CONTINUE");
	else
		delta = 1;
	items.push_back("NEW GAME");
	items.push_back("OPTION");
	while (true){
		StandardMenuOptions options;
		options.items = &items;
		options.minimum_size = {13,0};
		selection = game.handle_standard_menu(options);
		selection += (selection >= 0) * delta;
		
		if (selection < 0)
			return MainMenuResult::GoToTitleScreen;

		switch (selection){
			case 0:
				return MainMenuResult::ContinueGame;
			case 1:
				game.get_audio_interface().wait_for_sfx_to_end();
				return MainMenuResult::NewGame;
		}

		show_options(game);
		renderer.clear_screen();
	}
}

}
}
