#pragma once

#include "ScreenOwner.h"

enum class PokedexId;

namespace CppRed{

class PokedexPageDisplay : public ScreenOwner{
	PokedexId species;

	void coroutine_entry_point();
public:
	PokedexPageDisplay(Game &, PokedexId species);
	void pause() override{}
};

}
