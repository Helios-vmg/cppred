#include "CoroutineExecuter.h"
#include "Game.h"

namespace CppRed{

CoroutineExecuter::CoroutineExecuter(Game &game, std::function<void()> &&f):
		ScreenOwner(game),
		f(std::move(f)){
	this->done = false;
	this->coroutine.reset(new Coroutine("CoroutineExecuter coroutine", game.get_coroutine().get_clock(), [this](Coroutine &){
		this->f();
		this->done = true;
	}));
}

}
