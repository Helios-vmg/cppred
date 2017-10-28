#pragma once
#include "RendererStructs.h"
#include <SDL.h>
#include <memory>

class TextureSurface{
	friend class Texture;
	SDL_Texture *texture = nullptr;
	RGB *pixels;
	Point size;

	TextureSurface(SDL_Texture *, const Point &size);
	const char *try_lock(SDL_Texture *, const Point &size);
public:
	TextureSurface();
	TextureSurface(const TextureSurface &) = delete;
	TextureSurface(TextureSurface &&);
	~TextureSurface();
	void operator=(const TextureSurface &) = delete;
	const TextureSurface &operator=(TextureSurface &&);
	RGB &get_pixel(int x, int y){
		return this->pixels[x + y * this->size.x];
	}
	RGB *get_row(int y){
		return this->pixels + y * this->size.x;
	}
	const Point &get_size() const{
		return this->size;
	}
};

class Texture{
	friend class VideoDevice;
	std::unique_ptr<SDL_Texture, void(*)(SDL_Texture *)> texture;
	Point size;
	
	Texture(SDL_Texture *, const Point &size);
public:
	Texture();
	Texture(const Texture &) = delete;
	Texture(Texture &&);
	void operator=(const Texture &) = delete;
	const Texture &operator=(Texture &&);
	TextureSurface lock();
	bool try_lock(TextureSurface &dst);
	bool operator!() const{
		return !this->texture;
	}
	const Point &get_size() const{
		return this->size;
	}
};

class VideoDevice{
	std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window;
	std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> renderer;
	Point screen_size;
public:
	VideoDevice(const Point &size);
	void set_window_title(const char *);
	Point get_screen_size() const{
		return this->screen_size;
	}
	Texture allocate_texture(int w, int h);
	Texture allocate_texture(const Point &p){
		return this->allocate_texture(p.x, p.y);
	}
	void render_copy(const Texture &);
	void present();
};
