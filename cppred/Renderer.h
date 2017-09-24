#pragma once
#include "graphics_asset.h"
#include "utility.h"
#include <memory>
#include <vector>
#include <map>

struct SDL_Window;
class Engine;

enum class PaletteRegion{
	Background,
	Sprites0,
	Sprites1,
};

enum class TileRegion{
	Background,
	Window,
};

struct Tile{
	std::uint16_t tile_no;
	bool flipped_x;
	bool flipped_y;
};

struct SpriteTile : public Tile{
	bool has_priority;
};

struct Tilemap{
	static const int w = 32;
	static const int h = 32;
	static const int size = w * h;
	Tile tiles[size];
};

struct Point{
	int x, y;

	Point operator+(const Point &p) const{
		return { this->x + p.x, this->y + p.y };
	}
	Point operator-(const Point &p) const{
		return{ this->x - p.x, this->y - p.y };
	}
};

class SpriteOwner{
public:
	virtual void release_sprite(std::uint64_t) = 0;
	virtual std::uint64_t get_id() = 0;
};

class Sprite{
	SpriteOwner *owner;
	std::uint64_t id;
	int x, y, w, h;
	bool visible = false;
	PaletteRegion palette = PaletteRegion::Sprites0;
	std::vector<SpriteTile> tiles;
public:
	Sprite(SpriteOwner &, int w, int h);
	~Sprite();
	Sprite(const Sprite &) = delete;
	Sprite(Sprite &&) = delete;
	void operator=(const Sprite &) = delete;
	void operator=(Sprite &&) = delete;
	SpriteTile &get_tile(int x, int y);

	DEFINE_GETTER(id)
	DEFINE_GETTER_SETTER(x)
	DEFINE_GETTER_SETTER(y)
	DEFINE_GETTER(w)
	DEFINE_GETTER(h)
	DEFINE_GETTER_SETTER(visible)
	DEFINE_GETTER_SETTER(palette)
};

class Renderer{
	class Pimpl;
	std::unique_ptr<void, void(*)(void *)> pimpl;
	Pimpl &get_pimpl();
public:
	Renderer(Engine &, SDL_Window *);
	~Renderer();
	void set_palette(PaletteRegion region, byte_t value);
	void set_default_palettes();
	Tile &get_tile(TileRegion, int x, int y);
	Tilemap &get_tilemap(TileRegion);
	void render();
	void draw_image_to_tilemap(int x, int y, const GraphicsAsset &);
	void clear_screen();
	void set_enable_bg(bool value);
	void set_enable_window(bool value);
	void fill_rectangle(TileRegion region, int x, int y, int w, int h, int tile);
	void clear_sprites();
	std::shared_ptr<Sprite> create_sprite(int tiles_w, int tiles_h);
	typedef typename std::map<std::uint64_t, Sprite *>::iterator sprite_iterator;
	void require_redraw();
	//std::pair<sprite_iterator, sprite_iterator> iterate_sprites();


	//Constants:
	static const int tile_size = 8;
	static const int logical_screen_tile_width = 20;
	static const int logical_screen_tile_height = 18;
	static const int logical_screen_width = logical_screen_tile_width * tile_size;
	static const int logical_screen_height = logical_screen_tile_height * tile_size;
	static const int tilemap_width = Tilemap::w;
	static const int tilemap_height = Tilemap::h;
};

#include "../CodeGeneration/output/graphics_public.h"
