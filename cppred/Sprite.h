#pragma once

#include "RendererStructs.h"

class Renderer;

class Sprite{
	Renderer *owner;
	std::uint64_t id;
	int x, y, w, h;
	bool visible = false;
	Palette palette = null_palette;
	PaletteRegion palette_region = PaletteRegion::Sprites0;
	std::vector<SpriteTile> tiles;
public:
	Sprite(Renderer &, int w, int h);
	~Sprite();
	Sprite(const Sprite &) = delete;
	Sprite(Sprite &&) = delete;
	void operator=(const Sprite &) = delete;
	void operator=(Sprite &&) = delete;
	SpriteTile &get_tile(int x, int y);
	iterator_pair<decltype(tiles)> iterate_tiles(){
		return { this->tiles.begin(), this->tiles.end() };
	}

	DEFINE_GETTER(id)
	DEFINE_GETTER_SETTER(x)
	DEFINE_GETTER_SETTER(y)
	DEFINE_GETTER(w)
	DEFINE_GETTER(h)
	DEFINE_GETTER_SETTER(visible)
	DEFINE_GETTER_SETTER(palette)
	DEFINE_GETTER_SETTER(palette_region)
	void set_position(const Point &p){
		this->x = p.x;
		this->y = p.y;
	}
	Point get_position(){
		return { this->x, this->y };
	}
};
