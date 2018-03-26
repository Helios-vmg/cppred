#pragma once
#include "common_types.h"
#include "CppRed/AudioInterface.h"
#include "VideoDevice.h"
#include "pokemon_version.h"
#ifndef HAVE_PCH
#include <SDL.h>
#include <memory>
#include <vector>
#include <sstream>
#include <mutex>
#endif

class Engine;
class Coroutine;

enum class ConsoleRequestId{
	None,
	GetAudioProgram,
	Restart,
	FlipVersion,
	GetVersion,
};

struct ConsoleCommunicationChannel{
	ConsoleRequestId request_id = ConsoleRequestId::None;
	CppRed::AudioProgramInterface *audio_program = nullptr;
	PokemonVersion version;
};

class CharacterMatrix{
	Point shift;
	Point last_shift;
	std::vector<byte_t> character_matrix;
	std::vector<byte_t> last_character_matrix;
	Point matrix_size;
	bool matrix_modified = false;
	int text_scale;
public:
	CharacterMatrix(const Point &size, int scale);
	bool needs_redraw();
	void draw(RGB *dst, int w, int h);
	void write_character(int x, int y, byte_t character);
	void write_string(int x, int y, const char *string);
	void write_string2(const char *string);
	void clear();
	void set_shift(const Point &);
	DEFINE_GETTER(shift)
	DEFINE_GETTER(matrix_size)
	DEFINE_GETTER(text_scale)
};

class Console{
	Engine *engine;
	VideoDevice *device;
	bool visible;
	Texture background;
	Texture text_layer;
	Texture log_layer;
	CharacterMatrix console, log;
	bool log_enabled = false;
	std::mutex log_mutex;

	std::unique_ptr<Coroutine> coroutine;
	ConsoleCommunicationChannel *ccc = nullptr;

	int current_menu_position = -1;
	int current_menu_size = -1;
	bool selected = false;

	void initialize_background(byte_t alpha);
	static void blank_texture(SDL_Texture *, int width, int height);
	void initialize_text_layer(Texture &texture);

	void coroutine_entry_point();
	void yield();
	void yield(ConsoleCommunicationChannel &ccc);
	CppRed::AudioProgramInterface &get_audio_program();

	int handle_menu(const std::vector<std::string> &, int default_item = 0, int item_separation = 1);
	void draw_long_menu(const std::vector<std::string> &strings, int item_separation = 1);
	void sound_test();
	void cry_test();
	void restart_game();
	void flip_version();
	PokemonVersion get_version();
	static void draw(Texture &dst, CharacterMatrix &src);
	void draw_console_menu();
	void draw_console_log();
public:
	Console(Engine &engine);
	void toggle_visible(){
		this->visible = !this->visible;
	}
	bool handle_event(const SDL_Event &);
	ConsoleCommunicationChannel *update();
	void render();
	void log_string(const std::string &);
};

extern Console *global_console;

class Logger{
	bool active = true;
	std::stringstream stream;
public:
	Logger() = default;
	Logger(Logger &&logger): stream(std::move(stream)){
		logger.active = false;
	}
	~Logger(){
		if (global_console && this->active)
			global_console->log_string(this->stream.str());
	}
	template <typename T>
	void write(const T &data){
		this->stream << data;
	}
};

template <typename T>
Logger &&operator<<(Logger &&logger, const T &data){
	logger.write(data);
	return std::move(logger);
}
