#pragma once

#include <string>

#define DECLARE_SCRIPT(name) void name(CppRed::Game &game, CppRed::Actor &caller, const std::string &parameter)

namespace CppRed{
class Game;
class Actor;
namespace Scripts{

DECLARE_SCRIPT(PrintRedSNESText);
DECLARE_SCRIPT(RedsHouse1FText1);
DECLARE_SCRIPT(RedsHouse1FText2);

}
}
