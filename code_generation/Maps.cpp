#include "Maps.h"
#include "utility.h"
#include "../common/csv_parser.h"

Maps::Maps(const char *maps_path, const char *map_data_path, const Tilesets &tilesets){
	static const std::vector<std::string> order = { "name", "tileset", "width", "height", "map_data", "script", "objects", "id" };
	const int id_offset = 7;
	
	auto maps_data = read_data_csv(map_data_path);

	CsvParser csv(maps_path);
	auto rows = csv.row_count();

	for (size_t i = 0; i < rows; i++){
		auto columns = csv.get_ordered_row(i, order);
		if (!columns[id_offset].size())
			continue;

		this->maps.emplace_back(new Map(columns, tilesets, maps_data));
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

Map::Map(const std::vector<std::string> &columns, const Tilesets &tilesets, const data_map_t &maps_data){
	this->name = columns[0];
	this->tileset = tilesets.get(columns[1]);
	this->width = to_unsigned(columns[2]);
	this->height = to_unsigned(columns[3]);
	this->map_data_name = columns[4];
	auto it = maps_data.find(this->map_data_name);
	if (it == maps_data.end())
		throw std::runtime_error("Error: Map \"" + this->name + "\" references unknown map data \"" + columns[4] + "\"");
	this->map_data = it->second;
}

std::shared_ptr<Map> Maps::get(const std::string &name){
	auto it = this->map.find(name);
	if (it == this->map.end())
		throw std::runtime_error("Error: Invalid map \"" + name + "\"");
	return it->second;
}

void Map::render_to_file(const char *imagefile, std::vector<byte_t> &tiles_dst){
	const auto block_size = 4;
	const auto block_pixel_size = block_size * Tile::size;
	auto w = this->width * block_size;
	auto h = this->height * block_size;
	std::vector<byte_t> final_tiles(w * h);
	auto &blockset = this->tileset->get_blockset();
	for (unsigned y = 0; y < this->height; y++){
		for (unsigned x = 0; x < this->width; x++){
			auto base_dst = &final_tiles[x * block_size + y * block_size * w];
			auto block_id = (*this->map_data)[x + y * this->width];
			auto base_block = &blockset[block_size * block_size * block_id];
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
				throw std::runtime_error("Map::render_to_file(): Inconsistent source data.");
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

	save_png(imagefile, &pixel_data[0], pixel_w, pixel_h);
	
	for (auto t : final_tiles)
		tiles_dst.push_back(t);
}

std::shared_ptr<std::vector<byte_t>> Map::reorder_map_data(const std::map<std::string, ReorderedBlockset> &rbs) const{
	auto it = rbs.find(this->tileset->get_blockset_name());
	assert(it != rbs.end());
	auto &rb = it->second;
	auto ret = std::make_shared<std::vector<byte_t>>();
	signed char last = 0;
	for (unsigned y = 0; y < this->height; y++){
		for (unsigned i = 0; i < 4; i += 2){
			for (unsigned x = 0; x < this->width; x++){
				auto tile = (*this->map_data)[x + y * this->width];
				auto it1 = rb.block_renames.find(std::make_pair(tile, i));
				auto it2 = rb.block_renames.find(std::make_pair(tile, i + 1));
				assert(it1 != rb.block_renames.end() && it2 != rb.block_renames.end());
				ret->push_back(it1->second);
				ret->push_back(it2->second);
			}
		}
	}
	return ret;
}
