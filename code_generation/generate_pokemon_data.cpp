#include "generate_pokemon_data.h"
#include "../common/csv_parser.h"
#include "utility.h"
#include "PokemonData.h"
#include <vector>
#include <iostream>
#include <map>
#include <stdexcept>

extern const char * const pokemon_data_file;
extern const char * const evolutions_file;
extern const char * const pokemon_moves_file;
static const char * const input_files[] = {
	pokemon_data_file,
	evolutions_file,
	pokemon_moves_file,
};
static const char * const hash_key = "generate_pokemon_data";
static const char * const date_string = __DATE__ __TIME__;

static void generate_pokemon_data_internal(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data){
	std::vector<std::string> input_files(::input_files, ::input_files + array_length(::input_files));
	auto current_hash = hash_files(input_files, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating Pokemon data.\n";
		return;
	}
	std::cout << "Generating Pokemon data...\n";

	if (!pokemon_data)
		pokemon_data.reset(new PokemonData);

	pokemon_data->generate_enums("output/pokemon_enums.h");
	pokemon_data->generate_static_data_declarations("output/pokemon_declarations.h");
	pokemon_data->generate_static_data_definitions("output/pokemon_definitions.inl", "pokemon_declarations.h");

	known_hashes[hash_key] = current_hash;
}

void generate_pokemon_data(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data){
	try{
		generate_pokemon_data_internal(known_hashes, pokemon_data);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_pokemon_data(): " + e.what());
	}
}
