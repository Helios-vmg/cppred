#pragma once

#include <map>
#include <string>

struct Variable{
	unsigned id;
	bool is_string;
};

class Variables{
	bool initialized = false;
	std::map<std::string, Variable> variables;

	void init();
public:
	Variable get(const std::string &name);
	const std::map<std::string, Variable> &get_map();
};