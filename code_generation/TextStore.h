#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <memory>

enum class CommandType{
	None = 0,
	Text,
	Line,
	Next,
	Cont,
	Para,
	Page,
	Prompt,
	Done,
	Dex,
	Autocont,
	Mem,
	Num,
	Cry,
	End,
};

class PokemonData;
class Variables;

typedef std::uint8_t byte_t;

struct MapTextEntry{
	std::string text;
	std::string script;
};

class TextStore{
	std::string path;
	bool initialized = false;
	std::vector<byte_t> binary_data;
	std::map<std::string, int> sections;
	std::vector<std::pair<std::string, int>> text_by_id;
	std::unique_ptr<PokemonData> &pokemon_data;
	Variables &variables;

	void load_data();
public:
	TextStore(const char *path, std::unique_ptr<PokemonData> &pokemon_data, Variables &variables):
		path(path),
		pokemon_data(pokemon_data),
		variables(variables){}
	const std::vector<byte_t> &get_binary_data();
	const std::map<std::string, int> &get_sections();
	const std::vector<std::pair<std::string, int>> &get_text_by_id();
	int get_text_id_by_name(const std::string &name);
};
