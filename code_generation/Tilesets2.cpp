#include "Tilesets2.h"
#include "TextStore.h"
#include "../common/csv_parser.h"
#include "utility.h"
#include <sstream>

int Tileset2::next_id = 1;

std::map<std::string, std::set<BookcaseTile>> load_bookcases(const char *path, TextStore &text_store){
	static const std::vector<std::string> order = {
		"tileset",
		"tile",
		"name",
		"is_script",
	};
	CsvParser csv(path);
	auto rows = csv.row_count();
	std::map<std::string, std::set<BookcaseTile>> ret;
	for (auto i = rows; i--;){
		auto columns = csv.get_ordered_row(i, order);
		BookcaseTile bt;
		bt.tile_no = to_unsigned(columns[1]);
		bt.is_script = to_bool(columns[3]);
		if (bt.is_script)
			bt.name = columns[2];
		else
			bt.text_id = text_store.get_text_id_by_name(columns[2]);
		ret[columns[0]].insert(bt);
	}
	return ret;
}

Tilesets2::Tilesets2(
		const char *path,
		const char *bookcases_path,
		const std::map<std::string, std::shared_ptr<std::vector<byte_t>>> &blockset,
		const data_map_t &collision,
		GraphicsStore &gs,
		TextStore &text_store){

	static const std::vector<std::string> order = {
		"name",
		"blockset",
		"tiles",
		"collision_data",
		"counters",
		"grass",
		"type",
		"impassability_pairs",
		"impassability_pairs_water",
		"warp_check",
		"warp_tiles",
	};

	CsvParser csv(path);
	auto rows = csv.row_count();

	auto bookcases = load_bookcases(bookcases_path, text_store);

	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
		this->tilesets.emplace_back(new Tileset2(columns, blockset, collision, gs, bookcases[columns[0]]));
		auto back = this->tilesets.back();
		this->map[back->get_name()] = back;
	}
}

static TilesetType to_TilesetType(const std::string &s){
	if (s == "Indoor")
		return TilesetType::Indoor;
	if (s == "Cave")
		return TilesetType::Cave;
	if (s == "Outdoor")
		return TilesetType::Outdoor;
	throw std::runtime_error("Error: Can't parse string \"" + s + "\" as a TilesetType.");
}

template <typename K, typename V>
const V &get(const std::map<K, V> &map, const K &key){
	auto it = map.find(key);
	if (it == map.end())
		throw std::runtime_error("Error: Invalid key \"" + key + "\"");
	return it->second;
}

std::vector<std::pair<int, int>> read_pairs(const std::string &s, const std::string &name, const char *column_name){
	auto pairs = to_int_vector(s);
	std::vector<std::pair<int, int>> ret;
	if (pairs.size()){
		if (pairs.size() % 2)
			throw std::runtime_error("Error: Tileset \"" + name + "\" has a malformed " + column_name + ". Must be a list of evenly-many numbers.");
		ret.reserve(pairs.size() / 2);
		for (size_t i = 0; i < pairs.size(); i += 2)
			ret.push_back(std::make_pair(pairs[i], pairs[i + 1]));
	}
	return ret;
}

Tileset2::Tileset2(
		const std::vector<std::string> &columns,
		const std::map<std::string, std::shared_ptr<std::vector<byte_t>>> &blocksets,
		const data_map_t &collision,
		GraphicsStore &gs,
		const std::set<BookcaseTile> &bookcase_tiles){

	this->name = columns[0];
	this->blockset_name = columns[1];
	this->blockset = get(blocksets, this->blockset_name);
	if (this->blockset->size() % 4)
		throw std::runtime_error("Error: blockset \"" + this->blockset_name + "\" has invalid size. Size must be a multiple of 4.");
	this->tiles = gs.get(columns[2]);
	this->collision_name = columns[3];
	this->collision = get(collision, this->collision_name);
	this->counters = to_int_vector(columns[4]);
	this->grass = columns[5].size() ? (int)to_unsigned(columns[5]) : -1;
	this->tileset_type = to_TilesetType(columns[6]);
	this->impassability_pairs = read_pairs(columns[7], this->name, "impassability_pairs");
	this->impassability_pairs_water = read_pairs(columns[8], this->name, "impassability_pairs_water");
	this->warp_check = to_unsigned(columns[9]);
	this->warp_tiles = to_int_vector(columns[10], true);
	this->bookcase_tiles = bookcase_tiles;
}

std::shared_ptr<Tileset2> Tilesets2::get(const std::string &name) const{
	auto it = this->map.find(name);
	if (it == this->map.end())
		throw std::runtime_error("Error: Invalid tileset \"" + name + "\"");
	return it->second;
}

void write_pair_list(std::vector<byte_t> &dst, const std::vector<std::pair<int, int>> &list, const std::string &name, const char *what){
	if (list.size() > 16)
		throw std::runtime_error("Error: Tileset " + name + " has too many " + what + ". Max: 16.");
	write_varint(dst, (std::uint32_t)list.size());
	for (auto &p : list){
		write_varint(dst, (std::uint32_t)p.first);
		write_varint(dst, (std::uint32_t)p.second);
	}
}

void Tileset2::serialize(std::vector<byte_t> &dst){
	write_varint(dst, this->id);
	write_ascii_string(dst, this->name);
	write_ascii_string(dst, this->blockset_name);
	write_ascii_string(dst, this->tiles->name);
	write_ascii_string(dst, this->collision_name);
	if (this->counters.size() > 16)
		throw std::runtime_error("Error: Tileset " + this->name + " has too many counter tiles. Max: 16.");
	write_varint(dst, (std::uint32_t)this->counters.size());
	for (auto c : this->counters)
		write_varint(dst, (std::uint32_t)c);
	std::uint32_t grass_tile = std::numeric_limits<std::uint32_t>::max();
	if (this->grass >= 0)
		grass_tile = this->grass;
	write_varint(dst, grass_tile);
	const char *type;
	switch (this->tileset_type){
		case TilesetType::Indoor: 
			type = "indoor";
			break;
		case TilesetType::Cave:
			type = "cave";
			break;
		case TilesetType::Outdoor:
			type = "outdoor";
			break;
		default:
			throw std::exception();
	}
	write_ascii_string(dst, type);
	write_pair_list(dst, this->impassability_pairs, this->name, "impassability pairs");
	write_pair_list(dst, this->impassability_pairs_water, this->name, "water impassability pairs");
	if (this->bookcase_tiles.size() > 8)
		throw std::runtime_error("Error: Tileset " + this->name + " has too many bookcases. Max: 8.");
	write_varint(dst, (std::uint32_t)this->bookcase_tiles.size());
	for (auto &bt : this->bookcase_tiles){
		write_varint(dst, bt.tile_no);
		write_varint(dst, bt.is_script);
		if (bt.is_script)
			write_ascii_string(dst, bt.name);
		else
			write_varint(dst, bt.text_id);
	}
}
