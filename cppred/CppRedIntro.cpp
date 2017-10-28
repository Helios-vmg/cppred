#include "CppRedIntro.h"
#include "CppRedCommonFunctions.h"
#include "CppRedEngine.h"
#include "Engine.h"
#include "Renderer.h"
#include "CppRedCommonFunctions.h"
#include "utility.h"
#include "pokemon_version.h"
#include "../CodeGeneration/output/audio.h"
#include <iostream>

static void display_copyright(Engine &engine){
	auto &renderer = engine.get_renderer();
	renderer.draw_image_to_tilemap({ 2, 7 }, CopyrightScreen);
	renderer.set_palette(PaletteRegion::Background, BITMAP(11100100));
	engine.wait(3);
}

template <unsigned N>
static void draw_black_bars(Engine &engine){
	static_assert(N * 2 < Renderer::logical_screen_height, "N * 2 must be less than tilemap_height!");

	auto &renderer = engine.get_renderer();
	renderer.fill_rectangle(TileRegion::Background, { 0, 0 }, { Tilemap::w, N }, Tile(3));
	renderer.fill_rectangle(TileRegion::Background, { 0, Renderer::logical_screen_tile_height - N }, { Tilemap::w, N }, Tile(3));
}

static const Palette logo_palette_cycle[] = {
	BITMAP(11111100),
	BITMAP(11111100),
	BITMAP(01010100),
	BITMAP(10101000),
};

static const int falling_stars_visible_threshold = Renderer::tile_size * 11 + 1;
static const Palette falling_star_off = BITMAP(00000100);
static const Palette falling_star_on = BITMAP(00100100);
static const int star_waves = 4;

class shooting_star_graphics{
public:
	std::vector<Point> logo_tiles;
	std::vector<Point> game_freak_tiles;
	std::shared_ptr<Sprite> star;
	std::vector<std::shared_ptr<Sprite>> falling_stars;

	shooting_star_graphics(Engine &engine){
		auto &renderer = engine.get_renderer();
		this->logo_tiles = renderer.draw_image_to_tilemap({ 9, 7 }, GameFreakIntroLogo);
		this->game_freak_tiles = renderer.draw_image_to_tilemap({ 5, 10 }, GameFreak2);
		renderer.mass_set_palettes(this->logo_tiles, logo_palette_cycle[0]);
		renderer.mass_set_palettes(this->game_freak_tiles, logo_palette_cycle[0]);
		//renderer.set_palette(PaletteRegion::Background, BITMAP(11111100));
		this->star = renderer.create_sprite(2, 2);
		for (int i = 0; i < 2; i++){
			auto &tile0 = star->get_tile(0, i);
			tile0.tile_no = HalfStar.first_tile + i;
			tile0.has_priority = true;
			auto &tile1 = star->get_tile(1, i);
			tile1.tile_no = HalfStar.first_tile + i;
			tile1.flipped_x = true;
			tile1.has_priority = true;
		}
		this->star->set_x(Renderer::logical_screen_width - 12);
		this->star->set_y(-12);
		this->star->set_visible(true);
		this->star->set_palette(BITMAP(10100100));

		static const int xs[] = {
			40, 56,  80, 112,
			48, 64,  88, 104,
			44, 68,  76,  92,
			52, 84, 100, 108,
		};
		static_assert(array_length(xs) >= star_waves * 4, "");
		for (int i = 0; i < star_waves; i++){
			for (int j = 0; j < 4; j++){
				this->falling_stars.push_back(renderer.create_sprite(1, 1));
				auto last = this->falling_stars.back();
				last->get_tile(0, 0).tile_no = FallingStar.first_tile;
				last->set_y(falling_stars_visible_threshold - 8 * i);
				last->set_x(xs[j + i * 4]);
				last->set_palette(falling_star_off);
			}
		}
	}
};

static bool animate_big_star(CppRedEngine &cppred, shooting_star_graphics &graphics){
	auto &engine = cppred.get_engine();
	const double pixels_per_second = 240;
	auto &star = *graphics.star;
	auto x0 = star.get_x();
	auto y0 = star.get_y();
	auto t0 = engine.get_clock();
	bool ret = false;
	while (true){
		if (cppred.check_for_user_interruption()){
			ret = true;
			break;
		}
		auto t1 = engine.get_clock();
		auto offset = cast_round((t1 - t0) * pixels_per_second);
		if (y0 + offset > Renderer::logical_screen_height)
			break;
		star.set_y(y0 + offset);
		star.set_x(x0 - offset);
		engine.get_renderer().require_redraw();
	}
	graphics.star.reset();
	return ret;
}

static bool cycle_logo_palettes(CppRedEngine &cppred, shooting_star_graphics &graphics){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	for (int i = 0; i < 3; i++){
		renderer.mass_set_palettes(graphics.logo_tiles, logo_palette_cycle[(i + 1) % array_length(logo_palette_cycle)]);
		renderer.mass_set_palettes(graphics.game_freak_tiles, logo_palette_cycle[(i + 2) % array_length(logo_palette_cycle)]);

		if (cppred.check_for_user_interruption(1.0 / 6.0))
			return true;
	}
	return false;
}

