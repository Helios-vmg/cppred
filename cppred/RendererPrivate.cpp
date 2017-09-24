#include "RendererPrivate.h"
#include "utility.h"
#include <stdexcept>
#include <cassert>

#include "../CodeGeneration/output/graphics.inl"
#include "Engine.h"

static const RGB black = { 0, 0, 0, 0 };

Renderer::Pimpl::Pimpl(SDL_Window *window){
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
	Tile null = { 0, false, false };
	fill(this->bg_tilemap.tiles, null);
	fill(this->window_tilemap.tiles, null);
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
	return this->get_tilemap(region).tiles[x + y * Tilemap::w];
}

Tilemap &Renderer::Pimpl::get_tilemap(TileRegion region){
	switch (region){
		case TileRegion::Background:
			return this->bg_tilemap;
		case TileRegion::Window:
			return this->window_tilemap;
	}
	assert(false);
	return this->bg_tilemap;
}

void Renderer::Pimpl::render(){
	void *void_pixels;
	int pitch;
	if (SDL_LockTexture(this->main_texture, nullptr, &void_pixels, &pitch) >= 0){
		auto pixels = (RGB *)void_pixels;
		for (int y = 0; y < logical_screen_height; y++){
			for (int x = 0; x < logical_screen_width; x++){
				auto tile_no = this->bg_tilemap.tiles[x / tile_size + y / tile_size * Tilemap::w].tile_no;
				tile_no = tile_mapping[tile_no];
				int tile_offset_x = x % tile_size;
				int tile_offset_y = y % tile_size;
				auto color_index = this->tile_data[tile_no].data[tile_offset_x + tile_offset_y * tile_size];
				*(pixels++) = this->bg_palette.data[color_index];
			}
		}
		SDL_UnlockTexture(this->main_texture);
	}
	SDL_RenderCopy(this->renderer, this->main_texture, nullptr, nullptr);
	SDL_RenderPresent(this->renderer);
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
