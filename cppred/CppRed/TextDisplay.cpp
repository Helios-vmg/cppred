#include "TextDisplay.h"
#include "Game.h"

namespace CppRed{

TextDisplay::TextDisplay(Game &game, TextResourceId text_id):
		ScreenOwner(game),
		text_id(text_id){
}

std::unique_ptr<ScreenOwner> TextDisplay::run(){
	auto &renderer = this->game->get_engine().get_renderer();
	auto &tilemap = renderer.get_tilemap(TileRegion::Background);
	const auto w = Renderer::logical_screen_tile_width;
	const auto h = Renderer::logical_screen_tile_height;
	const auto w2 = w + 2;
	const auto h2 = h + 2;
	auto original_offset = renderer.get_bg_global_offset();
	Tile original_contents[w2 * h2];
	for (int y = 0; y < h2; y++)
		for (int x = 0; x < w2; x++)
			original_contents[x + y * w2] = tilemap.tiles[x + y * Tilemap::w];

	renderer.set_bg_global_offset({0, 0});
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			tilemap.tiles[x + y * Tilemap::w] = original_contents[(x + 2) + (y + 2) * w2];
	renderer.set_enable_window(true);
	auto dialog_state = Game::get_default_dialog_state();
	renderer.set_window_region_start((dialog_state.box_corner - Point(1, 1)) * Renderer::tile_size);
	renderer.set_window_region_size((dialog_state.box_size + Point(2, 2)) * Renderer::tile_size);
	this->game->run_dialog(this->text_id, TileRegion::Window, true);
	renderer.set_enable_window(false);

	renderer.set_bg_global_offset(original_offset);
	for (int y = 0; y < h2; y++)
		for (int x = 0; x < w2; x++)
			tilemap.tiles[x + y * Tilemap::w] = original_contents[x + y * w2];
	return nullptr;
}

}