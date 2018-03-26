#include "stdafx.h"
#include "ScreenOwner.h"
#include "Coroutine.h"
#include "HighResolutionClock.h"

namespace CppRed{

ScreenOwner::ScreenOwner(Game &game): game(&game){}

ScreenOwner::RunResult ScreenOwner::run(){
	this->coroutine->get_clock().step();
	this->coroutine->resume();
	return this->done ? RunResult::Terminate : RunResult::Continue;
}

ScreenOwner::~ScreenOwner(){}

}
