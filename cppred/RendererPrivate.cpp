#include "RendererPrivate.h"
#include "utility.h"
#include <stdexcept>
#include <cassert>
#include <iostream>
#include "Engine.h"

#include "../CodeGeneration/output/graphics.inl"

static const RGB black = { 0, 0, 0, 0 };

Renderer::Pimpl::Pimpl(Engine &engine, SDL_Window *window): engine(&engine){
	this->initialize_sdl(window);
	this->initialize_assets();
	this->initialize_data();
}

Renderer::Pimpl::~Pimpl(){
	SDL_DestroyTexture(this->main_texture);
	SDL_DestroyRenderer(this->renderer);
}

void Renderer::Pimpl::initialize_sdl(SDL_Window *window){
	this->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!this->renderer)
		throw std::runtime_error("Failed to initialize SDL renderer.");
	this->main_texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, logical_screen_width, logical_screen_height);
	if (!this->main_texture)
		throw std::runtime_error("Failed to create main texture.");

	void *void_pixels;
	int pitch;
	if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) >= 0){
		assert(pitch == logical_screen_width * 4);
		memset(void_pixels, 0xFF, pitch * logical_screen_height);
		SDL_UnlockTexture(this->main_texture);
	}
}

void Renderer::Pimpl::initialize_assets(){
	static_assert(packed_image_data_size * 4 % TileData::size == 0, "");
	this->tile_data.resize(packed_image_data_size * 4 / TileData::size);

	for (size_t i = 0; i < this->tile_data.size(); i++){
		auto &tile = this->tile_data[i];
		size_t offset = 0;
		for (int y = 0; y < tile_size; y++){
			int shift = 0;
			for (int x = 0; x < tile_size; x++, offset++, shift = (shift + 2) % 8)
				tile.data[offset] = (packed_image_data[(i * TileData::size + offset) / 4] >> shift) & BITMAP(00000011);
		}
	}
}

void Renderer::Pimpl::initialize_data(){
	this->clear_screen();
	fill(this->bg_palette.data, black);
	fill(this->sprite0_palette.data, black);
	fill(this->sprite1_palette.data, black);
	this->set_default_palettes();
	this->set_palette(PaletteRegion::Sprites1, 0);
}

template <bool BG>
static void set_palette_array(RGB dst[4], byte_t palette){
	for (unsigned i = 4; i--;){
		auto index = (palette >> (i * 2)) & 3;
		byte_t c = ~(byte_t)(index * 0xFF / 3);
		dst[i] = { c, c, c, 0xFF };
	}
	if (!BG)
		dst[0] = { 0, 0, 0, 0 };
}

void Renderer::Pimpl::set_palette(PaletteRegion region, byte_t value){
	this->skip_rendering = false;
	switch (region){
		case PaletteRegion::Background:
			set_palette_array<true>(this->bg_palette.data, value);
			break;
		case PaletteRegion::Sprites0:
			set_palette_array<false>(this->sprite0_palette.data, value);
			break;
		case PaletteRegion::Sprites1:
			set_palette_array<false>(this->sprite1_palette.data, value);
			break;
	}
}

void Renderer::Pimpl::set_default_palettes(){
	this->set_palette(PaletteRegion::Background, BITMAP(11100100));
	this->set_palette(PaletteRegion::Sprites0, BITMAP(11010000));
}

Tile &Renderer::Pimpl::get_tile(TileRegion region, int x, int y){
	this->skip_rendering = false;
	return this->get_tilemap(region).tiles[x + y * Tilemap::w];
}

Tilemap &Renderer::Pimpl::get_tilemap(TileRegion region){
	this->skip_rendering = false;
	switch (region){
		case TileRegion::Background:
			return this->bg_tilemap;
		case TileRegion::Window:
			return this->window_tilemap;
	}
	assert(false);
	return this->bg_tilemap;
}

bool sort_sprites(Sprite *a, Sprite *b){
	if (a->get_x() > b->get_x())
		return true;
	if (a->get_x() < b->get_x())
		return false;
	return a->get_id() < b->get_id();
}

//#define MEASURE_RENDERING_TIMES

