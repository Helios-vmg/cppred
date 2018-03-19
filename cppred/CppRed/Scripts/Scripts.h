#pragma once

#include <string>

#define DECLARE_SCRIPT(name) void name(const script_parameters &parameters)

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

DECLARE_SCRIPT(PrintRedSNESText);
DECLARE_SCRIPT(RedsHouse1FText1);
DECLARE_SCRIPT(RedsHouse1FText2);
DECLARE_SCRIPT(PalletTownScript);
DECLARE_SCRIPT(BluesHouseScript);
DECLARE_SCRIPT(BluesHouseText1);
DECLARE_SCRIPT(OaksLabScript);
DECLARE_SCRIPT(OaksLabText1);
DECLARE_SCRIPT(OaksLabText2);
DECLARE_SCRIPT(OaksLabText3);
DECLARE_SCRIPT(OaksLabText4);
DECLARE_SCRIPT(OaksLabText5);

}
}
