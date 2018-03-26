#include "stdafx.h"
#include "OakSpeech.h"
#include "Game.h"
#include "../CodeGeneration/output/audio.h"
#include "../CodeGeneration/output/variables.h"
#include "Coroutine.h"

static void fade_in(CppRed::Game &game){
	const Palette palettes[] = {
		BITMAP(01010100),
		BITMAP(10101000),
		BITMAP(11111100),
		BITMAP(11111000),
		BITMAP(11110100),
		BITMAP(11100100),
	};

	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	for (auto &p : palettes){
		renderer.set_palette(PaletteRegion::Background, p);
		Coroutine::get_current_coroutine().wait_frames(10);
	}
}

static void scroll_from_the_right(CppRed::Game &game){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	const auto t = Renderer::tile_size;
	{
		auto &c = Coroutine::get_current_coroutine().get_clock();
		auto t0 = c.get();
		double y;
		do{
			auto t1 = c.get();
			y = (t1 - t0) * 480 - 120;
			if (y > 0)
				y = 0;
			renderer.set_y_bg_offset(4 * t, (4 + 7) * t, { cast_round(y), 0 });
			game.get_coroutine().yield();
		}while (y < 0);
	}
}

static void scroll_portrait(CppRed::Game &game, std::vector<Point> &red_pic, bool direction, const GraphicsAsset &asset){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	const auto t = Renderer::tile_size;
	{
		auto &c = Coroutine::get_current_coroutine().get_clock();
		auto t0 = c.get();
		double y;
		double multiplier = !direction ? -20 : 20;
		do{
			auto t1 = c.get();
			y = (t1 - t0) * multiplier;
			if (!direction){
				if (y < -6)
					y = -6;
			}else{
				if (y > 6)
					y = 6;
			}
			renderer.set_y_bg_offset(4 * t, (4 + 7) * t, { cast_round(y * t), 0 });
			game.get_coroutine().yield();
		}while (!direction ? (y > -6) : (y < 6));
	}
	renderer.set_y_bg_offset(4 * t, (4 + 7) * t, Point{0, 0});
	for (auto p : red_pic)
		renderer.get_tile(TileRegion::Background, p).tile_no = 0;
	red_pic = renderer.draw_image_to_tilemap({ !direction ? 12 : 6, 4 }, asset);
}

const char * const default_names_red[] = {
	"RED",
	"ASH",
	"JACK",
};

const char * const default_names_blue[] = {
	"BLUE",
	"GARY",
	"JOHN",
};

decltype(default_names_red) &get_default_names(PokemonVersion version, bool flip = false){
	return ((version == PokemonVersion::Red) ^ flip) ? default_names_red : default_names_blue;
}

static std::string select_x_name(CppRed::Game &game, bool rival){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	std::vector<std::string> items;
	items.push_back("NEW NAME");
	auto &default_names_player = get_default_names(game.get_version());
	auto &default_names_rival = get_default_names(game.get_version(), true);
	auto &default_names = *(!rival ? &default_names_player : &default_names_rival);
	for (auto s : default_names)
		items.push_back(s);
	auto tilemap_copy = renderer.get_tilemap(TileRegion::Background);
	CppRed::StandardMenuOptions options;
	options.items = &items;
	options.title = "NAME";
	options.minimum_size = {0, 10};
	auto selection = game.handle_standard_menu(options);
	std::string ret;
	if (selection)
		ret = default_names[selection - 1];
	else
		ret = game.get_name_from_user(!rival ? CppRed::NameEntryType::Player : CppRed::NameEntryType::Rival);
	renderer.get_tilemap(TileRegion::Background) = tilemap_copy;
	return ret;
}