void Renderer::Pimpl::do_software_rendering(){
#ifdef MEASURE_RENDERING_TIMES
	auto t0 = this->engine->get_clock();
#endif
	if (this->skip_rendering)
		return;
	void *void_pixels;
	int pitch;
	if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) < 0)
		return;

	this->sprite_list.clear();
	for (auto &kv : this->sprites)
		this->sprite_list.push_back(kv.second);

	std::sort(this->sprite_list.begin(), this->sprite_list.end(), sort_sprites);

	auto pixels = (RGB *)void_pixels;
	for (int y = 0; y < logical_screen_height; y++){
		auto bg_offset = this->bg_global_offset + this->bg_offsets[y];
		auto window_offset = this->window_global_offset + this->window_offsets[y];
		for (int x = 0; x < logical_screen_width; x++){
			int color_index = -1;
			Palette *palette = nullptr;
			if (this->enable_bg){
				auto p = bg_offset;
				p.x += x;
				p.y += y;
				p.x = euclidean_modulo(p.x, Tilemap::w * tile_size);
				p.y = euclidean_modulo(p.y, Tilemap::h * tile_size);
				auto &tile = this->bg_tilemap.tiles[p.x / tile_size + p.y / tile_size * Tilemap::w];
				auto tile_no = tile.tile_no;
				tile_no = tile_mapping[tile_no];
				int tile_offset_x = p.x % tile_size;
				int tile_offset_y = p.y % tile_size;
				if (tile.flipped_x)
					tile_offset_x = (tile_size - 1) - tile_offset_x;
				if (tile.flipped_y)
					tile_offset_y = (tile_size - 1) - tile_offset_y;
				color_index = this->tile_data[tile_no].data[tile_offset_x + tile_offset_y * tile_size];
				palette = &this->bg_palette;
			}

			for (auto sprite : this->sprite_list){
				auto sprx = sprite->get_x();
				auto spry = sprite->get_y();
				auto sprite_is_here = (x >= sprx) & (x < sprx + sprite->get_w() * tile_size) & (y >= spry) & (y < spry + sprite->get_h() * tile_size);
				if (!sprite_is_here)
					continue;

				auto sprite_offset_x = x - sprx;
				auto sprite_offset_y = y - spry;
				auto sprite_tile_x = sprite_offset_x / tile_size;
				auto sprite_tile_y = sprite_offset_y / tile_size;

				auto &tile = sprite->get_tile(sprite_tile_x, sprite_tile_y);
				auto sprite_is_not_covered_here = tile.has_priority | !color_index;
				if (!sprite_is_not_covered_here)
					continue;

				auto tile_no = tile.tile_no;
				tile_no = tile_mapping[tile_no];
				int tile_offset_x = sprite_offset_x % tile_size;
				int tile_offset_y = sprite_offset_y % tile_size;
				if (tile.flipped_x)
					tile_offset_x = (tile_size - 1) - tile_offset_x;
				if (tile.flipped_y)
					tile_offset_y = (tile_size - 1) - tile_offset_y;
				auto index = this->tile_data[tile_no].data[tile_offset_x + tile_offset_y * tile_size];
				if (!index)
					continue;
				color_index = index;
				palette = sprite->get_palette() == PaletteRegion::Sprites0 ? &this->sprite0_palette : &this->sprite1_palette;
				break;
			}

			*(pixels++) = palette->data[color_index];
		}
	}
	SDL_UnlockTexture(this->main_texture);
#ifdef MEASURE_RENDERING_TIMES
	auto t1 = this->engine->get_clock();
	std::cout << "Rendering time: " << (t1 - t0) * 1000 << " ms\n";
#endif
}

void Renderer::Pimpl::render(){
	this->do_software_rendering();
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
	SDL_RenderPresent(this->renderer);
	this->skip_rendering = true;
}

void Renderer::Pimpl::draw_image_to_tilemap(int x, int y, const GraphicsAsset &asset){
	for (int i = 0; i < asset.height && y + i < Tilemap::h; i++){
		for (int j = 0; j < asset.width && x + j < Tilemap::w; j++){
			auto &tile = this->bg_tilemap.tiles[x + j + (y + i) * Tilemap::w];
			tile.tile_no = asset.first_tile + j + i * asset.width;
			tile.flipped_x = false;
			tile.flipped_y = false;
		}
	}
}

void Renderer::Pimpl::clear_screen(){
	Tile null = { 0, false, false };
	fill(this->bg_tilemap.tiles, null);
	fill(this->window_tilemap.tiles, null);
	Point zero = { 0, 0 };
	fill(this->bg_offsets, zero);
	fill(this->window_offsets, zero);
	this->bg_global_offset = zero;
	this->window_global_offset = zero;
	this->skip_rendering = false;
}

void Renderer::Pimpl::set_enable_bg(bool value){
	this->enable_bg = value;
	this->skip_rendering = false;
}

void Renderer::Pimpl::set_enable_window(bool value){
	this->enable_window = value;
	this->skip_rendering = false;
}

void Renderer::Pimpl::fill_rectangle(TileRegion region, int x, int y, int w, int h, int tile){
	if (tile < 0)
		return;
	tile %= tile_mapping_size;
	int x0 = std::max(x, 0);
	int y0 = std::max(y, 0);
	int x1 = std::min(x + w, Tilemap::w);
	int y1 = std::min(y + h, Tilemap::h);

	auto &tilemap = this->get_tilemap(region);
	for (int y2 = y0; y2 < y1; y2++)
		for (int x2 = x0; x2 < x1; x2++)
			tilemap.tiles[x2 + y2 * Tilemap::w].tile_no = tile;
}

std::shared_ptr<Sprite> Renderer::Pimpl::create_sprite(int tiles_w, int tiles_h){
	auto ret = std::make_shared<Sprite>(*this, tiles_w, tiles_h);
	this->sprites[ret->get_id()] = ret.get();
	if (this->sprite_list.capacity() < this->sprites.size())
		this->sprite_list.reserve(this->sprites.size());
	return ret;
}

void Renderer::Pimpl::clear_sprites(){
	this->sprites.clear();
}

void Renderer::Pimpl::release_sprite(std::uint64_t id){
	auto it = this->sprites.find(id);
	if (it == this->sprites.end())
		return;
	this->sprites.erase(it);
}

std::uint64_t Renderer::Pimpl::get_id(){
	return this->next_sprite_id++;
}

Sprite::Sprite(SpriteOwner &owner, int w, int h){
	if (w <= 0 || h <= 0)
		throw std::runtime_error("Sprite::Sprite(): Size must be >= 1x1");
	this->owner = &owner;
	this->id = owner.get_id();
	this->y = this->x = 0;
	this->w = w;
	this->h = h;
	this->tiles.resize(w * h);

	SpriteTile tile;
	tile.tile_no = 0;
	tile.flipped_x = false;
	tile.flipped_y = false;
	tile.has_priority = false;
	fill(this->tiles, tile);
}

Sprite::~Sprite(){
	if (this->owner)
		this->owner->release_sprite(this->id);
}

SpriteTile &Sprite::get_tile(int x, int y){
	if (x < 0 | y < 0 | x >= this->w | y >= this->h)
		throw std::runtime_error("Invalid coordinates.");
	return this->tiles[x + y * this->w];
}

void Renderer::Pimpl::require_redraw(){
	this->skip_rendering = false;
}
