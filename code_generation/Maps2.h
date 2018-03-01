#pragma once
#include <string>
#include "Tilesets2.h"
#include "ReorderedBlockset.h"
#include "utility.h"

struct MapConnection{
	std::string destination;
	int local_position;
	int remote_position;
};

class Map2{
	std::string name;
	std::shared_ptr<Tileset2> tileset;
	unsigned width, height;
	std::string map_data_name;
	std::shared_ptr<std::vector<byte_t>> map_data;
	std::string script;
	std::string objects;
	std::string random_encounters;
	std::string fishing_encounters;
	std::string music;
	unsigned border_block;
	MapConnection map_connections[4];
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
	const std::string &get_script() const{
		return this->script;
	}
	const std::string &get_objects() const{
		return this->objects;
	}
	const std::string &get_random_encounters() const{
		return this->random_encounters;
	}
	const std::string &get_fishing_encounters() const{
		return this->fishing_encounters;
	}
	const std::string &get_music() const{
		return this->music;
	}
	unsigned get_border_block() const{
		return this->border_block;
	}
	void serialize(std::vector<byte_t> &);
	void set_map_connection(int direction, const MapConnection &);
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
	void load_map_connections(const char *map_connections_path);
};
