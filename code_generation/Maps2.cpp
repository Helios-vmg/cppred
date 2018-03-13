#include "Maps2.h"
#include "utility.h"
#include "../common/csv_parser.h"
#include "TextStore.h"

Maps2::Maps2(const char *maps_path, const data_map_t &maps_data, const Tilesets2 &tilesets){
	static const std::vector<std::string> order = {
		"name",				  //  0
		"tileset",			  //  1
		"width",			  //  2
		"height",			  //  3
		"map_data",			  //  4
		"on_frame",			  //  5
		"objects",			  //  6
		"id",				  //  7
		"random_encounters",  //  8
		"fishing_encounters", //  9
		"music",			  // 10
		"border_block",		  // 11
		"special_warp_check", // 12
		"special_warp_tiles", // 13
		"on_load",			  // 14
	};
	const int id_offset = 7;

	CsvParser csv(maps_path);
	auto rows = csv.row_count();

	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
		if (!columns[id_offset].size())
			continue;

		this->maps.emplace_back(new Map2(columns, tilesets, maps_data));
		auto back = this->maps.back();
		this->map[back->get_name()] = back;
	}

	std::map<std::string, std::string> map_data_to_blockset;
	for (auto &map : this->maps){
		auto map_data = map->get_map_data_name();
		auto blockset = map->get_tileset().get_blockset_name();
		auto it = map_data_to_blockset.find(map_data);
		if (it == map_data_to_blockset.end()){
			map_data_to_blockset[map_data] = blockset;
			continue;
		}
		if (it->second != blockset)
			throw std::runtime_error("Error: Map data \"" + map_data + "\" fails validation.");
	}
}

Map2::Map2(const std::vector<std::string> &columns, const Tilesets2 &tilesets, const data_map_t &maps_data){
	this->name = columns[0];
	this->tileset = tilesets.get(columns[1]);
	this->width = to_unsigned(columns[2]) * 2;
	this->height = to_unsigned(columns[3]) * 2;
	this->map_data_name = columns[4];
	auto it = maps_data.find(this->map_data_name);
	if (it == maps_data.end())
		throw std::runtime_error("Error: Map \"" + this->name + "\" references unknown map data \"" + columns[4] + "\"");
	this->map_data = it->second;
	if (this->map_data->size() != this->width * this->height)
		throw std::runtime_error("Error: Map \"" + this->name + "\" has invalid size.");
	this->on_frame = columns[5];
	this->objects = columns[6];
	this->random_encounters = columns[8];
	this->fishing_encounters = columns[9];
	this->music = columns[10];
	this->border_block = to_unsigned(columns[11]);
	if (columns[12].size())
		this->special_warp_check = to_unsigned(columns[12]);
	this->special_warp_tiles = to_int_vector(columns[13], true);
	this->on_load = columns[14];
}

std::shared_ptr<Map2> Maps2::get(const std::string &name){
	auto it = this->map.find(name);
	if (it == this->map.end())
		throw std::runtime_error("Error: Invalid map \"" + name + "\"");
	return it->second;
}

void Map2::render_to_file(const char *imagefile){
	const auto block_size = 2;
	const auto block_pixel_size = block_size * Tile::size;
	auto w = this->width * block_size;
	auto h = this->height * block_size;
	std::vector<byte_t> final_tiles(w * h);
	auto &blockset = this->tileset->get_blockset();
	for (unsigned y = 0; y < this->height; y++){
		for (unsigned x = 0; x < this->width; x++){
			auto base_dst = &final_tiles[x * block_size + y * block_size * w];
			auto block_id = (*this->map_data)[x + y * this->width];
			if (block_id * Block::size >= blockset.size())
				throw std::runtime_error("Map2::render_to_file(): Data inconsistency detected in map \"" + this->name + "\".");
			auto base_block = &blockset[block_id * Block::size];
			for (unsigned y2 = 0; y2 < block_size; y2++)
				for (unsigned x2 = 0; x2 < block_size; x2++)
					base_dst[x2 + y2 * w] = *(base_block++);
		}
	}
	auto pixel_w = w * Tile::size;
	auto pixel_h = h * Tile::size;
	std::vector<pixel> pixel_data(pixel_w * pixel_h);
	auto &tiles = this->tileset->get_tiles().tiles;
	std::set<byte_t> collision;
	for (auto tile : this->tileset->get_collision())
		collision.insert(tile);
	for (unsigned y = 0; y < h; y++){
		for (unsigned x = 0; x < w; x++){
			auto src = final_tiles[x + y * w];
			if (src >= tiles.size())
				throw std::runtime_error("Map2::render_to_file(): Data inconsistency detected in map \"" + this->name + "\".");
			auto pixels = tiles[src].pixels;

			auto x0 = x % 2;
			auto y0 = y % 2;
			auto collision_tile = src;
			/*
			switch (x0 + y0 * 2){
				case 0:
					collision_tile = final_tiles[x + (y + 1) * w];
					break;
				case 1:
					collision_tile = final_tiles[(x - 1) + (y + 1) * w];
					break;
				case 2:
					break;
				case 3:
					collision_tile = final_tiles[(x - 1) + y * w];
					break;
			}
			*/
			bool passable = collision.find(collision_tile) != collision.end();

			auto dst = &pixel_data[x * Tile::size + y * Tile::size * pixel_w];
			for (unsigned y2 = 0; y2 < Tile::size; y2++){
				for (unsigned x2 = 0; x2 < Tile::size; x2++){
					auto c = *(pixels++);
					if (passable){
						c.g = 0;
						c.b = 0;
					}
					dst[x2 + y2 * pixel_w] = c;
				}
			}
		}
	}

	if (imagefile)
		save_png(imagefile, &pixel_data[0], pixel_w, pixel_h);
}

