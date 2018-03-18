#include "generate_trainer_parties.h"
#include "../common/sha1.h"
#include "PokemonData.h"

static const char * const events_file = "input/trainer_parties.csv";
static const char * const hash_key = "generate_trainer_parties";
static const char * const date_string = __DATE__ __TIME__;

struct TrainerPartyMember{
	int species;
	int level;
};

struct TrainerParty{
	std::vector<TrainerPartyMember> members;
};

struct TrainerClass{
	std::string name;
	std::vector<TrainerParty> parties;

	TrainerClass() = default;
	TrainerClass(const std::string &name): name(name){}
	TrainerClass(const TrainerClass &) = default;

	void serialize(std::vector<byte_t> &dst){
		write_ascii_string(dst, this->name);
		unsigned index = 0;
		write_varint(dst, this->parties.size());
		for (auto &party : this->parties){
			write_varint(dst, index++);
			write_varint(dst, party.members.size());
			for (auto &member : party.members){
				write_varint(dst, member.species);
				write_varint(dst, member.level);
			}
		}
	}
};

static void generate_trainer_parties_internal(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data){
	auto current_hash = hash_file(events_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating trainer parties.\n";
		return;
	}
	std::cout << "Generating trainer parties...\n";

	if (!pokemon_data)
		pokemon_data.reset(new PokemonData);

	static const std::vector<std::string> order = {
		"trainer_class_name",
		"party_index",
		"species",
		"level",
	};

	std::map<std::string, TrainerClass> classes;

	CsvParser csv(events_file);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, order);
		auto class_name = row[0];
		auto index = to_unsigned(row[1]) - 1;
		auto species_name = row[2];
		auto level = to_int(row[3]);

		auto it = classes.find(class_name);
		if (it == classes.end()){
			classes[class_name] = TrainerClass(class_name);
			it = classes.find(class_name);
		}
		auto &tc = it->second;

		if (tc.parties.size() <= index)
			tc.parties.resize(index + 1);
		tc.parties[index].members.push_back({ (int)pokemon_data->get_species_id(species_name), level });
	}

	std::ofstream header("output/trainer_parties.h");
	std::ofstream source("output/trainer_parties.inl");

	header <<
		generated_file_warning << "\n"
		"#pragma once\n"
		"\n";

	source <<
		generated_file_warning << "\n"
		"\n";

	std::vector<byte_t> trainer_parties_data;

	for (auto &kv : classes)
		kv.second.serialize(trainer_parties_data);

	write_buffer_to_header_and_source(header, source, trainer_parties_data, "trainer_parties_data");

	known_hashes[hash_key] = current_hash;
}

void generate_trainer_parties(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data){
	try{
		generate_trainer_parties_internal(known_hashes, pokemon_data);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_trainer_parties(): " + e.what());
	}
}
