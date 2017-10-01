#include "CppRedOakSpeech.h"
#include "CppRedEngine.h"

static void fade_in(CppRedEngine &cppred){
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

static void scroll_from_the_right(CppRedEngine &cppred){
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

static void scroll_portrait_to_the_right(CppRedEngine &cppred, std::vector<Point> &red_pic){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	const auto t = Renderer::tile_size;
	{
		auto t0 = engine.get_clock();
		double y;
		do{
			auto t1 = engine.get_clock();
			y = (t1 - t0) * -20;
			if (y < -6)
				y = -6;
			renderer.set_y_bg_offset(4 * t, (4 + 7) * t, { cast_round(y * t), 0 });
			engine.wait_exactly_one_frame();
		}while (y > -6);
	}
	renderer.set_y_bg_offset(4 * t, (4 + 7) * t, Point{0, 0});
	for (auto p : red_pic)
		renderer.get_tile(TileRegion::Background, p).tile_no = 0;
	red_pic = renderer.draw_image_to_tilemap({ 12, 4 }, RedPicFront);
}

const char * const default_names_a[] = {
	"RED",
	"ASH",
	"JACK",
};

const char * const default_names_b[] = {
	"BLUE",
	"GARY",
	"JOHN",
};

#if POKEMON_VERSION == RED
auto &default_names_player = default_names_a;
auto &default_names_rival = default_names_b;
#elif POKEMON_VERSION == BLUE
auto &default_names_player = default_names_b;
auto &default_names_rival = default_names_a;
#endif

static void select_player_name(CppRedEngine &cppred){
	std::vector<std::string> items;
	items.push_back("NEW NAME");
	for (auto s : default_names_player)
		items.push_back(s);
	auto selection = cppred.handle_standard_menu_with_title(TileRegion::Background, { 0, 0 }, items, "NAME", { 0, 10 }, true);

}

namespace CppRedScripts{

NamesChosenDuringOakSpeech oak_speech(CppRedEngine &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	NamesChosenDuringOakSpeech ret;

	cppred.play_sound(SoundId::Stop);
	cppred.play_sound(SoundId::Music_Routes2);
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
	cppred.play_cry(SpeciesId::Nidorina);
	cppred.run_dialog(TextResourceId::OakSpeechText2B);
	cppred.fade_out_to_white();
	cppred.clear_screen();
	auto red_pic = renderer.draw_image_to_tilemap({ 6, 4 }, RedPicFront);
	scroll_from_the_right(cppred);
	cppred.run_dialog(TextResourceId::IntroducePlayerText);
	scroll_portrait_to_the_right(cppred, red_pic);
	select_player_name(cppred);
	engine.wait(3600);

	return ret;
}

}
