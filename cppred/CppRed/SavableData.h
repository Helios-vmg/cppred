#pragma once
#include "MiscClasses.h"
#ifndef HAVE_PCH
#include <memory>
#include <string>
#endif

namespace CppRed{

class SavableData{
	SavableData() = default;
public:
	bool valid;
	std::string player_name;
	std::string rival_name;
	GameOptions options;

	static std::shared_ptr<SavableData> load(const std::string &path);
	void save(const std::string &path);
};

}
