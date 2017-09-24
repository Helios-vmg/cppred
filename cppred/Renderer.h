#pragma once
#include <memory>
#include "graphics_asset.h"

struct SDL_Window;

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

struct Tilemap{
	static const int w = 32;
	static const int h = 32;
	static const int size = w * h;
	Tile tiles[size];
};

class Renderer{
	class Pimpl;
	std::unique_ptr<void, void(*)(void *)> pimpl;
	Pimpl &get_pimpl();
public:
	Renderer(SDL_Window *);
	~Renderer();
	void set_palette(PaletteRegion region, byte_t value);
	void set_default_palettes();
	Tile &get_tile(TileRegion, int x, int y);
	Tilemap &get_tilemap(TileRegion);
	void render();
	void draw_image_to_tilemap(int x, int y, const GraphicsAsset &);

	static const int tile_size = 8;
	static const int logical_screen_tile_width = 20;
	static const int logical_screen_tile_height = 18;
	static const int logical_screen_width = logical_screen_tile_width * tile_size;
	static const int logical_screen_height = logical_screen_tile_height * tile_size;
	static const int tilemap_width = Tilemap::w;
	static const int tilemap_height = Tilemap::h;
};

#include "../CodeGeneration/output/graphics_public.h"
