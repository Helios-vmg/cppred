#include "CppRedIntro.h"
#include "CppRedCommonFunctions.h"
#include "CppRedEngine.h"
#include "Engine.h"
#include "Renderer.h"
#include "CppRedCommonFunctions.h"
#include "utility.h"
#include <iostream>

static void display_copyright(Engine &engine){
	auto &renderer = engine.get_renderer();
	renderer.draw_image_to_tilemap(2, 7, CopyrightScreen);
	renderer.set_palette(PaletteRegion::Background, BITMAP(11100100));
	engine.wait(3);
}

template <unsigned N>
static void draw_black_bars(Engine &engine){
	static_assert(N * 2 < Renderer::logical_screen_height, "N * 2 must be less than tilemap_height!");

	auto &renderer = engine.get_renderer();
	renderer.fill_rectangle(TileRegion::Background, 0, 0, Tilemap::w, N, 3);
	renderer.fill_rectangle(TileRegion::Background, 0, Renderer::logical_screen_tile_height - N, Tilemap::w, N, 3);
}

static std::shared_ptr<Sprite> load_shooting_star_graphics(Engine &engine){
	auto &renderer = engine.get_renderer();
	renderer.draw_image_to_tilemap(9, 7, GameFreakIntroLogo);
	renderer.draw_image_to_tilemap(5, 10, GameFreak2);
	renderer.set_palette(PaletteRegion::Background, BITMAP(11111100));
	auto star = renderer.create_sprite(2, 2);
	for (int i = 0; i < 2; i++){
		auto &tile0 = star->get_tile(0, i);
		tile0.tile_no = HalfStar.first_tile + i;
		tile0.has_priority = true;
		auto &tile1 = star->get_tile(1, i);
		tile1.tile_no = HalfStar.first_tile + i;
		tile1.flipped_x = true;
		tile1.has_priority = true;
	}
	star->set_x(Renderer::logical_screen_width - 12);
	star->set_y(-12);
	star->set_visible(true);
	renderer.set_palette(PaletteRegion::Sprites0, BITMAP(10100100));
	return star;
}

static bool animate_big_star(CppRedEngine &cppred){
	auto star = load_shooting_star_graphics(cppred.get_engine());
	auto &engine = cppred.get_engine();
	const double pixels_per_second = 240;
	auto x0 = star->get_x();
	auto y0 = star->get_y();
	auto t0 = engine.get_clock();
	while (true){
		if (cppred.check_for_user_interruption())
			return true;
		auto t1 = engine.get_clock();
		auto offset = (int)((t1 - t0) * pixels_per_second + 0.5);
		if (y0 + offset > Renderer::logical_screen_height)
			break;
		star->set_y(y0 + offset);
		star->set_x(x0 - offset);
		engine.get_renderer().require_redraw();
	}
	return false;
}

static bool cycle_logo_palettes(CppRedEngine &cppred){
	static const byte_t palettes[] = {
		BITMAP(11110100),
		BITMAP(11011000),
		BITMAP(11101100),
	};
	for (auto p : palettes){
		cppred.get_engine().get_renderer().set_palette(PaletteRegion::Background, p);
		if (cppred.check_for_user_interruption(1.0 / 6.0))
			return true;
	}
	return false;
}

static bool shooting_star_scene(CppRedEngine &cppred){
	cppred.play_sound(SoundId::SFX_Shooting_Star);
	if (animate_big_star(cppred) || cycle_logo_palettes(cppred))
		return true;
	return false;
}

template <unsigned N>
static void clear_middle_of_screen(Engine &engine){
	static_assert(N * 2 < Renderer::logical_screen_height, "N * 2 must be less than tilemap_height!");

	auto &renderer = engine.get_renderer();
	renderer.fill_rectangle(TileRegion::Background, 0, N, Tilemap::w, Renderer::logical_screen_tile_height - N * 2, 3);
}

static bool battle_scene(CppRedEngine &cppred){
	auto &engine = cppred.get_engine();
	cppred.play_sound(SoundId::Music_IntroBattle);
	clear_middle_of_screen<4>(engine);
	engine.get_renderer().clear_sprites();
	engine.wait_frames(3);
	//TODO
	return false;
}

namespace CppRed{

void intro(CppRedEngine &engine){
	display_copyright(engine.get_engine());
	
	clear_screen(engine.get_engine());
	draw_black_bars<4>(engine.get_engine());
	engine.get_engine().wait_frames(64);

	if (!shooting_star_scene(engine))
		engine.get_engine().wait_frames(40);

	battle_scene(engine);

	engine.fade_out_to_white();
	engine.get_engine().get_renderer().clear_screen();
	engine.get_engine().wait_exactly_one_frame();
}

}
