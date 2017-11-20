#include "generate_map_objects.h"
#include "../common/sha1.h"

static const char * const input_file = "input/map_objects.csv";
static const char * const hash_key = "generate_map_objects";
static const char * const date_string = __DATE__ __TIME__;

static const std::map<std::string, std::string> types_map = {
	{ "event_disp", "EventDisp", },
	{ "hidden", "HiddenObject", },
	{ "item", "ItemMapObject", },
	{ "npc", "NpcMapObject", },
	{ "pokemon", "PokemonMapObject", },
	{ "sign", "Sign", },
	{ "trainer", "TrainerMapObject", },
	{ "warp", "MapWarp", },
};

struct MapObject;

std::string event_disp_f(const MapObject &);
std::string hidden_f(const MapObject &);
std::string item_f(const MapObject &);
std::string npc_f(const MapObject &);
std::string pokemon_f(const MapObject &);
std::string sign_f(const MapObject &);
std::string trainer_f(const MapObject &);
std::string warp_f(const MapObject &);

static const std::map<std::string, std::string (*)(const MapObject &)> generators_map = {
	{ "event_disp", event_disp_f, },
	{ "hidden", hidden_f, },
	{ "item", item_f, },
	{ "npc", npc_f, },
	{ "pokemon", pokemon_f, },
	{ "sign", sign_f, },
	{ "trainer", trainer_f, },
	{ "warp", warp_f, },
};

struct MapObject{
	unsigned id;
	std::string name;
	std::string display_name;
	std::string type;
	unsigned x, y;
	std::string params[7];

	MapObject(const std::vector<std::string> &row){
		this->id = to_unsigned(row[0]);
		this->name = row[2];
		this->type = row[3];
		this->x = to_unsigned(row[4]);
		this->y = to_unsigned(row[5]);
		for (size_t i = 0; i < array_length(this->params); i++)
			this->params[i] = row[i + 6];

		if (!this->name.size()){
			this->name = this->type + '_' + this->hash();
			std::stringstream stream;
			stream << this->name << '(' << this->id << ')';
			this->display_name = stream.str();
		}else
			this->display_name = this->name;
	}
	std::string hash() const{
		std::stringstream stream;
		stream << this->id << "," << this->type << "," << this->x << "," << this->y;
		for (size_t i = 0; i < array_length(this->params); i++)
			stream << "," << this->params[i];
		auto s = stream.str();
		return SHA1::HashToString(s.c_str(), s.size());
	}
	std::string get_typename() const{
		auto it = types_map.find(this->type);
		if (it == types_map.end())
			throw std::runtime_error("Invalid map object type: " + this->type);
		return it->second;
	}
	std::string generate_declaration(){
		return "const " + this->get_typename() + " " + this->name + ";";
	}
	std::string generate_definition(){
		auto it = generators_map.find(this->type);
		if (it == generators_map.end())
			throw std::runtime_error("Invalid map object type: " + this->type);
		std::stringstream ret;
		ret << "const " << this->get_typename() << ' ' << this->name << "(\"" << this->display_name << "\", " << this->x << ", " << this->y << it->second(*this) << ");";
		return ret.str();
	}
};

std::string event_disp_f(const MapObject &mo){
	return "";
}

std::string hidden_f(const MapObject &mo){
	return ", \"" + mo.params[1] + "\", \"" + mo.params[0] + "\"";
}

std::string npc_f(const MapObject &mo){
	std::stringstream stream;
	stream << ", " << mo.params[0] << ", ObjectWithSprite::FacingDirection::";
	if (!mo.params[1].size())
		stream << "Undefined";
	else
		stream << mo.params[1];
	stream << ", " << mo.params[2] << ", " << mo.params[3] << ", " << mo.params[4];
	return stream.str();
}

std::string item_f(const MapObject &mo){
	return npc_f(mo) + ", ItemId::" + mo.params[5];
}

std::string pokemon_f(const MapObject &mo){
	return npc_f(mo) + ", SpeciesId::" + mo.params[5] + ", " + mo.params[6];
}
std::string sign_f(const MapObject &mo){
	return ", " + mo.params[0];
}

std::string trainer_f(const MapObject &mo){
	return npc_f(mo) + ", Trainer::" + mo.params[5] + " + " + mo.params[6];
}

std::string warp_f(const MapObject &mo){
	std::string ret = ", ";
	ret += mo.params[0];
	ret += ", WarpDestination(";
	if (mo.params[1].substr(0, 4) == "var:"){
		ret += '"';
		ret += mo.params[1].substr(4);
		ret += '"';
	}else{
		ret += "Map::";
		ret += mo.params[1];
	}
	ret += "), ";
	ret += mo.params[2];
	return ret;
}

static void generate_map_objects_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating map objects.\n";
		return;
	}
	std::cout << "Generating map objects...\n";

	static const std::vector<std::string> columns = {
		"id",
		"name",
		"object_name",
		"type",
		"x",
		"y",
		"param1",
		"param2",
		"param3",
		"param4",
		"param5",
		"param6",
		"param7",
	};

	CsvParser csv(input_file);
	std::map<std::string, std::vector<MapObject>> map_sets;
	size_t rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, columns);
		map_sets[row[1]].emplace_back(row);
	}

	{
		std::ofstream header("output/map_objects.h");
		header <<
			generated_file_warning << "\n"
			"#pragma once\n"
			"\n";

		for (auto &set : map_sets)
			header << "extern const MapObject * const " << set.first << "[" << set.second.size() << "];\n";
	}
	{
		std::ofstream source("output/map_objects.inl");
		source <<
			generated_file_warning << "\n"
			"\n";

		for (auto &set : map_sets){
			for (auto &o : set.second)
				source << o.generate_definition() << std::endl;
		}

		source << "\n";

		for (auto &set : map_sets){
			source << "extern const MapObject * const " << set.first << "[" << set.second.size() << "] = {\n";
			for (auto &o : set.second)
				source << "\t&" << o.name << ",\n";
			source << "};\n\n";
		}
	}

	known_hashes[hash_key] = current_hash;
}

void generate_map_objects(known_hashes_t &known_hashes){
	try{
		generate_map_objects_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_map_objects(): " + e.what());
	}
}
