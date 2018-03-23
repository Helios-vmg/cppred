#include "ScreenOwner.h"

namespace CppRed{

ScreenOwner::RunResult ScreenOwner::run(){
	this->coroutine->get_clock().step();
	this->coroutine->resume();
	return this->done ? RunResult::Terminate : RunResult::Continue;
}

}
