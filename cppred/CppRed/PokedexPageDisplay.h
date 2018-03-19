#pragma once

#include "ScreenOwner.h"

enum class PokedexId;

namespace CppRed{

class PokedexPageDisplay : public ScreenOwner{
	PokedexId species;
public:
	PokedexPageDisplay(Game &, PokedexId species);
	std::unique_ptr<ScreenOwner> run() override;
	void pause() override{}
};

}
