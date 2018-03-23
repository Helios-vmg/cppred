#pragma once

#include "ScreenOwner.h"
#include <functional>

namespace CppRed{

class CoroutineExecuter : public ScreenOwner{
	std::function<void()> f;
public:
	CoroutineExecuter(Game &, std::function<void()> &&);
	void pause() override{}
};

}
