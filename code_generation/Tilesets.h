#pragma once
#include <string>
#include <vector>
#include "utility.h"
#include "Graphics.h"
#include "../common/TilesetType.h"

class Tileset{
	std::string name;
	std::string blockset_name;
	std::shared_ptr<std::vector<byte_t>> blockset;
	std::shared_ptr<std::vector<byte_t>> collision;
	std::shared_ptr<Graphic> tiles;
	std::vector<int> counters;
	int grass = -1;
	TilesetType tileset_type;
public:
	Tileset(const std::vector<std::string> &columns, const data_map_t &blockset, const data_map_t &collision, GraphicsStore &gs);
	DELETE_COPY_CONSTRUCTORS(Tileset);
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
	const std::vector<byte_t> &get_collision() const{
		return *this->collision;
	}
	const Graphic &get_tiles() const{
		return *this->tiles;
	}
};

class Tilesets{
	std::vector<std::shared_ptr<Tileset>> tilesets;
	std::map<std::string, std::shared_ptr<Tileset>> map;
public:
	Tilesets(const char *path, const data_map_t &blockset, const data_map_t &collision, GraphicsStore &gs);
	std::shared_ptr<Tileset> get(const std::string &name) const;
};