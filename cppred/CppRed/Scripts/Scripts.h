#pragma once

#ifndef HAVE_PCH
#include <string>
#endif

namespace CppRed{
class Game;
class Actor;
namespace Scripts{

struct script_parameters{
	const char *script_name;
	const char *parameter;
	CppRed::Game *game;
	CppRed::Actor *caller;
};

}
}
