#pragma once
#include <string>
#include "Tilesets2.h"
#include "ReorderedBlockset.h"
#include "utility.h"

class Map2{
	std::string name;
	std::shared_ptr<Tileset2> tileset;
	unsigned width, height;
	std::string map_data_name;
	std::shared_ptr<std::vector<byte_t>> map_data;
	//scripts
	//objects
public:
	Map2(const std::vector<std::string> &columns, const Tilesets2 &tilesets, const data_map_t &maps_data);
	DELETE_COPY_CONSTRUCTORS(Map2);
	const std::string &get_name() const{
		return this->name;
	}
	const Tileset2 &get_tileset() const{
		return *this->tileset;
	}
	const std::string &get_map_data_name() const{
		return this->map_data_name;
	}
	const std::vector<byte_t> &get_map_data() const{
		return *this->map_data;
	}
	void render_to_file(const char *imagefile = nullptr);
	unsigned get_width() const{
		return this->width;
	}
	unsigned get_height() const{
		return this->height;
	}
};

class Maps2{
	std::vector<std::shared_ptr<Map2>> maps;
	std::map<std::string, std::shared_ptr<Map2>> map;
public:
	Maps2(const char *maps_path, const data_map_t &reordered_map_data, const Tilesets2 &tilesets);
	DELETE_COPY_CONSTRUCTORS(Maps2);
	std::shared_ptr<Map2> get(const std::string &name);
	const decltype(maps) &get_maps() const{
		return this->maps;
	}
};
