#pragma once

#include <memory>

namespace CppRed{

class Game;

class ScreenOwner{
protected:
	Game *game;

	ScreenOwner(Game &game): game(&game){}
public:
	virtual ~ScreenOwner(){}
	virtual std::unique_ptr<ScreenOwner> run() = 0;
};

}
