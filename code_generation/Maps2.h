#pragma once
#include <string>
#include "Tilesets2.h"
#include "ReorderedBlockset.h"
#include "utility.h"
#include "TextStore.h"

class TextStore;

struct MapConnection{
	std::string destination;
	int local_position;
	int remote_position;
};

struct MapTextEntry2{
	int text;
	std::string script;

	MapTextEntry2(int t): text(t){}
	MapTextEntry2(const std:: string &s): text(-1), script(s){}
};

class Map2{
	std::string name;
	std::shared_ptr<Tileset2> tileset;
	unsigned width, height;
	std::string map_data_name;
	std::shared_ptr<std::vector<byte_t>> map_data;
	std::string on_frame;
	std::string on_load;
	std::string objects;
	std::string random_encounters;
	std::string fishing_encounters;
	unsigned music;
	unsigned border_block;
	MapConnection map_connections[4];
	std::vector<MapTextEntry2> map_text;
	int special_warp_check = 0;
	std::vector<int> special_warp_tiles;
public:
	Map2(const std::vector<std::string> &columns, const Tilesets2 &tilesets, const data_map_t &maps_data, const std::map<std::string, unsigned> &audio_map);
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
		return this->on_frame;
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
	unsigned get_border_block() const{
		return this->border_block;
	}
	void serialize(std::vector<byte_t> &);
	void set_map_connection(int direction, const MapConnection &);
	void set_map_text(const std::vector<MapTextEntry> &map_text, TextStore &);
};

class Maps2{
	std::vector<std::shared_ptr<Map2>> maps;
	std::map<std::string, std::shared_ptr<Map2>> map;
public:
	Maps2(const char *maps_path, const data_map_t &reordered_map_data, const Tilesets2 &tilesets, const std::map<std::string, unsigned> &audio_map);
	DELETE_COPY_CONSTRUCTORS(Maps2);
	std::shared_ptr<Map2> get(const std::string &name);
	const decltype(maps) &get_maps() const{
		return this->maps;
	}
	void load_map_connections(const char *map_connections_path);
	void load_map_text(const std::map<std::string, std::vector<MapTextEntry>> &, TextStore &);
};
