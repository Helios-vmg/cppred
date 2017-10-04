#include "code_generators.h"
#include "../common/csv_parser.h"
#define FREEIMAGE_LIB
#include <FreeImage.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <set>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "../common/sha1.h"

typedef std::uint8_t byte_t;

static const char * const input_file = "input/graphics.csv";
static const char * const hash_key = "generate_graphics";
static const char * const date_string = __DATE__ __TIME__;

struct pixel{
	byte_t r;
	byte_t g;
	byte_t b;
	byte_t a;
#define F(x) if (this->x < other.x) return true; if (this->x > other.x) return false
	bool operator<(const pixel &other) const{
		F(r);
		F(g);
		F(b);
		return this->a < other.a;
	}
#undef F
	bool operator>(const pixel &other) const{
		return other < *this;
	}
	bool operator==(const pixel &other) const{
		return this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a;
	}
	bool operator!=(const pixel &other) const{
		return !(*this == other);
	}
};

constexpr pixel colorn(byte_t n){
	return{ n, n, n, 0xFF };
}

const pixel color3 = colorn(0x00);
const pixel color2 = colorn(0x55);
const pixel color1 = colorn(0xAA);
const pixel color0 = colorn(0xFF);

struct Tile{
	static const unsigned size = 8;
	pixel pixels[size * size];

	std::string hash() const{
		SHA1 sha1;
		sha1.Input(this->pixels, sizeof(this->pixels));
		return sha1.ToString();
	}
};

class Image;

template <typename T>
std::set<pixel> get_unique_colors(const T &s){
	std::set<pixel> ret;
	for (auto &p : s)
		ret.insert(p);
	return ret;
}

std::set<pixel> get_unique_colors(const Tile &tile){
	return ::get_unique_colors(tile.pixels);
}

class Image{
	typedef std::unique_ptr<FIBITMAP, void(*)(FIBITMAP *)> bitmap_ptr;

	static void internal_unload_bitmap(FIBITMAP *bitmap){
		if (bitmap)
			FreeImage_Unload(bitmap);
	}

	static bitmap_ptr to_ptr(FIBITMAP *bitmap){
		return bitmap_ptr(bitmap, internal_unload_bitmap);
	}

	static bitmap_ptr load_bitmap(const char *path){
		auto type = FreeImage_GetFileType(path);
		if (type == FIF_UNKNOWN)
			return bitmap_ptr(nullptr, internal_unload_bitmap);
		return to_ptr(FreeImage_Load(type, path, type == FIF_JPEG ? JPEG_ACCURATE : 0));
	}

