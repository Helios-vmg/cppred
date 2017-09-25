#include "Renderer.h"
#include "utility.h"
#include <stdexcept>
#include <cassert>
#include <iostream>
#include "Engine.h"

#include "../CodeGeneration/output/graphics.inl"

//#define MEASURE_RENDERING_TIMES
#define ALWAYS_RENDER

Renderer::Renderer(Engine &engine, SDL_Window *window): engine(&engine){
	this->initialize_sdl(window);
	this->initialize_assets();
	this->initialize_data();
}

Renderer::~Renderer(){
	SDL_DestroyTexture(this->main_texture);
	SDL_DestroyRenderer(this->renderer);
}

void Renderer::initialize_sdl(SDL_Window *window){
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

void Renderer::initialize_assets(){
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

void Renderer::initialize_data(){
	this->clear_screen();
	this->bg_palette = null_palette;
	this->sprite0_palette = null_palette;
	this->sprite1_palette = null_palette;
	for (int i = 0; i < 4; i++){
		byte_t c = (3 - i) * 0x55;
		this->final_palette[i] = { c, c, c, 0xFF };
	}
	this->set_default_palettes();
	this->set_palette(PaletteRegion::Sprites1, 0);
}

void Renderer::set_palette(PaletteRegion region, Palette value){
	this->skip_rendering = false;
	switch (region){
		case PaletteRegion::Background:
			this->bg_palette = value;
			break;
		case PaletteRegion::Sprites0:
			this->sprite0_palette = value;
			break;
		case PaletteRegion::Sprites1:
			this->sprite1_palette = value;
			break;
	}
}

void Renderer::set_default_palettes(){
	this->set_palette(PaletteRegion::Background, BITMAP(11100100));
	this->set_palette(PaletteRegion::Sprites0, BITMAP(11010000));
	this->clear_subpalettes(SubPaletteRegion::Background);
	this->clear_subpalettes(SubPaletteRegion::Window);
	this->clear_subpalettes(SubPaletteRegion::Sprites);
}

Tile &Renderer::get_tile(TileRegion region, int x, int y){
	this->skip_rendering = false;
	return this->get_tilemap(region).tiles[x + y * Tilemap::w];
}

Tilemap &Renderer::get_tilemap(TileRegion region){
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

void Renderer::do_software_rendering(){
#ifdef MEASURE_RENDERING_TIMES
	auto t0 = this->engine->get_clock();
#endif
#ifndef ALWAYS_RENDER
	if (this->skip_rendering)
		return;
#endif
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
			const Palette *palette = nullptr;
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
				palette = &tile.palette;
				if (!*palette)
					palette = &this->bg_palette;
			}

			if (true || this->enable_sprites){
				for (auto sprite : this->sprite_list){
					auto spry = sprite->get_y();
					auto sprx = sprite->get_x();
					if (!sprite->get_visible() | spry >= logical_screen_height | sprx >= logical_screen_width)
						continue;

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
					palette = &tile.palette;
					if (!*palette){
						palette = &sprite->get_palette();
						if (!*palette)
							palette = sprite->get_palette_region() == PaletteRegion::Sprites0 ? &this->sprite0_palette : &this->sprite1_palette;
					}
					break;
				}
			}

			if (color_index < 0)
				*(pixels++) = this->final_palette[0];
			else
				*(pixels++) = this->final_palette[palette->data[color_index]];
		}
	}
	SDL_UnlockTexture(this->main_texture);
#ifdef MEASURE_RENDERING_TIMES
	auto t1 = this->engine->get_clock();
	std::cout << "Rendering time: " << (t1 - t0) * 1000 << " ms\n";
#endif
}

void Renderer::render(){
	this->do_software_rendering();
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
	SDL_RenderPresent(this->renderer);
	this->skip_rendering = true;
}

