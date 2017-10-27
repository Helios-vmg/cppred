#pragma once
#include <SDL.h>
#include <memory>
#include "common_types.h"
#include <vector>
#include <boost/coroutine2/all.hpp>

class Engine;
class Renderer;

class Console{
	Engine *engine;
	Renderer *renderer;
	bool visible;
	std::unique_ptr<SDL_Texture, void(*)(SDL_Texture *)> background;
	std::unique_ptr<SDL_Texture, void(*)(SDL_Texture *)> text_layer;
	std::vector<byte_t> character_matrix;
	std::vector<byte_t> last_character_matrix;
	bool matrix_modified = false;

	typedef boost::coroutines2::asymmetric_coroutine<void>::pull_type coroutine_t;
	typedef boost::coroutines2::asymmetric_coroutine<void>::push_type yielder_t;
	std::unique_ptr<coroutine_t> coroutine;
	yielder_t *yielder = nullptr;

	int current_menu_position = -1;
	int current_menu_size = -1;
	bool selected = false;

	void initialize_background(byte_t alpha);
	static void blank_texture(SDL_Texture *, int width, int height);
	void initialize_text_layer();
	void write_character(int x, int y, byte_t character);
	void write_string(int x, int y, const char *string);

	void coroutine_entry_point();
	void yield();

	int handle_menu(const std::vector<std::string> &, int default_item = 0, int item_separation = 1);
	void draw_long_menu(const std::vector<std::string> &strings, int item_separation = 1);
	void sound_test();

public:
	Console(Engine &engine);
	void toggle_visible(){
		this->visible = !this->visible;
	}
	bool handle_event(const SDL_Event &);
	void update();
	void render();
};
