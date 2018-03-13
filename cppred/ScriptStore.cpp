#include "ScriptStore.h"
#include "CppRed/Scripts/Scripts.h"
#include "utility.h"
#include <algorithm>
#include <iostream>

#define ADD_SCRIPT(name) this->scripts.emplace_back(#name, CppRed::Scripts::name)

ScriptStore::ScriptStore(){
	ADD_SCRIPT(PrintRedSNESText);
	ADD_SCRIPT(RedsHouse1FText1);
	ADD_SCRIPT(RedsHouse1FText2);
	//Add scripts.

	std::sort(this->scripts.begin(), this->scripts.end(), [](const auto &a, const auto &b){ return a.first < b.first; });
}

void ScriptStore::execute(const std::string &script_name, CppRed::Game &game, CppRed::Actor &caller, const std::string &parameter) const{
	auto it = find_first_true(this->scripts.begin(), this->scripts.end(), [script_name](const auto &x){ return script_name <= x.first; });
	if (it == this->scripts.end() || it->first != script_name){
		std::cout << "Script not found. " << script_name << "(" << parameter << ")" << std::endl;
		return;
	}
	it->second(game, caller, parameter);
}
