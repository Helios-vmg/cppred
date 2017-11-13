#include "CppRedTitleScreen.h"
#include "CppRedGame.h"
#include "Engine.h"
#include "Renderer.h"
#include "CppRedData.h"
#include "../CodeGeneration/output/audio.h"
#include <cmath>

template <typename T, size_t N>
static void draw_image_from_offsets(Renderer &renderer, Point first_point, const GraphicsAsset &asset, const T (&offsets)[N]){
	for (size_t i = 0; i < N; i++){
		auto &tile = renderer.get_tile(TileRegion::Background, first_point);
		auto offset = offsets[i];
		tile.tile_no = offset >= 0 ? asset.first_tile + offsets[i] : ' ';
		tile.flipped_x = false;
		tile.flipped_y = false;
		first_point.x++;
	}
}

namespace EasingCurve{

double first_root_of_ith_parabola(int i, double base){
	const double s = pow(base, -0.5);
	return 1 + 2 * (pow(s, i + 1) - s) / (s - 1);
}

int reverse_first_root_of_ith_parabola(double x, double base){
	const double log_base = log(base);
	const double one_over_log_base = 1.0 / log(base);
	const double sqrt_base = sqrt(base);
	return 1 + (int)floor((2 * log((-2 * sqrt_base) / (-x + sqrt_base * (x - 1) - 1)) - log_base) * one_over_log_base);
}

double ith_parabola(double x, int i, double base){
	return -
		(x - first_root_of_ith_parabola(i - 1, base)) *
		(x - first_root_of_ith_parabola(i, base));
}

double f(double x, double base){
	return ith_parabola(x, reverse_first_root_of_ith_parabola(x, base), base);
}

double limit_of_curve(double base){
	const double s = sqrt(base);
	return (s + 1) / (s - 1);
}

}

static void bounce_logo(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	auto t0 = engine.get_clock();
	const double delta_speed = 0.5;
	const double base = 1 / (delta_speed * delta_speed);
	const double limit = EasingCurve::limit_of_curve(base);
	const double one_over_limit = 1.0 / limit;
	double x;
	bool played = false;
	do{
		auto t1 = engine.get_clock();
		x = (t1 - t0) * 3;
		if (x > limit)
			x = limit;
		//The easing function is designed to reach the first zero at exactly x = 1.
		//Therefore play the sound effect the first time the logo bounces.
		if (!played && x > 1){
			cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Intro_Crash);
			played = true;
		}
		renderer.set_y_bg_offset(0, 64, { 0, cast_round(64 * EasingCurve::f(x, base)) });
		engine.wait_exactly_one_frame();
	}while (x < limit);
}

static void scroll_version(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	auto t0 = engine.get_clock();
	const double duration = 26.0 / 60.0;
	double x;
	do{
		auto t1 = engine.get_clock();
		x = (t1 - t0) / duration;
		if (x > 1)
			x = 1;
		auto y = -(1 - x) * (Renderer::logical_screen_tile_width - 7) * Renderer::tile_size;
		renderer.set_y_bg_offset(64, 64 + 8, { cast_round(y) , 0 });
		engine.wait_exactly_one_frame();
	}while (x < 1);
}

static double pokemon_easing_curve(double x){
	const double acceleration = 68400.0 / 64.0;
	return acceleration * x * x;
}

static double pokeball_trajectory(double x){
	return (x - 1.0 / 6.0) * (-192 * 6.0) * x;
}

static const Point pokemon_location = {5, 10};

template <size_t N>
static void pick_new_pokemon(CppRedGame &cppred, const SpeciesId (&pokemons)[N], int &current_pokemon, Sprite &ball){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	
	int previous_pokemon = current_pokemon;
	current_pokemon = (current_pokemon + engine.get_prng()() % (N - 1) + 1) % N;

	//Scroll out.
	{
		auto t0 = engine.get_clock();
		const double duration = 0.3;
		double x = 0;
		do{
			auto t1 = engine.get_clock();
			x = t1 - t0;
			if (x > duration)
				x = duration;
			auto offset = cast_round(pokemon_easing_curve(x));
			renderer.set_y_bg_offset(80, 80 + 7 * Renderer::tile_size, {offset, 0});
			engine.wait_exactly_one_frame();
		}while (x < duration);
	}

	//Load new pokemon.
	renderer.draw_image_to_tilemap(pokemon_location, *pokemon_by_species_id[(int)pokemons[current_pokemon]]->front);

	if (pokemon_by_species_id[(int)pokemons[previous_pokemon]]->starter_index >= 0){
		auto y = ball.get_y();
		auto t0 = engine.get_clock();
		const double duration = 1.0/6.0;
		double x = 0;
		do{
			auto t1 = engine.get_clock();
			x = t1 - t0;
			if (x > duration)
				x = duration;
			auto position = cast_round(pokeball_trajectory(x));
			ball.set_y(y - position);
			engine.wait_exactly_one_frame();
		}while (x < duration);
	}

	//Scroll in.
	{
		auto t0 = engine.get_clock();
		const double duration = 0.3;
		double x = 0;
		do{
			auto t1 = engine.get_clock();
			x = t1 - t0;
			if (x > duration)
				x = duration;
			auto offset = cast_round(-pokemon_easing_curve(duration - x));
			renderer.set_y_bg_offset(80, 80 + 7 * Renderer::tile_size, {offset, 0});
			engine.wait_exactly_one_frame();
		}while (x < duration);
	}
}

