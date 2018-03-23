#include "TextDisplay.h"
#include "Game.h"

namespace CppRed{

TextDisplay::TextDisplay(Game &game, TextResourceId text_id, bool wait_at_end, bool hide_window_at_end):
		ScreenOwner(game),
		text_id(text_id),
		wait(wait_at_end),
		hide(hide_window_at_end){
	this->done = false;
	this->coroutine.reset(new Coroutine("TextDisplay coroutine", game.get_coroutine().get_clock(), [this](Coroutine &){
		auto &renderer = this->game->get_engine().get_renderer();
		this->game->run_dialogue(this->text_id, this->wait, this->hide);
		this->done = true;
	}));
}

}
