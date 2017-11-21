#pragma once
#include "code_generators.h"

class PokemonData;

void generate_trainer_parties(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data);
