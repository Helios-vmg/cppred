#pragma once
#include "GraphicsAsset.h"
#include "utility.h"
#include "RendererStructs.h"
#include "Sprite.h"
#include "VideoDevice.h"
#include <SDL.h>
#include <vector>
#include <map>
#include <memory>

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
	VideoDevice *device;
	Texture main_texture;
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
	struct RenderPoint{
		int value;
		const Palette *palette;
	};
	RenderPoint intermediate_render_surface[logical_screen_width * logical_screen_height];

	void initialize_assets();
	void initialize_data();
	void do_software_rendering();
	void render_non_sprites();
	void render_sprites();
	void render_sprite(Sprite &, const Palette **);
	void final_render(TextureSurface &);
	void set_y_offset(Point (&)[logical_screen_height], int y0, int y1, const Point &);
	std::vector<Point> draw_image_to_tilemap_internal(const Point &corner, const GraphicsAsset &, TileRegion, Palette, bool);
public:
	Renderer(VideoDevice &);
	static std::unique_ptr<VideoDevice> initialize_device(int scale);
	Renderer(const Renderer &) = delete;
	Renderer(Renderer &&) = delete;
	void operator=(const Renderer &) = delete;
	void operator=(Renderer &&) = delete;
	//SDL_Renderer *get_renderer() const{
	//	return this->renderer;
	//}
	VideoDevice &get_device(){
		return *this->device;
	}
	void set_palette(PaletteRegion region, Palette value);
	void set_default_palettes();
	Tile &get_tile(TileRegion, const Point &p);
	Tilemap &get_tilemap(TileRegion);
	void render();
	std::vector<Point> draw_image_to_tilemap(const Point &corner, const GraphicsAsset &, TileRegion = TileRegion::Background, Palette = null_palette);
	std::vector<Point> draw_image_to_tilemap_flipped(const Point &corner, const GraphicsAsset &, TileRegion = TileRegion::Background, Palette = null_palette);
	void mass_set_palettes(const std::vector<Point> &tiles, Palette palette);
	void mass_set_tiles(const std::vector<Point> &tiles, const Tile &);
	void clear_subpalettes(SubPaletteRegion);
	void clear_screen();
	void set_enable_bg(bool value);
	void set_enable_window(bool value);
	void set_enable_sprites(bool value);
	void fill_rectangle(TileRegion region, const Point &corner, const Point &size, const Tile &tile);
	void clear_sprites();
	std::shared_ptr<Sprite> create_sprite(int tiles_w, int tiles_h);
	std::shared_ptr<Sprite> create_sprite(const GraphicsAsset &);
	//std::pair<sprite_iterator, sprite_iterator> iterate_sprites();
	void release_sprite(std::uint64_t);
	std::uint64_t get_id();
	DEFINE_GETTER_SETTER(bg_global_offset)
	DEFINE_GETTER_SETTER(window_global_offset)
	void set_y_bg_offset(int y0, int y1, const Point &);
	void set_y_window_offset(int y0, int y1, const Point &);
};

static const std::uint16_t white_arrow = (std::uint16_t)('A' + 128);
static const std::uint16_t black_arrow = (std::uint16_t)('B' + 128);
static const std::uint16_t down_arrow = (std::uint16_t)('C' + 128);

#include "../CodeGeneration/output/graphics_public.h"
