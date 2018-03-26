#include "stdafx.h"
#include "VideoDevice.h"
#ifndef HAVE_PCH
#include <SDL.h>
#include <string>
#endif

VideoDevice::VideoDevice(const Point &size):
		window(nullptr, SDL_DestroyWindow),
		renderer(nullptr, SDL_DestroyRenderer){
	this->window.reset(SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, 0));
	this->screen_size = size;
	if (!this->window)
		throw std::runtime_error("Failed to initialize SDL window.");
	this->renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_PRESENTVSYNC));
	if (!this->renderer)
		throw std::runtime_error("Failed to initialize SDL renderer.");
}

void VideoDevice::set_window_title(const char *title){
	SDL_SetWindowTitle(this->window.get(), title);
}

Texture VideoDevice::allocate_texture(int w, int h){
	auto t = SDL_CreateTexture(this->renderer.get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (!t)
		return Texture();
	SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
	return { t, {w, h} };
}

void VideoDevice::render_copy(const Texture &texture){
	SDL_RenderCopy(this->renderer.get(), texture.texture.get(), nullptr, nullptr);
}

void VideoDevice::present(){
	SDL_RenderPresent(this->renderer.get());
}

Texture::Texture(): texture(nullptr, SDL_DestroyTexture){}

Texture::Texture(SDL_Texture *t, const Point &size): texture(t, SDL_DestroyTexture), size(size){}

Texture::Texture(Texture &&other): texture(std::move(other.texture)), size(other.size){}

const Texture &Texture::operator=(Texture &&other){
	this->texture = std::move(other.texture);
	this->size = other.size;
	return *this;
}

bool Texture::try_lock(TextureSurface &dst){
	return !dst.try_lock(this->texture.get(), this->size);
}

TextureSurface Texture::lock(){
	TextureSurface ret;
	this->try_lock(ret);
	//If we reach here, the texture is locked.
	return ret;
}

TextureSurface::TextureSurface(SDL_Texture *t, const Point &size){
	auto error = this->try_lock(t, size);
	if (error)
		throw std::runtime_error((std::string)"TextureSurface::TextureSurface(): " + error);
}

const char *TextureSurface::try_lock(SDL_Texture *texture, const Point &size){
	if (this->texture)
		throw std::runtime_error("TextureSurface::try_lock(): Invalid usage.");
	this->texture = texture;
	this->size = size;

	void *void_pixels;
	int pitch;
	if (SDL_LockTexture(this->texture, nullptr, &void_pixels, &pitch) < 0)
		return SDL_GetError();
	static_assert(sizeof(RGB) == 4, "RGB struct has been padded too far!");
	if (pitch != this->size.x * sizeof(RGB)){
		SDL_UnlockTexture(this->texture);
		return "Invalid texture format";
	}
	this->pixels = (RGB *)void_pixels;
	return nullptr;
}

TextureSurface::TextureSurface(){
	this->texture = nullptr;
	this->pixels = nullptr;
	this->size = { 0, 0 };
}

TextureSurface::TextureSurface(TextureSurface &&other){
	*this = std::move(other);
}

TextureSurface::~TextureSurface(){
	if (this->texture)
		SDL_UnlockTexture(this->texture);
}

const TextureSurface &TextureSurface::operator=(TextureSurface &&other){
	if (this->texture)
		SDL_UnlockTexture(this->texture);
	set_and_swap(this->texture, other.texture, nullptr);
	set_and_swap(this->pixels, other.pixels, nullptr);
	set_and_swap(this->size, other.size, Point{ 0, 0 });
	return *this;
}
