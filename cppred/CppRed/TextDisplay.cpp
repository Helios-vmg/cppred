#include "TextDisplay.h"
#include "Game.h"

namespace CppRed{

TextDisplay::TextDisplay(Game &game, TextResourceId text_id, bool hide_window_at_end):
		ScreenOwner(game),
		text_id(text_id),
		hide(hide_window_at_end){
}

std::unique_ptr<ScreenOwner> TextDisplay::run(){
	auto &renderer = this->game->get_engine().get_renderer();
	renderer.set_enable_window(true);
	this->game->run_dialog(this->text_id, TileRegion::Window, true);
	if (this->hide)
		renderer.set_enable_window(false);

	return nullptr;
}

}
