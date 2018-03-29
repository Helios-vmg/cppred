#pragma once
#include "RendererStructs.h"
#ifndef HAVE_PCH
#include <memory>
#include <list>
#endif

#define DECLARE_C_STRUCT(name) struct name; typedef struct name name;

DECLARE_C_STRUCT(SDL_Texture);
DECLARE_C_STRUCT(SDL_Window);
DECLARE_C_STRUCT(SDL_Renderer);
DECLARE_C_STRUCT(GPU_Target);
DECLARE_C_STRUCT(GPU_Image);

class Texture{
	friend class VideoDevice;
	std::unique_ptr<void, void (*)(void *)> texture;
	Point size;
	
	Texture(SDL_Texture *, const Point &size);
	Texture(GPU_Image *, const Point &size);
public:
	Texture();
	Texture(const Texture &) = delete;
	Texture(Texture &&);
	void operator=(const Texture &) = delete;
	const Texture &operator=(Texture &&);
	bool operator!() const{
		return !this->texture;
	}
	const Point &get_size() const{
		return this->size;
	}
	void replace_data(const void *);
};

class Shader{
	friend class ShaderProgram;
	Uint32 shader;
public:
	Shader(): shader(0){}
	Shader(const char *source, bool fragment_shader = true);
	Shader(Shader &&other): shader(0){
		*this = std::move(other);
	}
	Shader(const Shader &) = delete;
	const Shader &operator=(const Shader &) = delete;
	const Shader &operator=(Shader &&);
	~Shader();
};

class ShaderProgram{
	Uint32 program;
	std::vector<Shader> shaders;
	void initialize();
public:
	ShaderProgram(): program(0) {}
	ShaderProgram(ShaderProgram &&other): program(0){
		*this = std::move(other);
	}
	ShaderProgram(const ShaderProgram &) = delete;
	const ShaderProgram &operator=(const ShaderProgram &) = delete;
	const ShaderProgram &operator=(ShaderProgram &&);
	~ShaderProgram();
	void add(Shader &&shader);
	void activate();
};

class VideoDevice{
	std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window;
	std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> renderer;
	GPU_Target *gpu_target;
	Point screen_size;
	ShaderProgram sp;

	void init_shaders();
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
