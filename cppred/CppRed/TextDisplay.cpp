#include "TextDisplay.h"
#include "Game.h"

namespace CppRed{

TextDisplay::TextDisplay(Game &game, TextResourceId text_id):
		ScreenOwner(game),
		text_id(text_id){
}

std::unique_ptr<ScreenOwner> TextDisplay::run(){
	auto &renderer = this->game->get_engine().get_renderer();
	renderer.set_enable_window(true);
	auto dialog_state = Game::get_default_dialog_state();
	renderer.set_window_region_start((dialog_state.box_corner - Point(1, 1)) * Renderer::tile_size);
	renderer.set_window_region_size((dialog_state.box_size + Point(2, 2)) * Renderer::tile_size);
	this->game->run_dialog(this->text_id, TileRegion::Window, true);
	renderer.set_enable_window(false);

	return nullptr;
}

}
