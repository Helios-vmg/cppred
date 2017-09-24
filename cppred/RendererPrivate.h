#pragma once
#include "Renderer.h"
#include <SDL.h>
#include <vector>

struct TileData{
	static const size_t size = Renderer::tile_size * Renderer::tile_size;
	byte_t data[size];
};

struct RGB{
	byte_t r, g, b, a;
};

struct Palette{
	RGB data[256];
};

class Renderer::Pimpl{
	SDL_Renderer *renderer = nullptr;
	SDL_Texture *main_texture = nullptr;
	std::vector<TileData> tile_data;
	Tilemap bg_tilemap;
	Tilemap window_tilemap;
	Palette bg_palette;
	Palette sprite0_palette;
	Palette sprite1_palette;
	int scx = 0, scy = 0;
	int wx = 0, wy = 0;

	void initialize_sdl(SDL_Window *);
	void initialize_assets();
	void initialize_data();
public:
	Pimpl(SDL_Window *);
	~Pimpl();
	void set_palette(PaletteRegion region, byte_t value);
	void set_default_palettes();
	Tile &get_tile(TileRegion, int x, int y);
	Tilemap &get_tilemap(TileRegion);
	void render();
	void draw_image_to_tilemap(int x, int y, const GraphicsAsset &);
};

#include "../CodeGeneration/output/graphics_private.h"
