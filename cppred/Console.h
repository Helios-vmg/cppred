#pragma once
#include "common_types.h"
#include "CppRedAudioInterface.h"
#include "VideoDevice.h"
#include <SDL.h>
#include <memory>
#include <vector>
#include <boost/coroutine2/all.hpp>
#include "pokemon_version.h"

class Engine;

enum class ConsoleRequestId{
	None,
	GetAudioProgram,
	Restart,
	FlipVersion,
	GetVersion,
};

struct ConsoleCommunicationChannel{
	ConsoleRequestId request_id = ConsoleRequestId::None;
	CppRedAudioProgram *audio_program = nullptr;
	PokemonVersion version;
};

class Console{
	Engine *engine;
	VideoDevice *device;
	bool visible;
	Texture background;
	Texture text_layer;
	std::vector<byte_t> character_matrix;
	std::vector<byte_t> last_character_matrix;
	bool matrix_modified = false;
	Point matrix_size;

	typedef boost::coroutines2::asymmetric_coroutine<ConsoleCommunicationChannel *>::pull_type coroutine_t;
	typedef boost::coroutines2::asymmetric_coroutine<ConsoleCommunicationChannel *>::push_type yielder_t;
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
	CppRedAudioProgram &get_audio_program();

	int handle_menu(const std::vector<std::string> &, int default_item = 0, int item_separation = 1);
	void draw_long_menu(const std::vector<std::string> &strings, int item_separation = 1);
	void sound_test();
	void restart_game();
	void flip_version();
	PokemonVersion get_version();
public:
	Console(Engine &engine);
	void toggle_visible(){
		this->visible = !this->visible;
	}
	bool handle_event(const SDL_Event &);
	ConsoleCommunicationChannel *update();
	void render();
};