	static bitmap_ptr convert_to_32bit_bitmap(FIBITMAP *bitmap){
		auto converted = to_ptr(FreeImage_ConvertTo32Bits(bitmap));
		if (converted)
			return converted;
		converted = to_ptr(FreeImage_ConvertToType(bitmap, FIT_BITMAP, true));
		if (!converted)
			return bitmap_ptr(nullptr, internal_unload_bitmap);
		return convert_to_32bit_bitmap(converted.get());
	}
	Image(){}
	Image(unsigned w, unsigned h): w(w), h(h), bitmap(w * h){}
public:
	unsigned w = 0;
	unsigned h = 0;
	std::vector<pixel> bitmap;
	static std::unique_ptr<Image> load_image(const char *path){
		auto bitmap = load_bitmap(path);
		if (!bitmap)
			return nullptr;
		bitmap = convert_to_32bit_bitmap(bitmap.get());
		if (!bitmap)
			return nullptr;
		auto ret = std::unique_ptr<Image>(new Image);
		auto w = FreeImage_GetWidth(bitmap.get());
		auto h = FreeImage_GetHeight(bitmap.get());
		ret->w = w + (Tile::size - w % Tile::size) % Tile::size;
		ret->h = h + (Tile::size - h % Tile::size) % Tile::size;
		ret->bitmap.resize(ret->w * ret->h, color0);

		auto dst = &(ret->bitmap[0]);
		for (unsigned y = 0; y < h; y++){
			auto scanline = (pixel *)FreeImage_GetScanLine(bitmap.get(), h - 1 - y);
			for (unsigned x = 0; x < w; x++){
				dst->r = scanline->b;
				dst->g = scanline->g;
				dst->b = scanline->r;
				dst->a = scanline->a;
				dst++;
				scanline++;
			}
		}
		return ret;
	}
	std::set<pixel> get_unique_colors() const{
		return ::get_unique_colors(this->bitmap);
	}
	std::unique_ptr<std::vector<byte_t>> reduce(unsigned &bits) const{
		const auto unique = this->get_unique_colors();
		std::unique_ptr<std::vector<byte_t>> ret(new std::vector<byte_t>);
		bool force_four = false;
		if (unique.size() == 1){
			if (unique.find(color0) != unique.end()){
				ret->resize(this->bitmap.size(), 0);
				bits = 1;
				return ret;
			}
			if (unique.find(color3) != unique.end()){
				ret->resize(this->bitmap.size(), 0xFF);
				bits = 1;
				return ret;
			}
			if (unique.find(color1) == unique.end() && unique.find(color2) == unique.end())
				return nullptr;
			force_four = true;
		}
		if (!force_four && unique.size() == 2){
			auto count = (unique.find(color0) != unique.end()) + (unique.find(color3) != unique.end());
			if (count == unique.size()){
				unsigned bit = 0;
				for (auto &p : this->bitmap){
					unsigned value = p == color3;
					if (!bit)
						ret->push_back(value);
					else
						ret->back() |= value << bit;
					bit = (bit + 1) % 8;
				}
				bits = 1;
				return ret;
			}
		}
		if (force_four || unique.size() <= 4){
			auto count = (unique.find(color0) != unique.end()) + (unique.find(color3) != unique.end()) + (unique.find(color1) != unique.end()) + (unique.find(color2) != unique.end());
			if (!force_four && count != unique.size())
				return nullptr;
			unsigned bit = 0;
			for (auto &p : this->bitmap){
				unsigned value;
				if (p == color0)
					value = 0;
				else if (p == color1)
					value = 1;
				else if (p == color2)
					value = 2;
				else
					value = 3;
				if (!bit)
					ret->push_back(value);
				else
					ret->back() |= value << bit;
				bit = (bit + 2) % 8;
			}
			bits = 2;
			return ret;
		}
		return nullptr;
	}
	std::unique_ptr<Image> pad_out_pic(unsigned target_size) const{
		std::unique_ptr<Image> ret(new Image(target_size * Tile::size, target_size * Tile::size));
		std::fill(ret->bitmap.begin(), ret->bitmap.end(), color0);
		unsigned tiles_w = this->w / Tile::size;
		unsigned tiles_h = this->h / Tile::size;
		assert(tiles_w <= target_size && tiles_h <= target_size);
		unsigned y0 = target_size - tiles_h;
		unsigned x0 = (target_size + 1 - tiles_w) / 2;
		for (unsigned dst_y = y0; dst_y < target_size; dst_y++){
			unsigned src_y = dst_y - y0;
			assert(src_y < tiles_h);
			for (unsigned dst_x = x0; dst_x < x0 + tiles_w; dst_x++){
				auto src_x = dst_x - x0;
				auto src = this->bitmap.begin() + (src_x * Tile::size + src_y * tiles_w * (Tile::size * Tile::size));
				auto dst = ret->bitmap.begin() + (dst_x * Tile::size + dst_y * target_size * (Tile::size * Tile::size));
				for (unsigned i = 0; i < Tile::size; i++){
					auto src2 = src + this->w * i;
					std::copy(src2, src2 + Tile::size, dst + ret->w * i);
				}
			}
		}
		return ret;
	}
	std::unique_ptr<Image> double_size() const{
		std::unique_ptr<Image> ret(new Image(this->w * 2, this->h * 2));
		for (unsigned y = 0; y < this->h; y++){
			for (unsigned x = 0; x < this->w; x++){
				auto pix = this->bitmap[x + y * this->w];
				auto dst = ret->bitmap.begin() + (x * 2 + y * 2 * ret->w);
				dst[0] = pix;
				dst[1] = pix;
				dst[ret->w] = pix;
				dst[ret->w + 1] = pix;
			}
		}
		return ret;
	}
	std::vector<Tile> reorder_into_tiles() const{
		std::vector<Tile> ret;
		unsigned tiles_w = this->w / Tile::size;
		unsigned tiles_h = this->h / Tile::size;
		ret.reserve(tiles_w * tiles_h);
		for (unsigned y = 0; y < tiles_h; y++){
			for (unsigned x = 0; x < tiles_w; x++){
				ret.resize(ret.size() + 1);
				auto &dst = ret.back();
				auto src = this->bitmap.begin() + (x * Tile::size + y * tiles_w * (Tile::size * Tile::size));
				for (int i = 0; i < Tile::size; i++){
					auto src2 = src + this->w * i;
					std::copy(src2, src2 + Tile::size, dst.pixels + i * Tile::size);
				}
			}
		}
		return ret;
	}
};

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

enum class ImageType{
	Normal = 0,
	Charmap,
	Pic,
	BackPic,
};

struct Graphic{
	std::string name;
	std::string path;
	ImageType type;
	std::vector<Tile> tiles;
	std::vector<unsigned> corrected_tile_numbers;
	int first_tile = -1;
	int w = -1, h = -1;