std::vector<Point> Renderer::draw_image_to_tilemap(const Point &corner, const GraphicsAsset &asset, Palette palette){
	auto x = corner.x;
	auto y = corner.y;
	std::vector<Point> ret;
	ret.reserve(Tilemap::w * Tilemap::h);
	for (int i = 0; i < asset.height && y + i < Tilemap::h; i++){
		for (int j = 0; j < asset.width && x + j < Tilemap::w; j++){
			auto x2 = x + j;
			auto y2 = y + i;
			ret.push_back({ x2, y2 });
			auto &tile = this->bg_tilemap.tiles[x2 + y2 * Tilemap::w];
			tile.tile_no = asset.first_tile + j + i * asset.width;
			tile.flipped_x = false;
			tile.flipped_y = false;
			tile.palette = palette;
		}
	}
	return ret;
}

void Renderer::mass_set_palettes(const std::vector<Point> &tiles, Palette palette){
	for (auto &p : tiles)
		this->get_tile(TileRegion::Background, p.x, p.y).palette = palette;
}

void Renderer::clear_subpalettes(SubPaletteRegion region){
	this->require_redraw();
	switch (region){
		case SubPaletteRegion::Background:
			for (auto &tile : this->get_tilemap(TileRegion::Background).tiles)
				tile.palette = null_palette;
			break;
		case SubPaletteRegion::Window:
			for (auto &tile : this->get_tilemap(TileRegion::Window).tiles)
				tile.palette = null_palette;
			break;
		case SubPaletteRegion::Sprites:
			for (auto &sprite : this->sprites){
				sprite.second->set_palette(null_palette);
				auto its = sprite.second->iterate_tiles();
				for (auto it = its.first; it != its.second; ++it)
					it->palette = null_palette;
			}
			break;
	}
}

void Renderer::clear_screen(){
	Tile null;
	fill(this->bg_tilemap.tiles, null);
	fill(this->window_tilemap.tiles, null);
	Point zero = { 0, 0 };
	fill(this->bg_offsets, zero);
	fill(this->window_offsets, zero);
	this->bg_global_offset = zero;
	this->window_global_offset = zero;
	this->skip_rendering = false;
}

void Renderer::set_enable_bg(bool value){
	this->enable_bg = value;
	this->skip_rendering = false;
}

void Renderer::set_enable_window(bool value){
	this->enable_window = value;
	this->skip_rendering = false;
}

void Renderer::fill_rectangle(TileRegion region, const Point &corner, const Point &size, const Tile &tile){
	if (tile.tile_no < 0)
		return;
	auto tile_copy = tile;
	tile_copy.tile_no %= tile_mapping_size;
	int x0 = std::max(corner.x, 0);
	int y0 = std::max(corner.y, 0);
	int x1 = std::min(corner.x + size.x, Tilemap::w);
	int y1 = std::min(corner.y + size.y, Tilemap::h);

	auto &tilemap = this->get_tilemap(region);
	for (int y2 = y0; y2 < y1; y2++)
		for (int x2 = x0; x2 < x1; x2++)
			tilemap.tiles[x2 + y2 * Tilemap::w] = tile_copy;
}

std::shared_ptr<Sprite> Renderer::create_sprite(int tiles_w, int tiles_h){
	auto ret = std::make_shared<Sprite>(*this, tiles_w, tiles_h);
	this->sprites[ret->get_id()] = ret.get();
	if (this->sprite_list.capacity() < this->sprites.size())
		this->sprite_list.reserve(this->sprites.size());
	return ret;
}

std::shared_ptr<Sprite> Renderer::create_sprite(const GraphicsAsset &asset){
	auto ret = this->create_sprite(asset.width, asset.height);
	auto its = ret->iterate_tiles();
	auto i = asset.first_tile;
	for (auto it = its.first; it != its.second; ++it)
		it->tile_no = i++;
	return ret;
}

void Renderer::clear_sprites(){
	this->sprites.clear();
}

void Renderer::release_sprite(std::uint64_t id){
	auto it = this->sprites.find(id);
	if (it == this->sprites.end())
		return;
	this->sprites.erase(it);
}

std::uint64_t Renderer::get_id(){
	return this->next_sprite_id++;
}
