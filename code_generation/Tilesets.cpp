#include "Tilesets.h"
#include "../common/csv_parser.h"
#include "utility.h"
#include <sstream>

Tilesets::Tilesets(const char *path, const data_map_t &blockset, const data_map_t &collision, GraphicsStore &gs){
	static const std::vector<std::string> order = { "name", "blockset", "tiles", "collision_data", "counters", "grass", "type", };

	CsvParser csv(path);
	auto rows = csv.row_count();

	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
		this->tilesets.emplace_back(new Tileset(columns, blockset, collision, gs));
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

Tileset::Tileset(const std::vector<std::string> &columns, const data_map_t &blockset, const data_map_t &collision, GraphicsStore &gs){
	this->name = columns[0];
	this->blockset_name = columns[1];
	this->blockset = get(blockset, this->blockset_name);
	this->tiles = gs.get(columns[2]);
	this->collision = get(collision, columns[3]);
	{
		std::stringstream stream(columns[4]);
		int i;
		while (stream >> i)
			this->counters.push_back(i);
	}
	this->grass = columns[5].size() ? (int)to_unsigned(columns[5]) : -1;
	this->tileset_type = to_TilesetType(columns[6]);
}

std::shared_ptr<Tileset> Tilesets::get(const std::string &name) const{
	auto it = this->map.find(name);
	if (it == this->map.end())
		throw std::runtime_error("Error: Invalid tileset \"" + name + "\"");
	return it->second;
}