static bool animate_falling_stars(CppRedEngine &cppred, shooting_star_graphics &graphics){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	const double falling_rate = 20;
	auto t0 = engine.get_clock();
	while (true){
		auto t1 = engine.get_clock();
		if (t1 - t0 >= 2.4)
			break;
		for (int i = 0; i < star_waves; i++){
			auto y = falling_stars_visible_threshold - 8 * i + cast_round((t1 - t0) * falling_rate);
			for (int j = 0; j < 4; j++){
				auto &sprite = *graphics.falling_stars[j + i * 4];
				sprite.get_tile(0, 0).tile_no = FallingStar.first_tile;
				auto pseudo_frame = (int)((t1 - t0) * 60);
				sprite.set_y(y);
				sprite.set_visible(y >= falling_stars_visible_threshold);
				sprite.set_palette(pseudo_frame / 3 % 2 ? falling_star_on : falling_star_off);
			}
		}
		renderer.require_redraw();

		if (cppred.check_for_user_interruption())
			return true;
	}
	return false;
}

static bool shooting_star_scene(CppRedEngine &cppred){
	shooting_star_graphics graphics(cppred.get_engine());
	cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Shooting_Star);
	return animate_big_star(cppred, graphics) || cycle_logo_palettes(cppred, graphics) || animate_falling_stars(cppred, graphics);
}

template <unsigned N>
static void clear_middle_of_screen(Engine &engine){
	static_assert(N * 2 < Renderer::logical_screen_height, "N * 2 must be less than tilemap_height!");

	auto &renderer = engine.get_renderer();
	renderer.fill_rectangle(TileRegion::Background, { 0, N }, { Tilemap::w, Renderer::logical_screen_tile_height - N * 2 }, Tile(0));
}

template <int A, int B, int C>
double parabola_func(double x){
	const double a = (double)A / (double)C;
	const double b = (double)B / (double)C;
	return (a * x + b) * x;
}

#define nidorino_parabola1 parabola_func<1, -25, 52>
#define nidorino_parabola2 parabola_func<5, -125, 39>
#define nidorino_parabola3 parabola_func<1, -25, 13>
#define nidorino_parabola4 parabola_func<1, -85, 25>

template <double Parabola(double)>
void hop_sprite(CppRedEngine &cppred, Sprite &sprite, AudioResourceId sfx, Point &position, int sign, double x_multiplier){
	auto &engine = cppred.get_engine();
	cppred.get_audio_interface().play_sound(sfx);
	auto t0 = engine.get_clock();
	const double duration = 25.0;
	double scaled;
	do{
		auto t1 = engine.get_clock();
		scaled = (t1 - t0) * 60;
		if (scaled > duration)
			scaled = duration;
		Point delta = { sign * cast_round(scaled * x_multiplier), cast_round(Parabola(scaled)) };
		sprite.set_position(position + delta);
		engine.wait_exactly_one_frame();
	}while (scaled < duration);
	position = sprite.get_position();
}

template <bool MoveSprite>
bool move_gengar(CppRedEngine &cppred, Sprite &nidorino, Point &position, int length, int sign = 1){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	auto t0 = engine.get_clock();
	const double speed = 60;
	int step;
	auto initial_offset = renderer.get_bg_global_offset();
	do{
		auto t1 = engine.get_clock();
		step = cast_round((t1 - t0) * speed);
		if (step > length)
			step = length;
		Point offset = { sign * step, 0 };
		renderer.set_bg_global_offset(initial_offset + offset);
		if (MoveSprite)
			nidorino.set_position(position + offset);
		if (cppred.check_for_user_interruption())
			return true;
	}while (step < length);
	
	if (MoveSprite)
		position = nidorino.get_position();

	return false;
}

struct BattleSceneSprites{
	std::shared_ptr<Sprite> nidorino[3];
};

