#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <FreeImage.h>
#include <set>
#include <vector>

typedef std::uint8_t byte_t;

struct pixel{
	byte_t r;
	byte_t g;
	byte_t b;
	byte_t a;
	bool operator<(const pixel &other) const{
		return (std::uint32_t)*this < (std::uint32_t)other;
	}
	bool operator>(const pixel &other) const{
		return other < *this;
	}
	bool operator==(const pixel &other) const{
		return this->r == other.r && this->g == other.g && this->b == other.b && this->a == other.a;
	}
	bool operator!=(const pixel &other) const{
		return !(*this == other);
	}
	explicit operator std::uint32_t() const;
};

extern const pixel color3;
extern const pixel color2;
extern const pixel color1;
extern const pixel color0;

struct Tile{
	static const unsigned size = 8;
	pixel pixels[size * size];

	std::string hash() const;
	std::set<pixel> get_unique_colors() const;
};

class Image{
	typedef std::unique_ptr<FIBITMAP, void(*)(FIBITMAP *)> bitmap_ptr;
	static void internal_unload_bitmap(FIBITMAP *bitmap);
	static bitmap_ptr load_bitmap(const char *path);
	static bitmap_ptr convert_to_32bit_bitmap(FIBITMAP *bitmap);
	Image(){}
	Image(unsigned w, unsigned h): w(w), h(h), bitmap(w * h){}
public:
	unsigned w = 0;
	unsigned h = 0;
	std::vector<pixel> bitmap;
	static std::unique_ptr<Image> load_image(const char *path);
	std::set<pixel> get_unique_colors() const;
	std::unique_ptr<std::vector<byte_t>> reduce(unsigned &bits) const;
	std::unique_ptr<Image> pad_out_pic(unsigned target_size) const;
	std::unique_ptr<Image> double_size() const;
	std::vector<Tile> reorder_into_tiles() const;
	static bitmap_ptr to_ptr(FIBITMAP *bitmap);
};

template <typename T>
std::set<pixel> get_unique_colors(const T &s){
	std::set<pixel> ret;
	for (auto &p : s)
		ret.insert(p);
	return ret;
}

void save_png(const char *path, const pixel *data, int w, int h);
