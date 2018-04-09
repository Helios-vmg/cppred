#pragma once

#include "utility.h"

enum class PaletteRegion{
	Background = 0,
	Sprites0 = 1,
	Sprites1 = 2,
};

enum class SubPaletteRegion{
	All,
	Background,
	Window,
	Sprites,
};

enum class TileRegion{
	Background,
	Window,
};

struct RGB{
	byte_t r, g, b, a;
};

class Palette{
public:
	char data[4];

	Palette(){
		for (int i = 0; i < 4; i++)
			this->data[i] = -1;
	}
	Palette(char x0, char x1, char x2, char x3){
		this->data[0] = x0;
		this->data[1] = x1;
		this->data[2] = x2;
		this->data[3] = x3;
	}
	Palette(byte_t c){
		*this = c;
	}
	const Palette &operator=(byte_t c){
		for (int i = 0; i < 4; i++)
			this->data[i] = (c >> (i * 2)) & BITMAP(00000011);
		return *this;
	}
	bool operator!() const{
		return this->data[0] == -1;
	}
};

static const Palette zero_palette = { 0, 0, 0, 0 };
static const Palette null_palette = { -1, -1, -1, -1 };
static const Palette default_palette = { 0, 1, 2, 3 };
static const Palette default_world_sprite_palette = { 0, 0, 1, 3 };

template <unsigned N>
struct BasicTileData{
	static const size_t size = N * N;
	byte_t data[size];
};

class Tile{
public:
	std::uint16_t tile_no;
	bool flipped_x;
	bool flipped_y;
	Palette palette;

	explicit Tile(std::uint16_t tile_no = 0, bool flipped_x = false, bool flipped_y = false, Palette palette = null_palette):
		tile_no(tile_no),
		flipped_x(flipped_x),
		flipped_y(flipped_y),
		palette(palette){}
};

class SpriteTile : public Tile{
public:
	bool has_priority;

	SpriteTile(std::uint16_t tile_no = 0, bool flipped_x = false, bool flipped_y = false, bool has_priority = false, Palette palette = null_palette):
		Tile(tile_no, flipped_x, flipped_y, palette),
		has_priority(has_priority){}
};

struct Tilemap{
	static const int w = 32;
	static const int h = 32;
	static const int size = w * h;
	Tile tiles[size];
	void swap(Tilemap &other){
		for (int i = size; i--;)
			std::swap(this->tiles[i], other.tiles[i]);
	}
};
