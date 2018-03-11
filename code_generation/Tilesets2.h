#pragma once
#include <string>
#include <vector>
#include "utility.h"
#include "Graphics.h"
#include "../common/TilesetType.h"
#include "ReorderedBlockset.h"

class Tileset2{
	static int next_id;
	int id = next_id++;
	std::string name;
	std::string blockset_name;
	std::shared_ptr<std::vector<byte_t>> blockset;
	std::string collision_name;
	std::shared_ptr<std::vector<byte_t>> collision;
	std::shared_ptr<Graphic> tiles;
	std::vector<int> counters;
	int grass = -1;
	TilesetType tileset_type;
	std::vector<std::pair<int, int>> impassability_pairs;
	std::vector<std::pair<int, int>> impassability_pairs_water;
	int warp_check;
	std::vector<int> warp_tiles;
public:
	Tileset2(const std::vector<std::string> &columns, const std::map<std::string, std::shared_ptr<std::vector<byte_t>>> &blockset, const data_map_t &collision, GraphicsStore &gs);
	DELETE_COPY_CONSTRUCTORS(Tileset2);
	const std::string &get_name() const{
		return this->name;
	}
	const std::vector<int> &get_counters() const{
		return this->counters;
	}
	int get_grass() const{
		return this->grass;
	}
	TilesetType get_type() const{
		return this->tileset_type;
	}
	const std::vector<byte_t> &get_blockset() const{
		return *this->blockset;
	}
	const std::string &get_blockset_name() const{
		return this->blockset_name;
	}
	const std::string &get_collision_name() const{
		return this->collision_name;
	}
	const std::vector<byte_t> &get_collision() const{
		return *this->collision;
	}
	const Graphic &get_tiles() const{
		return *this->tiles;
	}
	int get_id() const{
		return this->id;
	}
	int get_warp_check() const{
		return this->warp_check;
	}
	const std::vector<int> &get_warp_tiles() const{
		return this->warp_tiles;
	}
	void serialize(std::vector<byte_t> &);
};

class Tilesets2{
	std::vector<std::shared_ptr<Tileset2>> tilesets;
	std::map<std::string, std::shared_ptr<Tileset2>> map;
public:
	Tilesets2(const char *path, const std::map<std::string, std::shared_ptr<std::vector<byte_t>>> &blockset, const data_map_t &collision, GraphicsStore &gs);
	std::shared_ptr<Tileset2> get(const std::string &name) const;
	const std::vector<std::shared_ptr<Tileset2>> &get_tilesets() const{
		return this->tilesets;
	}
};
