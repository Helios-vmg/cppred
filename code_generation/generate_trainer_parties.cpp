#include "generate_trainer_parties.h"
#include "../common/sha1.h"

static const char * const input_file = "input/trainer_parties.csv";
static const char * const hash_key = "generate_trainer_parties";
static const char * const date_string = __DATE__ __TIME__;

struct TrainerPartyMember{
	std::string species;
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
};

static void generate_trainer_parties_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating trainer parties.\n";
		return;
	}
	std::cout << "Generating trainer parties...\n";

	static const std::vector<std::string> order = {
		"trainer_class_name",
		"party_index",
		"species",
		"level",
	};

	std::map<std::string, TrainerClass> classes;

	CsvParser csv(input_file);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, order);
		auto class_name = row[0];
		auto index = to_unsigned(row[1]) - 1;
		auto species = row[2];
		auto level = to_int(row[3]);

		auto it = classes.find(class_name);
		if (it == classes.end()){
			classes[class_name] = TrainerClass(class_name);
			it = classes.find(class_name);
		}
		auto &tc = it->second;

		if (tc.parties.size() <= index)
			tc.parties.resize(index + 1);
		tc.parties[index].members.push_back({ species, level });
	}

	{
		std::ofstream header("output/trainer_parties.h");

		header <<
			generated_file_warning << "\n"
			"#pragma once\n"
			"\n";

		header << "namespace Trainer{\n";

		for (auto &kv : classes)
			header << "extern const BaseTrainerParty * const " << kv.second.name << "[" << kv.second.parties.size() << "];\n";

		header << "}\n";
	}
	{
		std::ofstream source("output/trainer_parties.inl");
		source <<
			generated_file_warning << "\n"
			"\n";

		for (auto &kv : classes){
			int index = 0;
			for (auto &party : kv.second.parties){
				source << "const TrainerParty<" << party.members.size() << "> " << kv.second.name << index++ << "({";
				for (auto &member : party.members)
					source << "TrainerPartyMember{SpeciesId::" << member.species << ", " << member.level << "},";
				source << "});\n";
			}
		}

		source << "\n"
			"namespace Trainer{\n";

		for (auto &kv : classes){
			source << "extern const BaseTrainerParty * const " << kv.second.name << "[" << kv.second.parties.size() << "] = {\n";
			int index = 0;
			for (auto &party : kv.second.parties)
				source << "\t&" << kv.second.name << index++ << ",\n";
			source << "};\n";
		}

		source << "}\n";
	}

	known_hashes[hash_key] = current_hash;
}

void generate_trainer_parties(known_hashes_t &known_hashes){
	try{
		generate_trainer_parties_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_trainer_parties(): " + e.what());
	}
}
