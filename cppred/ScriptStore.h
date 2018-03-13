#pragma once

#include <vector>
#include <utility>
#include <string>

namespace CppRed{
class Game;
class Actor;
}

class ScriptStore{
public:
	typedef void (*script_f)(CppRed::Game &game, CppRed::Actor &caller, const std::string &parameter);
private:
	std::vector<std::pair<std::string, script_f>> scripts;
public:
	ScriptStore();
	void execute(const std::string &script_name, CppRed::Game &game, CppRed::Actor &caller, const std::string &parameter = std::string()) const;
};
