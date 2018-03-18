#include "generate_map_objects.h"
#include "../common/sha1.h"
#include "PokemonData.h"
#include "Variables.h"

static const char * const events_file = "input/map_objects.csv";
static const char * const names_input_file = "input/map_object_names.csv";
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

static std::string event_disp_f(const MapObject &);
static std::string hidden_f(const MapObject &);
static std::string item_f(const MapObject &);
static std::string npc_f(const MapObject &);
static std::string pokemon_f(const MapObject &);
static std::string sign_f(const MapObject &);
static std::string trainer_f(const MapObject &);
static std::string warp_f(const MapObject &);

static void event_disp_f(std::vector<byte_t> &dst, const MapObject &);
static void hidden_f    (std::vector<byte_t> &dst, const MapObject &);
static void item_f      (std::vector<byte_t> &dst, const MapObject &);
static void npc_f       (std::vector<byte_t> &dst, const MapObject &);
static void pokemon_f   (std::vector<byte_t> &dst, const MapObject &);
static void sign_f      (std::vector<byte_t> &dst, const MapObject &);
static void trainer_f   (std::vector<byte_t> &dst, const MapObject &);
static void warp_f      (std::vector<byte_t> &dst, const MapObject &);

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

static const std::map<std::string, void (*)(std::vector<byte_t> &, const MapObject &)> serializers_map = {
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
	PokemonData *pokemon_data;
	int legacy_sprite_id = -1;
	Variables &variables;

	MapObject(const std::vector<std::string> &row, PokemonData &pokemon_data, const std::map<unsigned, std::string> &name_map, Variables &variables):
			pokemon_data(&pokemon_data),
			variables(variables){
		this->id = to_unsigned(row[0]);
		{
			auto it = name_map.find(this->id);
			if (it != name_map.end())
				this->name = it->second;;
		}
		this->type = row[2];
		this->x = to_unsigned(row[3]);
		this->y = to_unsigned(row[4]);
		for (size_t i = 0; i < array_length(this->params); i++)
			this->params[i] = row[i + 5];
		if (row[12].size())
			this->legacy_sprite_id = (int)to_unsigned(row[12]);

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
	void serialize(std::vector<byte_t> &dst){
		auto it = serializers_map.find(this->type);
		if (it == serializers_map.end())
			throw std::runtime_error("Invalid map object type: " + this->type);

		write_ascii_string(dst, this->type);
		write_ascii_string(dst, this->display_name);
		write_varint(dst, this->x);
		write_varint(dst, this->y);
		it->second(dst, *this);
	}
};

//------------------------------------------------------------------------------

static std::string event_disp_f(const MapObject &mo){
	return "";
}

static std::string hidden_f(const MapObject &mo){
	return ", \"" + mo.params[1] + "\", \"" + mo.params[0] + "\"";
}

static std::string npc_f(const MapObject &mo){
	std::stringstream stream;
	stream << ", " << mo.params[0] << ", ObjectWithSprite::FacingDirection::";
	if (!mo.params[1].size())
		stream << "Undefined";
	else
		stream << mo.params[1];
	stream << ", " << mo.params[2] << ", " << mo.params[3] << ", " << mo.params[4];
	return stream.str();
}

static std::string item_f(const MapObject &mo){
	auto item = mo.params[5];
	if (!item.size())
		item = "None";
	return npc_f(mo) + ", ItemId::" + item;
}

static std::string pokemon_f(const MapObject &mo){
	return npc_f(mo) + ", SpeciesId::" + mo.params[5] + ", " + mo.params[6];
}

static std::string sign_f(const MapObject &mo){
	return ", " + mo.params[0];
}

static std::string trainer_f(const MapObject &mo){
	return npc_f(mo) + ", GET_TRAINER(Trainer::" + mo.params[5] + ", " + mo.params[6] + ")";
}

static std::string warp_f(const MapObject &mo){
	std::string ret = ", ";
	ret += mo.params[0];
	ret += ", WarpDestination(";
	if (mo.params[1].substr(0, 4) == "var:"){
		ret += '"';
		ret += mo.params[1].substr(4);
		ret += '"';
	}else{
		ret += "Maps::";
		ret += mo.params[1];
	}
	ret += "), ";
	ret += mo.params[2];
	return ret;
}

//------------------------------------------------------------------------------

static void event_disp_f(std::vector<byte_t> &dst, const MapObject &mo){}

static void hidden_f(std::vector<byte_t> &dst, const MapObject &mo){
	write_ascii_string(dst, mo.params[1]);
	write_ascii_string(dst, mo.params[0]);
}

static void npc_f(std::vector<byte_t> &dst, const MapObject &mo){
	assert(mo.legacy_sprite_id >= 0);
	write_varint(dst, mo.legacy_sprite_id);
	write_ascii_string(dst, mo.params[0]);
	std::string s = "Undefined";
	if (mo.params[1].size())
		s = mo.params[1];
	write_ascii_string(dst, s);
	write_varint(dst, mo.params[2] == "true");
	write_varint(dst, to_unsigned(mo.params[3]));
	write_varint(dst, to_unsigned(mo.params[4]));
}

static void item_f(std::vector<byte_t> &dst, const MapObject &mo){
	npc_f(dst, mo);
	auto item = mo.params[5];
	if (!item.size())
		item = "None";
	write_ascii_string(dst, item);
}

static void pokemon_f(std::vector<byte_t> &dst, const MapObject &mo){
	npc_f(dst, mo);
	write_varint(dst, mo.pokemon_data->get_species_id(mo.params[5]));
	write_varint(dst, to_unsigned(mo.params[6]));
}

static void sign_f(std::vector<byte_t> &dst, const MapObject &mo){
	write_varint(dst, to_unsigned(mo.params[0]));
}

static void trainer_f(std::vector<byte_t> &dst, const MapObject &mo){
	npc_f(dst, mo);
	write_ascii_string(dst, mo.params[5]);
	write_varint(dst, to_unsigned(mo.params[6]) - 1);
}

static void warp_f(std::vector<byte_t> &dst, const MapObject &mo){
	write_varint(dst, to_unsigned(mo.params[0]));
	if (mo.params[1].substr(0, 4) == "var:"){
		write_ascii_string(dst, mo.params[1]);
		try{
			auto name = mo.params[1].substr(4);
			auto variable = mo.variables.get(name);
			if (variable.is_string)
				throw std::runtime_error("Variable " + name + " is not a number.");
			write_varint(dst, variable.id);
		}catch (std::exception &e){
			std::stringstream stream;
			stream << "Error while processing warp with ID " << mo.id << ": " << e.what();
			throw std::runtime_error(stream.str());
		}
	}else
		write_ascii_string(dst, mo.params[1]);
	write_varint(dst, to_unsigned(mo.params[2]));
}

//------------------------------------------------------------------------------

static std::map<unsigned, std::string> load_name_map(){
	static const std::vector<std::string> order = {"id", "name"};
	CsvParser csv(names_input_file);
	auto rows = csv.row_count();
	std::map<unsigned, std::string> ret;
	for (auto i = rows; i--;){
		auto columns = csv.get_ordered_row(i, order);
		ret[to_unsigned(columns[0])] = columns[1];
	}
	return ret;
}

//------------------------------------------------------------------------------

static void generate_map_objects_internal(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data, Variables &variables){
	auto current_hash = hash_files({events_file, names_input_file}, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating map objects.\n";
		return;
	}
	std::cout << "Generating map objects...\n";

	if (!pokemon_data)
		pokemon_data.reset(new PokemonData);

	static const std::vector<std::string> order = {
		"id",				//  0
		"name",				//  1
		"type",				//  2
		"x",				//  3
		"y",				//  4
		"param1",			//  5
		"param2",			//  6
		"param3",			//  7
		"param4",			//  8
		"param5",			//  9
		"param6",			// 10
		"param7",			// 11
		"legacy_sprite_id",	// 12
	};

	CsvParser csv(events_file);
	std::map<std::string, std::vector<MapObject>> map_sets;
	size_t rows = csv.row_count();
	auto name_map = load_name_map();
	for (size_t i = 0; i < rows; i++){
		auto row = csv.get_ordered_row(i, order);
		map_sets[row[1]].emplace_back(row, *pokemon_data, name_map, variables);
	}

	std::ofstream header("output/map_objects.h");
	std::ofstream source("output/map_objects.inl");
	header <<
		generated_file_warning << "\n"
		"#pragma once\n"
		"\n";

	source <<
		generated_file_warning << "\n"
		"\n";

	std::vector<byte_t> map_objects_data;

	for (auto &set : map_sets){
		write_ascii_string(map_objects_data, set.first);
		write_varint(map_objects_data, set.second.size());
		for (auto &o : set.second)
			o.serialize(map_objects_data);
	}

	write_buffer_to_header_and_source(header, source, map_objects_data, "map_objects_data");

	known_hashes[hash_key] = current_hash;
}

void generate_map_objects(known_hashes_t &known_hashes, std::unique_ptr<PokemonData> &pokemon_data, Variables &variables){
	try{
		generate_map_objects_internal(known_hashes, pokemon_data, variables);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_map_objects(): " + e.what());
	}
}
