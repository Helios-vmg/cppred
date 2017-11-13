#include "CppRedOakSpeech.h"
#include "CppRedGame.h"
#include "../CodeGeneration/output/audio.h"

static void fade_in(CppRedGame &cppred){
	const Palette palettes[] = {
		BITMAP(01010100),
		BITMAP(10101000),
		BITMAP(11111100),
		BITMAP(11111000),
		BITMAP(11110100),
		BITMAP(11100100),
	};

	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	for (auto &p : palettes){
		renderer.set_palette(PaletteRegion::Background, p);
		engine.wait_frames(10);
	}
}

static void scroll_from_the_right(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	const auto t = Renderer::tile_size;
	{
		auto t0 = engine.get_clock();
		double y;
		do{
			auto t1 = engine.get_clock();
			y = (t1 - t0) * 480 - 120;
			if (y > 0)
				y = 0;
			renderer.set_y_bg_offset(4 * t, (4 + 7) * t, { cast_round(y), 0 });
			engine.wait_exactly_one_frame();
		}while (y < 0);
	}
}

static void scroll_portrait(CppRedGame &cppred, std::vector<Point> &red_pic, bool direction, const GraphicsAsset &asset){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	const auto t = Renderer::tile_size;
	{
		auto t0 = engine.get_clock();
		double y;
		double multiplier = !direction ? -20 : 20;
		do{
			auto t1 = engine.get_clock();
			y = (t1 - t0) * multiplier;
			if (!direction){
				if (y < -6)
					y = -6;
			}else{
				if (y > 6)
					y = 6;
			}
			renderer.set_y_bg_offset(4 * t, (4 + 7) * t, { cast_round(y * t), 0 });
			engine.wait_exactly_one_frame();
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

static std::string select_x_name(CppRedGame &cppred, bool rival){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	std::vector<std::string> items;
	items.push_back("NEW NAME");
	auto &default_names_player = cppred.get_version() == PokemonVersion::Red ? default_names_red : default_names_blue;
	auto &default_names_rival = cppred.get_version() == PokemonVersion::Red ? default_names_blue : default_names_red;
	auto &default_names = *(!rival ? &default_names_player : &default_names_rival);
	for (auto s : default_names)
		items.push_back(s);
	auto tilemap_copy = renderer.get_tilemap(TileRegion::Background);
	auto selection = cppred.handle_standard_menu_with_title(TileRegion::Background, { 0, 0 }, items, "NAME", { 0, 10 }, true);
	std::string ret;
	if (selection)
		ret = default_names[selection - 1];
	else
		ret = cppred.get_name_from_user(!rival ? NameEntryType::Player : NameEntryType::Rival);
	renderer.get_tilemap(TileRegion::Background) = tilemap_copy;
	return ret;
}

static void oak_introduction(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	cppred.get_audio_interface().play_sound(AudioResourceId::Stop);
	cppred.get_audio_interface().play_sound(AudioResourceId::Music_Routes2);
	renderer.clear_screen();
	engine.wait(1);
	renderer.draw_image_to_tilemap({ 6, 4 }, ProfOakPic);
	fade_in(cppred);

	cppred.run_dialog(TextResourceId::OakSpeechText1);
	cppred.fade_out_to_white();
	cppred.clear_screen();
	renderer.draw_image_to_tilemap_flipped({ 6, 4 }, *pokemon_by_species_id[(int)SpeciesId::Nidorino]->front);
	scroll_from_the_right(cppred);
	cppred.run_dialog(TextResourceId::OakSpeechText2A);
	cppred.get_audio_interface().play_cry(SpeciesId::Nidorina);
	cppred.run_dialog(TextResourceId::OakSpeechText2B);
	cppred.fade_out_to_white();
	cppred.clear_screen();
}

static std::string select_player_name(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	auto red_pic = renderer.draw_image_to_tilemap({ 6, 4 }, RedPicFront);
	scroll_from_the_right(cppred);
	cppred.run_dialog(TextResourceId::IntroducePlayerText);
	scroll_portrait(cppred, red_pic, false, RedPicFront);
	auto ret = select_x_name(cppred, false);
	cppred.get_variable_store().set_string("temp_player_name", ret);
	scroll_portrait(cppred, red_pic, true, RedPicFront);
	cppred.run_dialog(TextResourceId::YourNameIsText);
	cppred.fade_out_to_white();
	cppred.clear_screen();
	return ret;
}

static std::string select_rival_name(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	auto blue_pic = renderer.draw_image_to_tilemap({ 6, 4 }, Rival1Pic);
	fade_in(cppred);
	cppred.run_dialog(TextResourceId::IntroduceRivalText);
	scroll_portrait(cppred, blue_pic, false, Rival1Pic);
	auto ret = select_x_name(cppred, true);
	cppred.get_variable_store().set_string("temp_rival_name", ret);
	scroll_portrait(cppred, blue_pic, true, Rival1Pic);
	cppred.run_dialog(TextResourceId::HisNameIsText);
	cppred.fade_out_to_white();
	cppred.clear_screen();
	return ret;
}

static void red_closing(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	renderer.draw_image_to_tilemap({ 6, 4 }, RedPicFront);
	fade_in(cppred);

	cppred.run_dialog(TextResourceId::OakSpeechText3);
	cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Shrink);
	engine.wait_frames(4);
	renderer.draw_image_to_tilemap({ 6, 4 }, ShrinkPic1);
	engine.wait(0.5);
	auto to_erase = renderer.draw_image_to_tilemap({ 6, 4 }, ShrinkPic2);
	engine.wait(0.5);
	renderer.mass_set_tiles(to_erase, Tile());
	auto red = renderer.create_sprite(2, 2);
	red->set_visible(true);
	red->set_palette(default_world_sprite_palette);
	for (int i = 0; i < 4; i++)
		red->get_tile(i % 2, i / 2).tile_no = RedSprite.first_tile + i;
	red->set_position({ 8 * Renderer::tile_size, Renderer::tile_size * (7 * 2 + 1) / 2 });
	engine.wait(0.5);
	cppred.fade_out_to_white();
}

namespace CppRedScripts{

NamesChosenDuringOakSpeech oak_speech(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	NamesChosenDuringOakSpeech ret;
	oak_introduction(cppred);
	ret.player_name = select_player_name(cppred);
	ret.rival_name = select_rival_name(cppred);
	red_closing(cppred);

	auto &variables = cppred.get_variable_store();
	variables.delete_string("temp_player_name");
	variables.delete_string("temp_rival_name");
	return ret;
}

}
