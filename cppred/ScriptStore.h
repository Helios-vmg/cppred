#pragma once

#include <vector>
#include <utility>
#include <string>

namespace CppRed{
class Game;
class Actor;
namespace Scripts{
struct script_parameters;
}
}

class ScriptStore{
public:
	typedef void (*script_f)(const CppRed::Scripts::script_parameters &parameter);
private:
	std::vector<std::pair<std::string, script_f>> scripts;
public:
	ScriptStore();
	void execute(const CppRed::Scripts::script_parameters &parameter) const;
	script_f get_script(const char *name) const;
};
