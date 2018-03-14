#include "ScriptStore.h"
#include "CppRed/Scripts/Scripts.h"
#include "utility.h"
#include <algorithm>
#include "Console.h"

#define ADD_SCRIPT(name) this->scripts.emplace_back(#name, CppRed::Scripts::name)

ScriptStore::ScriptStore(){
	ADD_SCRIPT(PrintRedSNESText);
	ADD_SCRIPT(RedsHouse1FText1);
	ADD_SCRIPT(RedsHouse1FText2);
	ADD_SCRIPT(PalletTownScript);
	ADD_SCRIPT(PalletTown_onload);
	//Add scripts.

	std::sort(this->scripts.begin(), this->scripts.end(), [](const auto &a, const auto &b){ return a.first < b.first; });
}

void ScriptStore::execute(const CppRed::Scripts::script_parameters &parameter) const{
	auto f = this->get_script(parameter.script_name);
	if (!f){
		Logger() << "Script not found. " << parameter.script_name << "(" << (parameter.parameter ? parameter.parameter : "null") << ")\n";
		return;
	}
	f(parameter);
}

ScriptStore::script_f ScriptStore::get_script(const char *name) const{
	auto it = find_first_true(this->scripts.begin(), this->scripts.end(), [name](const auto &x){ return name <= x.first; });
	if (it == this->scripts.end() || it->first != name)
		return nullptr;
	return it->second;
}
