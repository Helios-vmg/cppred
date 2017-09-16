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
		ret->w = w + (8 - w % 8) % 8;
		ret->h = h + (8 - h % 8) % 8;
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
		std::set<pixel> ret;
		for (auto &p : this->bitmap)
			ret.insert(p);
		return ret;
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

static void generate_graphics_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_file(input_file, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating graphics.\n";
		return;
	}
	std::cout << "Generating graphics...\n";

	unsigned total_bytes = 0;
	static const std::vector<std::string> columns = {
		"name",
		"path",
	};
	const std::string dst_name = "gfx";

	CsvParser csv(input_file);

	std::ofstream header("output/" + dst_name + ".h");
	std::ofstream source("output/" + dst_name + ".cpp");
	header << generated_file_warning << "\n";
	source << generated_file_warning << "\n";
	header << "#include \"../../cppred/StaticImage.h\"\n\n";
	source << "#include \"" + dst_name + ".h\"\n\n";
	for (size_t i = 0; i < csv.row_count(); i++){
		auto row = csv.get_ordered_row(i, columns);
		auto name = row[0];
		if (name.size() == 0)
			throw std::runtime_error("Invalid input.");
		if (name[0] == '#')
			continue;
		auto path = "input/" + row[1];
		auto image = Image::load_image(path.c_str());
		if (!image)
			throw std::runtime_error("Error processing " + path);
		unsigned bits;
		auto buffer = image->reduce(bits);
		if (!buffer)
			throw std::runtime_error("Error processing " + path);
		header << "extern const StaticImage<" << buffer->size() << "> " << name << ";\n";
		source << "const StaticImage<" << buffer->size() << "> " << name << " = {\n"
			<< "    " << image->w / 8 << ", " << image->h / 8 << ",\n"
			<< "    BitmapEncoding::";
		switch (bits){
			case 1:
				source << "Bit1";
				break;
			case 2:
				source << "Bit2";
				break;
			case 24:
				source << "Rgb";
				break;
		}
		source << ",\n"
			<< "    {\n";
		print(source, *buffer, 1);
		source << "    }\n"
			<< "};\n";
		total_bytes += buffer->size();
	}

	known_hashes[hash_key] = current_hash;
}

void generate_graphics(known_hashes_t &known_hashes){
	FreeImage_Initialise();
	try{
		generate_graphics_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_graphics(): " + e.what());
	}
	FreeImage_DeInitialise();
}