void Map2::serialize(std::vector<byte_t> &dst){
	write_ascii_string(dst, this->name);
	write_ascii_string(dst, this->tileset->get_name());
	write_varint(dst, this->width);
	write_varint(dst, this->height);
	write_ascii_string(dst, this->map_data_name);
	for (auto &mc : this->map_connections){
		write_ascii_string(dst, mc.destination);
		if (!mc.destination.size())
			continue;
		write_signed_varint(dst, mc.local_position);
		write_signed_varint(dst, mc.remote_position);
	}
	write_varint(dst, this->border_block);
	write_ascii_string(dst, this->objects);
	write_varint(dst, this->map_text.size());
	for (auto &text : this->map_text){
		write_signed_varint(dst, text.text);
		write_ascii_string(dst, text.script);
	}
	write_varint(dst, this->special_warp_check > 0 ? this->special_warp_check : this->tileset->get_warp_check());
	const std::vector<int> *warp_tiles = nullptr;
	if (this->special_warp_tiles.size())
		warp_tiles = &this->special_warp_tiles;
	else
		warp_tiles = &this->tileset->get_warp_tiles();
	if (warp_tiles->size() > 8)
		throw std::runtime_error("Error: map " + this->name + " has too many warp tiles. Max: 8");
	write_varint(dst, warp_tiles->size());
	for (auto tile : *warp_tiles)
		write_varint(dst, tile);
	write_ascii_string(dst, this->on_frame);
	write_ascii_string(dst, this->on_load);
	//TODO: Serialize other members.
}

int to_direction(const std::string &s){
	if (s.size() == 1){
		switch (s[0]){
			case 'N':
				return 0;
			case 'E':
				return 1;
			case 'S':
				return 2;
			case 'W':
				return 3;
		}
	}
	throw std::runtime_error("Invalid map connection direction: " + s);
}

void Maps2::load_map_connections(const char *map_connections_path){
	static const std::vector<std::string> order = {
		"map_name",
		"direction",
		"destination",
		"local_position",
		"remote_position",
	};
	CsvParser csv(map_connections_path);
	auto rows = csv.row_count();
	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
		auto source = columns[0];
		auto it = this->map.find(source);
		if (it == this->map.end())
			throw std::runtime_error("Invalid map connection. Source not found: " + source);
		auto destination = columns[2];
		auto it2 = this->map.find(destination);
		if (it2 == this->map.end())
			throw std::runtime_error("Invalid map connection. Destination not found: " + destination);
		auto direction = to_direction(columns[1]);
		auto local_position = to_int(columns[3]);
		auto remote_position = to_int(columns[4]);
		MapConnection mc;
		mc.destination = destination;
		mc.local_position = local_position;
		mc.remote_position = remote_position;
		it->second->set_map_connection(direction, mc);
	}
}

void Map2::set_map_connection(int direction, const MapConnection &mc){
	if (direction < 0 || (size_t)direction >= array_length(this->map_connections))
		throw std::runtime_error("Internal error.");
	this->map_connections[direction] = mc;
}

void Maps2::load_map_text(const std::map<std::string, std::vector<MapTextEntry>> &map_text, TextStore &text_store){
	for (auto &map : this->maps){
		auto it = map_text.find(map->get_name());
		if (it == map_text.end())
			continue;
		map->set_map_text(it->second, text_store);
	}
}

void Map2::set_map_text(const std::vector<MapTextEntry> &map_text, TextStore &text_store){
	this->map_text.clear();
	this->map_text.reserve(map_text.size());
	for (auto &text : map_text){
		if (text.text.size())
			this->map_text.emplace_back(text_store.get_text_id_by_name(text.text));
		else
			this->map_text.emplace_back(text.script);
	}
}
