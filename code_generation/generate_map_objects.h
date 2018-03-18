#pragma once
#include "code_generators.h"

class PokemonData;
class Variables;

void generate_map_objects(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &, Variables &);