static void oak_introduction(CppRed::Game &game){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	game.get_audio_interface().play_sound(AudioResourceId::Stop);
	game.get_audio_interface().play_sound(AudioResourceId::Music_Routes2);
	renderer.clear_screen();
	game.get_coroutine().wait(1);
	renderer.draw_image_to_tilemap({ 6, 4 }, ProfOakPic);
	fade_in(game);

	game.run_dialogue(TextResourceId::OakSpeechText1, false, false);
	game.fade_out_to_white();
	game.clear_screen();
	game.reset_dialogue_state();
	renderer.draw_image_to_tilemap_flipped({ 6, 4 }, *pokemon_by_species_id[(int)SpeciesId::Nidorino]->front);
	scroll_from_the_right(game);
	game.run_dialogue(TextResourceId::OakSpeechText2A, false, false);
	game.get_audio_interface().play_cry(SpeciesId::Nidorina);
	game.run_dialogue(TextResourceId::OakSpeechText2B, false, false);
	game.fade_out_to_white();
	game.clear_screen();
	game.reset_dialogue_state();
}

static std::string select_player_name(CppRed::Game &game){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	auto red_pic = renderer.draw_image_to_tilemap({ 6, 4 }, RedPicFront);
	scroll_from_the_right(game);
	game.run_dialogue(TextResourceId::IntroducePlayerText, false, false);
	scroll_portrait(game, red_pic, false, RedPicFront);
	auto ret = select_x_name(game, false);
	game.get_variable_store().set(CppRed::StringVariableId::temp_player_name, ret);
	scroll_portrait(game, red_pic, true, RedPicFront);
	game.reset_dialogue_state(false);
	game.run_dialogue(TextResourceId::YourNameIsText, false, false);
	game.fade_out_to_white();
	game.clear_screen();
	game.reset_dialogue_state();
	return ret;
}

static std::string select_rival_name(CppRed::Game &game){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	auto blue_pic = renderer.draw_image_to_tilemap({ 6, 4 }, Rival1Pic);
	fade_in(game);
	game.run_dialogue(TextResourceId::IntroduceRivalText, false, false);
	scroll_portrait(game, blue_pic, false, Rival1Pic);
	auto ret = select_x_name(game, true);
	game.get_variable_store().set(CppRed::StringVariableId::temp_rival_name, ret);
	scroll_portrait(game, blue_pic, true, Rival1Pic);
	game.reset_dialogue_state(false);
	game.run_dialogue(TextResourceId::HisNameIsText, false, false);
	game.fade_out_to_white();
	game.clear_screen();
	game.reset_dialogue_state();
	return ret;
}

static void red_closing(CppRed::Game &game){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	renderer.draw_image_to_tilemap({ 6, 4 }, RedPicFront);
	fade_in(game);

	game.run_dialogue(TextResourceId::OakSpeechText3, false, false);
	game.get_audio_interface().play_sound(AudioResourceId::SFX_Shrink);
	Coroutine::get_current_coroutine().wait_frames(4);
	renderer.draw_image_to_tilemap({ 6, 4 }, ShrinkPic1);
	game.get_coroutine().wait(0.5);
	auto to_erase = renderer.draw_image_to_tilemap({ 6, 4 }, ShrinkPic2);
	game.get_coroutine().wait(0.5);
	renderer.mass_set_tiles(to_erase, Tile());
	auto red = renderer.create_sprite(2, 2);
	red->set_visible(true);
	renderer.set_palette(PaletteRegion::Sprites0, default_world_sprite_palette);
	for (int i = 0; i < 4; i++)
		red->get_tile(i % 2, i / 2).tile_no = RedSprite.first_tile + i;
	red->set_position({ 8 * Renderer::tile_size, Renderer::tile_size * (7 * 2 + 1) / 2 });
	game.get_coroutine().wait(0.5);
	game.get_audio_interface().fade_out_music_to_silence(0.5);
	game.fade_out_to_white();
	game.reset_dialogue_state();
	Coroutine::get_current_coroutine().wait(1);
}

namespace CppRed{
namespace Scripts{

NamesChosenDuringOakSpeech oak_speech(Game &game){
	NamesChosenDuringOakSpeech ret;
#ifndef CPPRED_TESTING
	oak_introduction(game);
	ret.player_name = select_player_name(game);
	ret.rival_name = select_rival_name(game);
	red_closing(game);

#else
	game.get_audio_interface().play_sound(AudioResourceId::Stop);
	auto &default_names_player = get_default_names(game.get_version());
	auto &default_names_rival = get_default_names(game.get_version(), true);
	ret.player_name = default_names_player[0];
	ret.rival_name = default_names_rival[0];
#endif
	return ret;
}

}
}
