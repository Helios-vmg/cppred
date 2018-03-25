#pragma once
#include "GraphicsAsset.h"
#include "utility.h"
#include "RendererStructs.h"
#include "Sprite.h"
#include "VideoDevice.h"
#include <SDL.h>
#include <vector>
#include <deque>
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
	struct WindowLayer{
		Tilemap window_tilemap;
		Point window_offsets[logical_screen_height];
		Point window_origin;
		Point window_region_start;
		Point window_region_size;
	};

	struct RendererContext{
		Tilemap bg_tilemap;
		std::deque<WindowLayer> windows;
		WindowLayer *window;
		Palette bg_palette;
		Palette sprite0_palette;
		Palette sprite1_palette;
		Point bg_offsets[logical_screen_height];
		Point bg_global_offset = { 0, 0 };
		sprite_map_t sprites;
		bool enable_bg = false;
		bool enable_window = false;
		bool enable_sprites = true;

		RendererContext();
		RendererContext(const RendererContext &);
		void push_window();
		void pop_window();
	};

	VideoDevice *device;
	Texture main_texture;
	std::vector<TileData> tile_data;
	RGB final_palette[4];
	std::uint64_t next_sprite_id = 0;
	struct RenderPoint{
		int value;
		const Palette *palette;
		bool complete;
	};
	RenderPoint intermediate_render_surface[logical_screen_width * logical_screen_height];
	std::deque<RendererContext> stack;
	RendererContext *current_context;
	std::vector<Sprite *> sprite_list;

	RendererContext &context(){
		return *this->current_context;
	}
	const RendererContext &context() const{
		return *this->current_context;
	}

#define Renderer_DEFINE_ACCESSOR(name) \
	decltype(RendererContext::name) &name(){ return this->context().name; } \
	const decltype(RendererContext::name) &name() const{ return this->context().name; }
	
#define Renderer_DEFINE_WINDOW_ACCESSOR(name) \
	decltype(WindowLayer::name) &name(){ return this->context().window->name; } \
	const decltype(WindowLayer::name) &name() const{ return this->context().window->name; }

	Renderer_DEFINE_ACCESSOR(bg_tilemap)
	Renderer_DEFINE_WINDOW_ACCESSOR(window_tilemap)
	Renderer_DEFINE_ACCESSOR(bg_palette)
	Renderer_DEFINE_ACCESSOR(sprite0_palette)
	Renderer_DEFINE_ACCESSOR(sprite1_palette)
	Renderer_DEFINE_ACCESSOR(bg_offsets)
	Renderer_DEFINE_WINDOW_ACCESSOR(window_offsets)
	Renderer_DEFINE_ACCESSOR(bg_global_offset)
	Renderer_DEFINE_WINDOW_ACCESSOR(window_origin)
	Renderer_DEFINE_WINDOW_ACCESSOR(window_region_start)
	Renderer_DEFINE_WINDOW_ACCESSOR(window_region_size)
	Renderer_DEFINE_ACCESSOR(sprites)
	Renderer_DEFINE_ACCESSOR(enable_bg)
	Renderer_DEFINE_ACCESSOR(enable_window)
	Renderer_DEFINE_ACCESSOR(enable_sprites)
	Renderer_DEFINE_ACCESSOR(window)

	void initialize_assets();
	void initialize_data();
	void do_software_rendering();
	void render_background();
	void render_sprites(bool priority);
	void render_sprite(Sprite &, const Palette **);
	void render_window(const WindowLayer &);
	void render_windows();
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
	bool get_enable_window();
	void set_enable_sprites(bool value);
	void fill_rectangle(TileRegion region, const Point &corner, const Point &size, const Tile &tile);
	void clear_sprites();
	std::shared_ptr<Sprite> create_sprite(int tiles_w, int tiles_h);
	std::shared_ptr<Sprite> create_sprite(const GraphicsAsset &);
	//std::pair<sprite_iterator, sprite_iterator> iterate_sprites();
	void release_sprite(std::uint64_t);
	std::uint64_t get_id();
	const Point &get_bg_global_offset() const{
		return this->bg_global_offset();
	}
	void set_bg_global_offset(const Point &p){
		this->bg_global_offset() = p;
	}
	const Point &get_window_origin() const{
		return this->window_origin();
	}
	void set_window_origin(const Point &p){
		this->window_origin() = p;
	}
	const Point &get_window_region_start() const{
		return this->window_region_start();
	}
	void set_window_region_start(const Point &p){
		this->window_region_start() = p;
	}
	const Point &get_window_region_size() const{
		return this->window_region_size();
	}
	void set_window_region_size(const Point &p){
		this->window_region_size() = p;
	}

	void set_y_bg_offset(int y0, int y1, const Point &);
	void set_y_window_offset(int y0, int y1, const Point &);
	void push();
	void pop();
	void push_window();
	void pop_window();
};

class AutoRendererPusher{
	Renderer *renderer;
public:
	AutoRendererPusher(Renderer &renderer): renderer(&renderer){
		this->renderer->push();
	}
	AutoRendererPusher(const AutoRendererPusher &) = delete;
	AutoRendererPusher(AutoRendererPusher &&other){
		*this = std::move(other);
	}
	~AutoRendererPusher(){
		if (this->renderer)
			this->renderer->pop();
	}
	const AutoRendererPusher &operator=(const AutoRendererPusher &) = delete;
	const AutoRendererPusher &operator=(AutoRendererPusher &&other){
		if (this->renderer)
			this->renderer->pop();
		this->renderer = other.renderer;
		other.renderer = nullptr;
		return *this;
	}
};

class AutoRendererWindowPusher{
	Renderer *renderer;
public:
	AutoRendererWindowPusher(): renderer(nullptr){}
	AutoRendererWindowPusher(Renderer &renderer): renderer(&renderer){
		this->renderer->push_window();
	}
	AutoRendererWindowPusher(const AutoRendererPusher &) = delete;
	AutoRendererWindowPusher(AutoRendererPusher &&other){
		*this = std::move(other);
	}
	~AutoRendererWindowPusher(){
		if (this->renderer)
			this->renderer->pop_window();
	}
	const AutoRendererWindowPusher &operator=(const AutoRendererWindowPusher &) = delete;
	const AutoRendererWindowPusher &operator=(AutoRendererWindowPusher &&other){
		if (this->renderer)
			this->renderer->pop_window();
		this->renderer = other.renderer;
		other.renderer = nullptr;
		return *this;
	}
	void release(){
		this->renderer = nullptr;
	}
};

static const std::uint16_t white_arrow = (std::uint16_t)('A' + 128);
static const std::uint16_t black_arrow = (std::uint16_t)('B' + 128);
static const std::uint16_t down_arrow = (std::uint16_t)('C' + 128);
static const std::uint16_t multiplication_symbol = (std::uint16_t)'*';
static const std::uint16_t male_symbol = (std::uint16_t)'%';
static const std::uint16_t female_symbol = (std::uint16_t)'+';
static const std::uint16_t poke_symbol = (std::uint16_t)'{';
static const std::uint16_t mon_symbol = (std::uint16_t)'}';
static const std::uint16_t decimal_symbol = (std::uint16_t)('d' + 128);

#include "../CodeGeneration/output/graphics_public.h"