	Graphic() = default;
	Graphic(const Graphic &other){
		*this = other;
	}
	Graphic(Graphic &&other){
		*this = std::move(other);
	}
	const Graphic &operator=(const Graphic &other){
		this->name = other.name;
		this->path = other.path;
		this->type = other.type;
		this->tiles = other.tiles;
		this->corrected_tile_numbers = other.corrected_tile_numbers;
		this->first_tile = other.first_tile;
		this->w = other.w;
		this->h = other.h;
		return *this;
	}
	const Graphic &operator=(Graphic &&other){
		this->name = std::move(other.name);
		this->path = std::move(other.path);
		this->type = other.type;
		this->tiles = std::move(other.tiles);
		this->corrected_tile_numbers = std::move(other.corrected_tile_numbers);
		this->first_tile = other.first_tile;
		this->w = other.w;
		this->h = other.h;
		return *this;
	}

	bool operator<(const Graphic &other) const{
		return this->type == ImageType::Charmap && other.type != ImageType::Charmap;
	}
};

struct ExtendedTile{
	Graphic *graphic;
	Tile *tile;
};

std::set<pixel> get_unique_colors(const std::vector<ExtendedTile> &tiles){
	std::set<pixel> ret;
	for (auto &t : tiles){
		auto temp = get_unique_colors(*t.tile);
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

static void generate_graphics_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating graphics.\n";
		return;
	}
	std::cout << "Generating graphics...\n";

	FreeImage_Initialise();
	std::unique_ptr<void, void(*)(void *)> fi_initializer((void *)1, [](void *){ FreeImage_DeInitialise(); });

	unsigned total_bytes = 0;
	static const std::vector<std::string> columns = {
		"name",
		"path",
		"type",
	};
	const std::string dst_name = "gfx";
	
	CsvParser csv(input_file);
	std::vector<Graphic> graphics;
	graphics.reserve(csv.row_count());
	int first_tile = 0;

	for (size_t i = 0; i < csv.row_count(); i++){
		auto row = csv.get_ordered_row(i, columns);
		Graphic gr;
		gr.name = row[0];
		if (gr.name.size() == 0)
			throw std::runtime_error("Invalid input.");
		if (gr.name[0] == '#')
			continue;
		gr.path = "input/" + row[1];
		gr.type = (ImageType)to_unsigned(row[2]);

		graphics.push_back(gr);
	}

	std::sort(graphics.begin(), graphics.end());

	for (auto &gr : graphics){
		auto image = Image::load_image(gr.path.c_str());
		if (!image)
			throw std::runtime_error("Error processing " + gr.path);

		switch (gr.type){
			case ImageType::Normal:
			case ImageType::Charmap:
				break;
			case ImageType::Pic:
				image = image->pad_out_pic(7);
				break;
			case ImageType::BackPic:
				image = image->double_size();
				break;
			default:
				throw std::runtime_error("Internal error.");
		}

		gr.first_tile = first_tile;
		gr.tiles = image->reorder_into_tiles();
		first_tile += gr.tiles.size();
		gr.w = image->w / Tile::size;
		gr.h = image->h / Tile::size;
	}
	
	std::vector<ExtendedTile> final_tiles;
	{
		std::map<std::string, unsigned> unique_tile_mapping;
		for (auto &g : graphics){
			for (auto &t : g.tiles){
				auto hash = t.hash();
				auto it = unique_tile_mapping.find(hash);
				auto id = final_tiles.size();
				if (it == unique_tile_mapping.end()){
					unique_tile_mapping[hash] = id;
					final_tiles.push_back({ &g, &t });
				}else{
					id = it->second;
				}
				g.corrected_tile_numbers.push_back(id);
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
		for (auto &g : graphics)
			header << "extern const GraphicsAsset " << g.name << ";\n";
	}
	size_t packed_image_data_size,
		tile_mapping_size;
	{
		std::ofstream source("output/graphics.inl");
		source <<
			generated_file_warning << "\n"
			"\n";

		for (auto &g : graphics)
			source << "const GraphicsAsset " << g.name << " = { " << g.first_tile << ", " << g.w << ", " << g.h << " };\n";

		source << "const byte_t packed_image_data[] = ";
		write_buffer_to_stream(source, bit_packed);
		packed_image_data_size = bit_packed.size();
		source << std::dec << ";\n"
			"\n"
			"static const std::uint16_t tile_mapping[] = ";
		{
			std::vector<unsigned> temp;
			for (auto &g : graphics)
				for (auto i : g.corrected_tile_numbers)
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
			"extern const byte_t packed_image_data[];\n"
			"extern const std::uint16_t tile_mapping[];\n"
			"static const size_t packed_image_data_size = " << packed_image_data_size << ";\n"
			"static const size_t tile_mapping_size = " << tile_mapping_size << ";\n"
			;
	}


	known_hashes[hash_key] = current_hash;
}

void generate_graphics(known_hashes_t &known_hashes){
	try{
		generate_graphics_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_graphics(): " + e.what());
	}
}
