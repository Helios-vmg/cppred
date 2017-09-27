#pragma once
#include "GraphicsAsset.h"
#include "utility.h"
#include "RendererStructs.h"
#include "Sprite.h"
#include <SDL.h>
#include <vector>
#include <map>
#include <memory>

struct SDL_Window;
class Engine;

class Renderer{
public:
	//Constants:
	static const int tile_size = 8;
	static const int logical_screen_tile_width = 20;
	static const int logical_screen_tile_height = 18;
	static const int logical_screen_width = logical_screen_tile_width * tile_size;
	static const int logical_screen_height = logical_screen_tile_height * tile_size;
	static const int tilemap_width = Tilemap::w;
	static const int tilemap_height = Tilemap::h;
	//Types:
	typedef BasicTileData<tile_size> TileData;
	typedef std::map<std::uint64_t, Sprite *> sprite_map_t;
	typedef typename sprite_map_t::iterator sprite_iterator;

private:
	Engine *engine;
	SDL_Renderer *renderer = nullptr;
	SDL_Texture *main_texture = nullptr;
	std::vector<TileData> tile_data;
	Tilemap bg_tilemap;
	Tilemap window_tilemap;
	RGB final_palette[4];
	Palette bg_palette;
	Palette sprite0_palette;
	Palette sprite1_palette;
	Point bg_offsets[logical_screen_height];
	Point window_offsets[logical_screen_height];
	Point bg_global_offset = { 0, 0 };
	Point window_global_offset = { 0, 0 };
	sprite_map_t sprites;
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
	void set_y_offset(Point(&)[logical_screen_height], int y0, int y1, const Point &);
public:
	Renderer(Engine &, SDL_Window *);
	~Renderer();
	Renderer(const Renderer &) = delete;
	Renderer(Renderer &&) = delete;
	void operator=(const Renderer &) = delete;
	void operator=(Renderer &&) = delete;
	void set_palette(PaletteRegion region, Palette value);
	void set_default_palettes();
	Tile &get_tile(TileRegion, int x, int y);
	Tilemap &get_tilemap(TileRegion);
	void render();
	std::vector<Point> draw_image_to_tilemap(const Point &corner, const GraphicsAsset &, TileRegion = TileRegion::Background, Palette = null_palette);
	void mass_set_palettes(const std::vector<Point> &tiles, Palette palette);
	void clear_subpalettes(SubPaletteRegion);
	void clear_screen();
	void set_enable_bg(bool value);
	void set_enable_window(bool value);
	void set_enable_sprites(bool value);
	void fill_rectangle(TileRegion region, const Point &corner, const Point &size, const Tile &tile);
	void clear_sprites();
	std::shared_ptr<Sprite> create_sprite(int tiles_w, int tiles_h);
	std::shared_ptr<Sprite> create_sprite(const GraphicsAsset &);
	void require_redraw(){
		this->skip_rendering = false;
	}
	//std::pair<sprite_iterator, sprite_iterator> iterate_sprites();
	void release_sprite(std::uint64_t);
	std::uint64_t get_id();
	DEFINE_GETTER_SETTER(bg_global_offset)
	DEFINE_GETTER_SETTER(window_global_offset)
	void set_y_bg_offset(int y0, int y1, const Point &);
	void set_y_window_offset(int y0, int y1, const Point &);
};

#include "../CodeGeneration/output/graphics_public.h"
