#pragma once
#include "MiscClasses.h"
#include <memory>
#include <string>

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
