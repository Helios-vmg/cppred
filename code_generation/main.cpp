#include "utility.h"
#include "code_generators.h"
#include "../common/csv_parser.h"
#include <iostream>
#include <stdexcept>
#include <map>
#include <memory>

const char * const hashes_path = "output/hashes.csv";

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
		auto hashes = load_hashes();
		auto bitmaps = generate_bitmaps(hashes);
		generate_rams(hashes, bitmaps);
		generate_graphics(hashes);
		generate_maps(hashes);
		generate_pokemon_data(hashes);
		generate_text(hashes);
		generate_charmap(hashes);
		generate_moves(hashes);
		generate_sound_data(hashes);
		save_hashes(hashes);
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}catch (...){
		std::cerr << "Unknown exception.\n";
		return -1;
	}
	return 0;
}
