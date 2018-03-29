 #include "stdafx.h"
#include "VideoDevice.h"
#ifndef HAVE_PCH
#include <SDL.h>
#include <SDL_gpu.h>
#include <string>
#endif

const char *vertex_shader =
#ifndef __ANDROID__
"#version 120\n"
#else
"#version 100\n"
"precision mediump int;\n"
"precision mediump float;\n"
#endif
"\n"
"attribute vec3 gpu_Vertex;\n"
"attribute vec2 gpu_TexCoord;\n"
"attribute vec4 gpu_Color;\n"
"uniform mat4 modelViewProjection;\n"
"\n"
"varying vec4 color;\n"
"varying vec2 texCoord;\n"
"\n"
"void main(void)\n"
"{\n"
"	color = gpu_Color;\n"
"	texCoord = vec2(gpu_TexCoord);\n"
"	gl_Position = modelViewProjection * vec4(gpu_Vertex, 1.0);\n"
"}\n";

VideoDevice::VideoDevice(const Point &size):
		window(nullptr, SDL_DestroyWindow),
		renderer(nullptr, SDL_DestroyRenderer){
	this->screen_size = size;
#if 0
	this->window.reset(SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, 0));
	if (!this->window)
		throw std::runtime_error("Failed to initialize SDL window.");
	this->renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_PRESENTVSYNC));
	if (!this->renderer)
		throw std::runtime_error("Failed to initialize SDL renderer.");
#else
	GPU_SetFullscreen(false, false);
	this->gpu_target = GPU_Init(size.x, size.y, GPU_DEFAULT_INIT_FLAGS | GPU_INIT_ENABLE_VSYNC);
	if (!this->gpu_target)
		throw std::runtime_error("Failed to initialize GPU target.");
	//this->init_shaders();
#endif
}

void VideoDevice::set_window_title(const char *title){
	SDL_SetWindowTitle(this->window.get(), title);
}

Texture VideoDevice::allocate_texture(int w, int h){
#if 0
	auto t = SDL_CreateTexture(this->renderer.get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	if (!t)
		return Texture();
	SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
#else
	auto t = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
	if (!t)
		return Texture();

	//SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };
	//GPU_SetColor(t, color);
	GPU_SetImageFilter(t, GPU_FILTER_NEAREST);

#endif
	return { t, {w, h} };
}

void VideoDevice::render_copy(const Texture &texture){
	GPU_Rect src_rect;
	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = texture.get_size().x;
	src_rect.h = texture.get_size().y;
	GPU_Rect dst_rect;
	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.w = this->screen_size.x;
	dst_rect.h = this->screen_size.y;
	GPU_BlitRect((GPU_Image *)texture.texture.get(), &src_rect, this->gpu_target, &dst_rect);
}

void VideoDevice::present(){
	GPU_FlushBlitBuffer();
	GPU_Flip(this->gpu_target);
}

Texture::Texture(): texture(nullptr, [](void *){}){}

Texture::Texture(SDL_Texture *t, const Point &size):
	texture(t, [](void *p){ SDL_DestroyTexture((SDL_Texture *)p); }),
	size(size){
}

Texture::Texture(GPU_Image *t, const Point &size):
	texture(t, [](void *p){ GPU_FreeImage((GPU_Image *)p); }),
	size(size){
}

Texture::Texture(Texture &&other): texture(std::move(other.texture)), size(other.size){}

const Texture &Texture::operator=(Texture &&other){
	this->texture = std::move(other.texture);
	this->size = other.size;
	return *this;
}

void Texture::replace_data(const void *buffer){
	GPU_UpdateImageBytes((GPU_Image *)this->texture.get(), nullptr, (const byte_t *)buffer, this->size.x * sizeof(RGB));
}

Shader::Shader(const char *source, bool fragment_shader){
	this->shader = GPU_CompileShader(fragment_shader ? GPU_FRAGMENT_SHADER : GPU_VERTEX_SHADER, source);
	if (!this->shader)
		throw std::runtime_error(GPU_GetShaderMessage());
}

Shader::~Shader(){
	if (this->shader)
		GPU_FreeShader(this->shader);
}

const Shader &Shader::operator=(Shader &&other){
	if (this->shader)
		GPU_FreeShader(this->shader);
	this->shader = other.shader;
	other.shader = 0;
	return *this;
}

void VideoDevice::init_shaders(){
	this->sp.add(Shader(vertex_shader, false));
	this->sp.activate();
}

const ShaderProgram &ShaderProgram::operator=(ShaderProgram &&other){
	this->program = other.program;
	other.program = 0;
	return *this;
}

ShaderProgram::~ShaderProgram(){
	if (this->program)
		GPU_FreeShaderProgram(this->program);
}

void ShaderProgram::initialize(){
	if (this->program || !this->shaders.size())
		return;
	std::vector<Uint32> shaders;
	shaders.reserve(this->shaders.size());
	for (auto &s : this->shaders)
		shaders.push_back(s.shader);
	this->program = GPU_LinkManyShaders(&shaders[0], (int)shaders.size());
	if (!this->program)
		throw std::runtime_error(GPU_GetShaderMessage());
	//auto uloc = GPU_GetUniformLocation(this->program, "tex");
	//GPU_SetUniformi(uloc, 0);
}

void ShaderProgram::add(Shader &&shader){
	this->shaders.emplace_back(std::move(shader));
}

void ShaderProgram::activate(){
	this->initialize();
	GPU_ShaderBlock block = GPU_LoadShaderBlock(this->program, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "modelViewProjection");
	GPU_ActivateShaderProgram(this->program, &block);
}
