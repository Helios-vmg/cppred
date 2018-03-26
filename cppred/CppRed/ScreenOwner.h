#pragma once

#ifndef HAVE_PCH
#include <memory>
#endif

class Coroutine;

namespace CppRed{

class Game;

class ScreenOwner{
protected:
	Game *game;
	std::unique_ptr<Coroutine> coroutine;
	bool done;

	ScreenOwner(Game &game);
public:
	virtual ~ScreenOwner();
	enum class RunResult{
		Continue,
		Terminate,
	};
	virtual RunResult run();
	virtual void pause() = 0;
};

}
