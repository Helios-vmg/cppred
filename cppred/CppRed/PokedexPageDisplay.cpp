#include "stdafx.h"
#include "PokedexPageDisplay.h"
#include "Game.h"
#include "../Engine.h"
#include "../AudioRenderer.h"

namespace CppRed{

PokedexPageDisplay::PokedexPageDisplay(Game &game, PokedexId species): ScreenOwner(game), species(species){
	this->done = false;
	this->coroutine.reset(new Coroutine("PokedexPageDisplay coroutine", game.get_coroutine().get_clock(), [this](Coroutine &){
		this->coroutine_entry_point();
	}));
}

template <size_t Digits, int max = std::numeric_limits<int>::max(), int min = 0>
std::array<char, Digits + 1> int_to_char_array(int n, char padding = '0'){
	auto min2 = std::max(min, 0);
	std::array<char, Digits + 1> ret;
	ret[Digits] = 0;
	if (n < min2)
		n = min2;
	else if (n > max)
		n = max;
	auto m = n;
	for (int i = Digits; i--;){
		if (!n){
			ret[i] = padding;
			continue;
		}
		
		ret[i] = n % 10 + '0';
		n /= 10;
	}
	if (!m)
		ret[Digits - 1] = '0';
	return ret;
}

void PokedexPageDisplay::coroutine_entry_point(){
	auto &engine = this->game->get_engine();
	auto &renderer = engine.get_renderer();
	auto &pokemon = *pokemon_by_pokedex_id[(int)this->species - 1];
	AutoRendererPusher pusher(renderer);
	renderer.clear_screen();
	renderer.clear_sprites();
	renderer.draw_image_to_tilemap({0, 0}, PokedexPageLayout);
	renderer.fill_rectangle(TileRegion::Background, {1, 1}, {7, 7}, Tile());
	this->game->put_string({9, 2}, TileRegion::Background, pokemon.display_name, 10);
	this->game->put_string({9, 4}, TileRegion::Background, pokemon.brief, 10);
	this->game->put_string({4, 8}, TileRegion::Background, int_to_char_array<3, 999>((int)pokemon.pokedex_id).data());
	renderer.fill_rectangle(TileRegion::Background, {1, 11}, {Renderer::logical_screen_tile_width - 2, 6}, Tile());
	auto &tilemap = renderer.get_tilemap(TileRegion::Background);
	auto copy = tilemap;
	this->game->put_string({13, 6}, TileRegion::Background, "?");
	this->game->put_string({15, 6}, TileRegion::Background, "??");
	this->game->put_string({13, 8}, TileRegion::Background, " ???");

	auto &coroutine = Coroutine::get_current_coroutine();
	auto &audio_interface = this->game->get_audio_interface();
	auto &mixer = engine.get_mixer();
	mixer.add_volume_divisor(2);
	coroutine.wait(0.5);

	this->game->draw_portrait(*pokemon.front, TileRegion::Background, {1, 1}, true);

	audio_interface.play_cry(pokemon.species_id);
	audio_interface.wait_for_sfx_to_end();

	tilemap = copy;
	this->game->draw_portrait(*pokemon.front, TileRegion::Background, {1, 1}, true);
	this->game->put_string({11, 6}, TileRegion::Background, int_to_char_array<3, 999>(pokemon.height_feet, ' ').data());
	this->game->put_string({15, 6}, TileRegion::Background, int_to_char_array<2, 99>(pokemon.height_inches, '0').data());
	this->game->put_string({11, 8}, TileRegion::Background, int_to_char_array<4, 9999>(pokemon.weight_tenths_of_pounds / 10, ' ').data());
	this->game->put_string({16, 8}, TileRegion::Background, int_to_char_array<1, 9>(pokemon.weight_tenths_of_pounds % 10, ' ').data());

	this->game->run_dex_entry(pokemon.pokedex_entry);
	
	mixer.remove_volume_divisor(2);
	this->done = true;
}

}