namespace CppRedScripts{

TitleScreenResult title_screen(CppRedGame &cppred){
	static const char version_offsets_red[] = { 0, 1, -1, 5, 6, 7, 8, 9 };
	static const SpeciesId pokemons_red[] = {
		SpeciesId::Charmander,
		SpeciesId::Squirtle,
		SpeciesId::Bulbasaur,
		SpeciesId::Weedle,
		SpeciesId::NidoranMale,
		SpeciesId::Scyther,
		SpeciesId::Pikachu,
		SpeciesId::Clefairy,
		SpeciesId::Rhydon,
		SpeciesId::Abra,
		SpeciesId::Gastly,
		SpeciesId::Ditto,
		SpeciesId::Pidgeotto,
		SpeciesId::Onix,
		SpeciesId::Ponyta,
		SpeciesId::Magikarp,
	};
	static const char version_offsets_blue[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
	static const SpeciesId pokemons_blue[] = {
		SpeciesId::Squirtle,
		SpeciesId::Charmander,
		SpeciesId::Bulbasaur,
		SpeciesId::Mankey,
		SpeciesId::Hitmonlee,
		SpeciesId::Vulpix,
		SpeciesId::Chansey,
		SpeciesId::Aerodactyl,
		SpeciesId::Jolteon,
		SpeciesId::Snorlax,
		SpeciesId::Gloom,
		SpeciesId::Poliwag,
		SpeciesId::Doduo,
		SpeciesId::Porygon,
		SpeciesId::Gengar,
		SpeciesId::Raichu,
	};
	auto &pokemons = cppred.get_version() == PokemonVersion::Red ? pokemons_red : pokemons_blue;
	auto &version_offsets = cppred.get_version() == PokemonVersion::Red ? version_offsets_red : version_offsets_blue;

	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	cppred.palette_whiteout();
	cppred.clear_screen();

	renderer.set_default_palettes();
	renderer.set_enable_bg(true);
	renderer.set_enable_window(true);
	renderer.set_enable_sprites(true);

	renderer.draw_image_to_tilemap({2, 1}, PokemonLogoGraphics);
	const Point copyright_location = {(Renderer::logical_screen_tile_width - CopyrightTitleScreen.width) / 2, Renderer::logical_screen_tile_height - 1};
	const Point window_offset = {0, 10};
	int current_pokemon = 0;
	renderer.draw_image_to_tilemap(copyright_location - window_offset, CopyrightTitleScreen, TileRegion::Window);
	renderer.draw_image_to_tilemap(pokemon_location - window_offset, *pokemon_by_species_id[(int)pokemons[current_pokemon]]->front, TileRegion::Window);
	renderer.set_window_global_offset(window_offset * Renderer::tile_size);

	auto pc = renderer.create_sprite(PlayerCharacterTitleGraphics);
	//Hide pokeball in Red's hand.
	pc->get_tile(0, 2).tile_no = ' ';
	pc->set_position({82, 80});
	pc->set_visible(true);
	for (auto pair = pc->iterate_tiles(); pair.first != pair.second; ++pair.first)
		pair.first->has_priority = true;

	auto pokeball = renderer.create_sprite(1, 1);
	pokeball->get_tile(0, 0).tile_no = PlayerCharacterTitleGraphics.first_tile + PlayerCharacterTitleGraphics.width * 2;
	pokeball->set_position({ 82, 100 });
	pokeball->set_visible(true);
	for (auto pair = pokeball->iterate_tiles(); pair.first != pair.second; ++pair.first)
		pair.first->has_priority = true;

	renderer.set_y_bg_offset(0, 64, { 0, 64 });

	cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Intro_Crash);

	//Bounce logo.
	bounce_logo(cppred);

	engine.wait_frames(36);
	cppred.get_audio_interface().play_sound(AudioResourceId::SFX_Intro_Whoosh);

	draw_image_from_offsets(renderer, { 7, 8 }, RedBlueVersion, version_offsets);
	//Scroll version from the right.
	scroll_version(cppred);

	cppred.get_audio_interface().play_sound(AudioResourceId::Music_TitleScreen);

	renderer.set_enable_window(false);
	renderer.draw_image_to_tilemap(copyright_location, CopyrightTitleScreen);
	renderer.draw_image_to_tilemap(pokemon_location, *pokemon_by_species_id[(int)pokemons[current_pokemon]]->front);

	InputState user_input;
	while (!cppred.check_for_user_interruption(200.0/60.0, &user_input))
		pick_new_pokemon(cppred, pokemons, current_pokemon, *pokeball);

	cppred.get_audio_interface().play_cry(pokemons[current_pokemon]);
	cppred.wait_for_sound_to_finish();
	renderer.clear_screen();

	pc.reset();
	pokeball.reset();

	bool clear_save = check_flag(user_input.get_value(), InputState::mask_up | InputState::mask_select | InputState::mask_b);
	return clear_save ? TitleScreenResult::GoToClearSaveDialog : TitleScreenResult::GoToMainMenu;
}

}
