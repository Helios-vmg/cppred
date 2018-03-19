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
	this->game->run_dialog_from_script(this->text_id);
	if (this->hide)
		this->game->reset_dialog_state();

	return nullptr;
}

}
