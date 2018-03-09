#include "utility.h"
#include "generate_graphics.h"
#include "generate_maps.h"
#include "generate_pokemon_data.h"
#include "generate_text.h"
#include "generate_moves.h"
#include "generate_items.h"
#include "generate_audio.h"
#include "generate_map_objects.h"
#include "generate_trainer_parties.h"
#include "PokemonData.h"
#include "TextStore.h"
#include "../common/csv_parser.h"
#include <iostream>
#include <stdexcept>
#include <map>
#include <memory>
#include <ctime>

const char * const hashes_path = "output/hashes.csv";
const char * const text_file = "input/text.txt";

known_hashes_t load_hashes(){
	known_hashes_t ret;

	std::unique_ptr<CsvParser> csv;
	try{
		csv = std::make_unique<CsvParser>(hashes_path);
	}catch (std::exception &){
		return ret;
	}

	auto rows = csv->row_count();
	for (size_t i = 0; i < rows; i++){
		auto key = csv->get_cell(i, 0);
		auto value = csv->get_cell(i, 1);
		ret[key] = value;
	}

	return ret;
}

void save_hashes(const known_hashes_t &hashes){
	std::ofstream file(hashes_path);

	file << "key,hash\n";
	for (auto &kv : hashes)
		file << kv.first << ',' << kv.second << std::endl;
}

int main(){
	try{

		auto t0 = clock();
		auto hashes = load_hashes();
		GraphicsStore gs;
		std::unique_ptr<PokemonData> pokemon_data;
		TextStore ts(text_file, pokemon_data);
		generate_graphics(hashes, gs);
		generate_maps(hashes, gs, ts);
		generate_pokemon_data(hashes, pokemon_data);
		generate_text(hashes, ts);
		generate_moves(hashes);
		generate_items(hashes);
		generate_audio(hashes);
		generate_map_objects(hashes, pokemon_data);
		generate_trainer_parties(hashes, pokemon_data);
		save_hashes(hashes);
		auto t1 = clock();
		std::cout << "Elapsed: " << (double)(t1 - t0) / CLOCKS_PER_SEC << " s.\n";
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}catch (...){
		std::cerr << "Unknown exception.\n";
		return -1;
	}
	return 0;
}
