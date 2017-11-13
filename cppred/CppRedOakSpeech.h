#pragma once
#include <string>

class Engine;
class CppRedGame;

namespace CppRedScripts{

struct NamesChosenDuringOakSpeech{
	std::string player_name;
	std::string rival_name;
};

NamesChosenDuringOakSpeech oak_speech(CppRedGame &cppred);

}