static BattleSceneSprites battle_scene(CppRedEngine &cppred){
	const Point gengar_position = { 13, 7 };
	const Point nidorino_initial_position = { -6, 72 };

	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	cppred.get_audio_interface().play_sound(AudioResourceId::Music_IntroBattle);
	clear_middle_of_screen<4>(engine);
	engine.wait_frames(3);
	renderer.set_default_palettes();
	renderer.draw_image_to_tilemap(gengar_position, FightIntroBackMon1);

	GraphicsAsset assets_red[] = {
		FightIntroFrontMon1_red,
		FightIntroFrontMon2_red,
		FightIntroFrontMon3_red,
	};
	GraphicsAsset assets_blue[] = {
		FightIntroFrontMon1_blue,
		FightIntroFrontMon2_blue,
		FightIntroFrontMon3_blue,
	};
	auto &assets = cppred.get_version() == PokemonVersion::Red ? assets_red : assets_blue;
	
	auto nidorino = renderer.create_sprite(assets[0]);
	auto nidorino2 = renderer.create_sprite(assets[1]);
	auto nidorino3 = renderer.create_sprite(assets[2]);
	BattleSceneSprites ret = { nidorino, nidorino2, nidorino3 };
	nidorino->set_position(nidorino_initial_position);
	nidorino->set_visible(true);
	
	auto nidorino_position = nidorino->get_position();

	//Animate "rotation" around pokemon ring.
	if (move_gengar<true>(cppred, *nidorino, nidorino_position, 80))
		return ret;

	static const AudioResourceId hophop[] = {
		AudioResourceId::SFX_Intro_Hip,
		AudioResourceId::SFX_Intro_Hop,
	};

	//Nidorino then hops once to the left, then to the right, then repeats once.
	for (int i = 0, sign = 1; i < 4; i++){
		hop_sprite<nidorino_parabola1>(cppred, *nidorino, hophop[i % 2], nidorino_position, sign, 8.0 / 25.0);
		sign = -sign;
		if (i % 2 && cppred.check_for_user_interruption(1.0 / 6.0))
			return ret;
	}

	//Gengar moves back and raises arm.
	cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Intro_Raise);
	renderer.draw_image_to_tilemap(gengar_position, FightIntroBackMon2);
	if (move_gengar<false>(cppred, *nidorino, nidorino_position, 8) || cppred.check_for_user_interruption(0.5))
		return ret;

	//Now moves forward and lowers arm.
	cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Intro_Crash);
	renderer.draw_image_to_tilemap(gengar_position, FightIntroBackMon3);
	if (move_gengar<false>(cppred, *nidorino, nidorino_position, 18, -1))
		return ret;

	//Nidorino dodges.
	nidorino->set_visible(false);
	nidorino2->set_visible(true);
	hop_sprite<nidorino_parabola2>(cppred, *nidorino2, AudioResourceId::SFX_Intro_Hip, nidorino_position, 1, 1);
	if (cppred.check_for_user_interruption(0.5))
		return ret;

	//Now moves back to original position and original pose.
	if (move_gengar<false>(cppred, *nidorino, nidorino_position, 10, 1))
		return ret;
	renderer.draw_image_to_tilemap(gengar_position, FightIntroBackMon1);
	if (cppred.check_for_user_interruption(1))
		return ret;

	nidorino->set_visible(true);
	nidorino2->set_visible(false);

	//Nidorino then hops once to the left, then to the right, then repeats once.
	for (int i = 0, sign = -1; i < 2; i++){
		hop_sprite<nidorino_parabola3>(cppred, *nidorino, hophop[i % 2], nidorino_position, sign, 16.0 / 25.0);
		sign = -sign;
		if (i % 2 && cppred.check_for_user_interruption(1.0 / 6.0))
			return ret;
	}

	//Duck Nidorino.
	nidorino->set_visible(false);
	nidorino2->set_visible(true);
	{
		auto t0 = engine.get_clock();
		const double duration = 20.0;
		double scaled;
		do{
			auto t1 = engine.get_clock();
			scaled = (t1 - t0) * 60;
			if (scaled > duration)
				scaled = duration;
			Point delta = { 0, cast_round(scaled * 1.0 / 5.0) };
			nidorino2->set_position(nidorino_position + delta);
			engine.wait_exactly_one_frame();
		} while (scaled < duration);
		nidorino_position = nidorino2->get_position();
	}

	if (cppred.check_for_user_interruption(0.5))
		return ret;

	//Lunge Nidorino.
	cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Intro_Lunge);
	nidorino2->set_visible(false);
	nidorino3->set_visible(true);
	{
		auto t0 = engine.get_clock();
		double scaled;
		auto first_y = nidorino_position.y;
		auto min_y = first_y - 25;
		do{
			auto t1 = engine.get_clock();
			scaled = (t1 - t0) * 60;
			auto temp = nidorino_parabola4(scaled);
			Point delta = { cast_round(temp), cast_round(temp * 0.5) };

			nidorino3->set_position(nidorino_position + delta);
			if (nidorino3->get_y() < min_y)
				nidorino3->set_y(min_y);
			engine.wait_exactly_one_frame();
		}while (nidorino3->get_y() > min_y);
		nidorino_position = nidorino3->get_position();
	}

	return ret;
}

namespace CppRedScripts{

void intro(CppRedEngine &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	engine.get_renderer().set_enable_bg(true);
	engine.get_renderer().set_enable_sprites(true);
	display_copyright(engine);
	
	clear_screen(cppred.get_engine());
	draw_black_bars<4>(engine);
	engine.wait_frames(64);

	if (!shooting_star_scene(cppred))
		engine.wait_frames(40);

	{
		//Warning: temp contains side effect in its destructor. Do not remove this!
		auto temp = battle_scene(cppred);

		cppred.fade_out_to_white();
	}
	renderer.clear_screen();
	engine.wait_exactly_one_frame();
}

}
