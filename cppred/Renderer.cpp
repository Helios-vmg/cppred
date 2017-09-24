#include "RendererPrivate.h"
#include "utility.h"

Renderer::Renderer(Engine &engine, SDL_Window *window): pimpl(nullptr, deleter<Pimpl>){
	this->pimpl.reset(new Pimpl(engine, window));
}

Renderer::~Renderer() = default;

Renderer::Pimpl &Renderer::get_pimpl(){
	return *(Pimpl *)this->pimpl.get();
}

void Renderer::set_palette(PaletteRegion region, byte_t value){
	this->get_pimpl().set_palette(region, value);
}

void Renderer::set_default_palettes(){
	this->get_pimpl().set_default_palettes();
}

Tile &Renderer::get_tile(TileRegion region, int x, int y){
	return this->get_pimpl().get_tile(region, x, y);
}

Tilemap &Renderer::get_tilemap(TileRegion region){
	return this->get_pimpl().get_tilemap(region);
}

void Renderer::render(){
	this->get_pimpl().render();
}

void Renderer::draw_image_to_tilemap(int x, int y, const GraphicsAsset &asset){
	this->get_pimpl().draw_image_to_tilemap(x, y, asset);
}

void Renderer::clear_screen(){
	this->get_pimpl().clear_screen();
}

void Renderer::set_enable_bg(bool value){
	this->get_pimpl().set_enable_bg(value);
}

void Renderer::set_enable_window(bool value){
	this->get_pimpl().set_enable_window(value);
}

void Renderer::fill_rectangle(TileRegion region, int x, int y, int w, int h, int tile){
	this->get_pimpl().fill_rectangle(region, x, y, w, h, tile);
}

void Renderer::clear_sprites(){
	this->get_pimpl().clear_sprites();
}

std::shared_ptr<Sprite> Renderer::create_sprite(int tiles_w, int tiles_h){
	return this->get_pimpl().create_sprite(tiles_w, tiles_h);
}

void Renderer::require_redraw(){
	return this->get_pimpl().require_redraw();
}
