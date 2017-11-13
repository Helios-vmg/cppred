#pragma once
#include <string>

class Engine;

namespace CppRed{
class Game;
namespace Scripts{

struct NamesChosenDuringOakSpeech{
	std::string player_name;
	std::string rival_name;
};

NamesChosenDuringOakSpeech oak_speech(Game &game);

}
}
