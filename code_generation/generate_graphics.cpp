#include "generate_graphics.h"
#include "Graphics.h"
#include "../common/csv_parser.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

static const char * const input_file = graphics_csv_path;
static const char * const hash_key = "generate_graphics";
static const char * const date_string = __DATE__ __TIME__;

static void print(std::ostream &stream, const std::vector<byte_t> &v, unsigned base_indent = 0){
	base_indent++;
	unsigned line_length = 0;
	for (unsigned i = 0; i < base_indent; i++){
		stream << "    ";
		line_length += 4;
	}
	stream << std::hex;
	for (auto b : v){
		if (line_length >= 80){
			stream << "\n";
			line_length = 0;
			for (unsigned i = 0; i < base_indent; i++){
				stream << "    ";
				line_length += 4;
			}
		}
		stream << "0x" << std::setw(2) << std::setfill('0') << (unsigned)b << ", ";
		line_length += 6;
	}
	stream << "\n" << std::dec;
}

struct ExtendedTile{
	std::shared_ptr<Graphic> graphic;
	Tile *tile;
};

std::set<pixel> get_unique_colors(const std::vector<ExtendedTile> &tiles){
	std::set<pixel> ret;
	for (auto &t : tiles){
		auto temp = t.tile->get_unique_colors();
		for (auto &p : temp)
			ret.insert(p);
	}
	return ret;
}

std::vector<byte_t> bit_pack(const std::vector<ExtendedTile> &tiles){
	auto unique_colors = get_unique_colors(tiles);

	{
		auto e = unique_colors.end();
		auto f0 = unique_colors.find(color0);
		auto f1 = unique_colors.find(color1);
		auto f2 = unique_colors.find(color2);
		auto f3 = unique_colors.find(color3);
		if (unique_colors.size() != ((f0 != e) + (f1 != e) + (f2 != e) + (f3 != e)))
			throw std::runtime_error("The graphics assets must only use colors [000000, 555555, AAAAAA, FFFFFF].");
	}

	static const unsigned colors_per_byte = 4;
	
	std::vector<byte_t> ret(tiles.size() * (Tile::size * Tile::size / colors_per_byte));
	for (size_t i = 0; i < tiles.size(); i++){
		auto &tile = tiles[i].tile->pixels;
		for (size_t j = 0; j < array_length(tile); j++){
			auto shift = (j % colors_per_byte) * 2;
			auto bit_value = 3 - tile[j].r / 0x55;
			auto dst = (j + i * array_length(tile)) / colors_per_byte;
			ret[dst] |= bit_value << shift;
		}
	}
	return ret;
}

std::vector<byte_t> byte_pack(const std::vector<ExtendedTile> &tiles){
	auto unique_colors = get_unique_colors(tiles);

	{
		auto e = unique_colors.end();
		auto f0 = unique_colors.find(color0);
		auto f1 = unique_colors.find(color1);
		auto f2 = unique_colors.find(color2);
		auto f3 = unique_colors.find(color3);
		if (unique_colors.size() != ((f0 != e) + (f1 != e) + (f2 != e) + (f3 != e)))
			throw std::runtime_error("The graphics assets must only use colors [000000, 555555, AAAAAA, FFFFFF].");
	}

	static const unsigned colors_per_byte = 1;

	std::vector<byte_t> ret(tiles.size() * (Tile::size * Tile::size / colors_per_byte));
	for (size_t i = 0; i < tiles.size(); i++){
		auto &tile = tiles[i].tile->pixels;
		for (size_t j = 0; j < array_length(tile); j++){
			auto shift = (j % colors_per_byte) * 2;
			auto bit_value = 3 - tile[j].r / 0x55;
			auto dst = (j + i * array_length(tile)) / colors_per_byte;
			ret[dst] |= bit_value << shift;
		}
	}
	return ret;
}

static void generate_graphics_internal(known_hashes_t &known_hashes, GraphicsStore &gs){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating graphics.\n";
		return;
	}
	std::cout << "Generating graphics...\n";

	const std::string dst_name = "gfx";
	
	auto &graphics = gs.get();
	
	std::vector<ExtendedTile> final_tiles;
	{
		std::map<std::string, unsigned> unique_tile_mapping;
		for (auto &g : graphics){
			for (auto &t : g->tiles){
				auto hash = t.hash();
				auto it = unique_tile_mapping.find(hash);
				auto id = final_tiles.size();
				if (it == unique_tile_mapping.end()){
					unique_tile_mapping[hash] = id;
					final_tiles.push_back({ g, &t });
				}else{
					id = it->second;
				}
				g->corrected_tile_numbers.push_back(id);
			}
		}
	}

	auto bit_packed = bit_pack(final_tiles);

	{
		std::ofstream header("output/graphics_public.h");
		header <<
			generated_file_warning << "\n"
			"#pragma once\n"
			"\n";
		header << "extern const std::pair<const char *, const GraphicsAsset *> graphics_assets_map[" << graphics.size() << "];\n"
			"static const size_t graphics_assets_map_size = " << graphics.size() << ";\n";
		for (auto &g : graphics)
			header << "extern const GraphicsAsset " << g->name << ";\n";
	}
	size_t packed_image_data_size,
		tile_mapping_size;
	{
		std::ofstream source("output/graphics.inl");
		source <<
			generated_file_warning << "\n"
			"\n";

		source << "extern const std::pair<const char *, const GraphicsAsset *> graphics_assets_map[" << graphics.size() << "] = {\n";
		for (auto &g : graphics)
			source << "\t{ \"" << g->name << "\", &" << g->name << " },\n";
		source << "};\n\n";

		for (auto &g : graphics)
			source << "const GraphicsAsset " << g->name << " = { " << g->first_tile << ", " << g->w << ", " << g->h << " };\n";

		source << "extern const byte_t packed_image_data[] = ";
		write_buffer_to_stream(source, bit_packed);
		packed_image_data_size = bit_packed.size();
		source << std::dec << ";\n"
			"\n"
			"extern const std::uint16_t tile_mapping[] = ";
		{
			std::vector<unsigned> temp;
			for (auto &g : graphics)
				for (auto i : g->corrected_tile_numbers)
					temp.push_back(i);
			write_collection_to_stream(source, temp.begin(), temp.end());
			tile_mapping_size = temp.size();
		}
		source << ";\n";
	}

	{
		std::ofstream header("output/graphics_private.h");
		header <<
			generated_file_warning << "\n"
			"#pragma once\n"
			"\n"
			"extern const byte_t packed_image_data[" << packed_image_data_size << "];\n"
			"extern const std::uint16_t tile_mapping[" << tile_mapping_size << "];\n"
			"static const size_t packed_image_data_size = " << packed_image_data_size << ";\n"
			"static const size_t tile_mapping_size = " << tile_mapping_size << ";\n"
			;
	}


	known_hashes[hash_key] = current_hash;
}

void generate_graphics(known_hashes_t &known_hashes, GraphicsStore &gs){
	try{
		generate_graphics_internal(known_hashes, gs);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_graphics(): " + e.what());
	}
}
