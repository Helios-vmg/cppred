#pragma once
#include <string>
#include <vector>
#include "Blocksets.h"
#include "Graphics.h"
#include "../common/TilesetType.h"

class Tileset{
	std::string name;
	std::shared_ptr<std::vector<byte_t>> blockset;
	std::shared_ptr<Graphic> tiles;
	//collision data
	std::vector<int> counters;
	int grass = -1;
	TilesetType tileset_type;
public:
	Tileset(const std::vector<std::string> &columns, const Blocksets &blockset, GraphicsStore &gs);
	Tileset(const Tileset &) = delete;
	Tileset(Tileset &&) = delete;
	void operator=(const Tileset &) = delete;
	void operator=(Tileset &&) = delete;
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
	const Graphic &get_tiles() const{
		return *this->tiles;
	}
};

class Tilesets{
	std::vector<std::shared_ptr<Tileset>> tilesets;
	std::map<std::string, std::shared_ptr<Tileset>> map;
public:
	Tilesets(const char *path, const Blocksets &blockset, GraphicsStore &gs);
	std::shared_ptr<Tileset> get(const std::string &name) const;
};