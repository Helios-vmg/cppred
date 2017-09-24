#include "RendererPrivate.h"
#include "utility.h"

Renderer::Renderer(SDL_Window *window): pimpl(nullptr, deleter<Pimpl>){
	this->pimpl.reset(new Pimpl(window));
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
