#pragma once
#include "Renderer.h"
#include "Engine.h"
#include <SDL.h>
#include <vector>
#include <map>

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

class Renderer::Pimpl : public SpriteOwner{
	Engine *engine;
	SDL_Renderer *renderer = nullptr;
	SDL_Texture *main_texture = nullptr;
	std::vector<TileData> tile_data;
	Tilemap bg_tilemap;
	Tilemap window_tilemap;
	Palette bg_palette;
	Palette sprite0_palette;
	Palette sprite1_palette;
	Point bg_offsets[logical_screen_height];
	Point window_offsets[logical_screen_height];
	Point bg_global_offset = { 0, 0 };
	Point window_global_offset = { 0, 0 };
	std::map<std::uint64_t, Sprite *> sprites;
	std::vector<Sprite *> sprite_list;
	std::uint64_t next_sprite_id = 0;
	bool enable_bg = false;
	bool enable_window = false;
	bool enable_sprites = true;
	bool skip_rendering = true;

	void initialize_sdl(SDL_Window *);
	void initialize_assets();
	void initialize_data();
	void do_software_rendering();
public:
	Pimpl(Engine &engine, SDL_Window *);
	~Pimpl();
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
	std::shared_ptr<Sprite> create_sprite(int tiles_w, int tiles_h);
	void clear_sprites();
	void require_redraw();
	//std::pair<sprite_iterator, sprite_iterator> iterate_sprites();


	//Overrides:
	void release_sprite(std::uint64_t) override;
	std::uint64_t get_id() override;
};

#include "../CodeGeneration/output/graphics_private.h"
