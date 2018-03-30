#include "generate_trainer_parties.h"
#include "../common/sha1.h"
#include "PokemonData.h"

static const char * const trainer_parties_file = "input/trainer_parties.csv";
static const char * const trainers_file = "input/trainers.csv";
extern const char * const pokemon_data_file;
extern const char * const evolutions_file;
extern const char * const pokemon_moves_file;
extern const char * const moves_file;
extern const char * const pokemon_types_file;
extern const char * const effects_file;

static const std::vector<std::string> input_files = {
	trainer_parties_file,
	trainers_file,
	pokemon_data_file,
	evolutions_file,
	pokemon_moves_file,
	moves_file,
	pokemon_types_file,
	effects_file,
};

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
	unsigned id;
	std::string name;
	std::string display_name;
	std::string battle_image;
	unsigned base_reward;
	std::vector<int> ai_move_choice_mods;
	std::string ai_class;
	unsigned ai_param;
	std::vector<TrainerParty> parties;

	TrainerClass() = default;
	TrainerClass(const std::vector<std::string> &columns){
		this->id = to_unsigned(columns[0]);
		this->name = columns[1];
		this->display_name = columns[2];
		this->battle_image = columns[3];
		this->base_reward = to_unsigned(columns[4]);
		this->ai_move_choice_mods = to_int_vector(columns[5]);
		this->ai_class = columns[6];
		this->ai_param = to_unsigned(columns[7]);
	}
	TrainerClass(const TrainerClass &) = default;

	void serialize(std::vector<byte_t> &dst){
		write_varint(dst, this->id);
		write_ascii_string(dst, this->name);
		write_ascii_string(dst, this->display_name);
		write_ascii_string(dst, this->battle_image);
		write_varint(dst, this->base_reward);
		write_varint(dst, this->ai_move_choice_mods.size());
		for (auto i : this->ai_move_choice_mods)
			write_varint(dst, i);
		write_ascii_string(dst, this->ai_class);
		write_varint(dst, this->ai_param);

		write_varint(dst, this->parties.size());
		for (auto &party : this->parties){
			write_varint(dst, party.members.size());
			for (auto &member : party.members){
				write_varint(dst, member.species);
				write_varint(dst, member.level);
			}
		}
	}
};

static void generate_trainer_parties_internal(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data){
	auto current_hash = hash_files(input_files, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating trainer parties.\n";
		return;
	}
	std::cout << "Generating trainer parties...\n";

	if (!pokemon_data)
		pokemon_data.reset(new PokemonData);
	
	std::map<std::string, TrainerClass> classes;

	{
		static const std::vector<std::string> order1 = {
			"id",
			"name",
			"display_name",
			"battle_image",
			"base_reward",
			"ai_move_choice_mods",
			"ai_class",
			"ai_param",
		};

		CsvParser csv(trainers_file);
		auto rows = csv.row_count();
		for (auto i = rows; i--;){
			TrainerClass tc(csv.get_ordered_row(i, order1));
			classes[tc.name] = tc;
		}
	}

	{
		static const std::vector<std::string> order2 = {
			"trainer_class_name",
			"party_index",
			"species",
			"level",
		};

		CsvParser csv(trainer_parties_file);
		auto rows = csv.row_count();
		for (size_t i = 0; i < rows; i++){
			auto row = csv.get_ordered_row(i, order2);
			auto class_name = row[0];
			auto index = to_unsigned(row[1]) - 1;
			auto species_name = row[2];
			auto level = to_int(row[3]);

			auto it = classes.find(class_name);
			auto &tc = it->second;

			if (tc.parties.size() <= index)
				tc.parties.resize(index + 1);
			tc.parties[index].members.push_back({(int)pokemon_data->get_species_id(species_name), level});
		}
	}

	std::ofstream header("output/trainers.h");
	std::ofstream source("output/trainers.inl");

	header <<
		generated_file_warning << "\n"
		"#pragma once\n"
		"\n";

	source <<
		generated_file_warning << "\n"
		"\n";

	std::vector<byte_t> trainer_parties_data;
	write_varint(trainer_parties_data, classes.size());
	for (auto &kv : classes)
		kv.second.serialize(trainer_parties_data);

	write_buffer_to_header_and_source(header, source, trainer_parties_data, "trainer_parties_data");

	header << "\nenum class TrainerId{\n";
	{
		std::map<unsigned, TrainerClass *> temp;
		for (auto &kv : classes)
			temp[kv.second.id] = &kv.second;
		for (auto &kv : temp)
			header << "    " << kv.second->name << " = " << kv.first << ",\n";
		header << "};\n\n";
	}

	known_hashes[hash_key] = current_hash;
}

void generate_trainer_parties(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data){
	try{
		generate_trainer_parties_internal(known_hashes, pokemon_data);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_trainer_parties(): " + e.what());
	}
}
