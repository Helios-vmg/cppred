#include "Maps2.h"
#include "utility.h"
#include "../common/csv_parser.h"

Maps2::Maps2(const char *maps_path, const data_map_t &maps_data, const Tilesets2 &tilesets){
	static const std::vector<std::string> order = { "name", "tileset", "width", "height", "map_data", "script", "objects", };

	CsvParser csv(maps_path);
	auto rows = csv.row_count();

	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
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
