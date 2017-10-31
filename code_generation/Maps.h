#pragma once
#include <string>
#include "Tilesets.h"
#include "utility.h"

class Map{
	std::string name;
	std::shared_ptr<Tileset> tileset;
	unsigned width, height;
	std::shared_ptr<std::vector<byte_t>> map_data;
	//scripts
	//objects
public:
	Map(const std::vector<std::string> &columns, const Tilesets &tilesets, const data_map_t &maps_data);
	DELETE_COPY_CONSTRUCTORS(Map);
	const std::string &get_name() const{
		return this->name;
	}
	void render_to_file(const char *dst);
};

class Maps{
	std::vector<std::shared_ptr<Map>> maps;
	std::map<std::string, std::shared_ptr<Map>> map;
public:
	Maps(const char *maps_path, const char *map_data_path, const Tilesets &tilesets);
	DELETE_COPY_CONSTRUCTORS(Maps);
	std::shared_ptr<Map> get(const std::string &name);
	const decltype(maps) &get_maps() const{
		return this->maps;
	}
};
