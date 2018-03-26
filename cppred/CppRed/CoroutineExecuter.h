#pragma once

#include "ScreenOwner.h"
#ifndef HAVE_PCH
#include <functional>
#endif

namespace CppRed{

class CoroutineExecuter : public ScreenOwner{
	std::function<void()> f;
public:
	CoroutineExecuter(Game &, std::function<void()> &&);
	void pause() override{}
};

}
