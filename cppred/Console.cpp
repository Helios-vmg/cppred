#include "stdafx.h"
#include "Console.h"
#include "Renderer.h"
#include "Engine.h"
#include "CppRed/AudioProgram.h"
#include "../CodeGeneration/output/audio.h"
#include "font.inl"
#include "Coroutine.h"
#include "HighResolutionClock.h"
#ifndef HAVE_PCH
#include <sstream>
#include <iomanip>
#include <cassert>
#endif

#ifdef RGB
#undef RGB
#endif

Console *global_console = nullptr;

static int console_text_scale = 2;
static int log_text_scale = 1;

Console::Console(Engine &engine):
		engine(&engine),
		device(&engine.get_renderer().get_device()),
		console(this->device->get_screen_size() * (1.0 / 8.0 / console_text_scale), console_text_scale),
		log(this->device->get_screen_size() * (1.0 / 8.0 / log_text_scale) + Point(0, 1), log_text_scale),
		visible(false){
	auto &dev = *this->device;
	auto size = dev.get_screen_size();
	this->background = dev.allocate_texture(size);
	this->text_layer = dev.allocate_texture(size);
	this->log_layer = dev.allocate_texture(size);

	this->initialize_background(0x80);
	this->initialize_text_layer(this->text_layer);
	this->initialize_text_layer(this->log_layer);
	this->coroutine.reset(new Coroutine("Console coroutine", engine.get_stepping_clock(), [this](Coroutine &){
		this->coroutine_entry_point();
	}));
}

CharacterMatrix::CharacterMatrix(const Point &size, int scale){
	this->text_scale = scale;
	this->matrix_size = size;
	this->character_matrix.resize(this->matrix_size.x * this->matrix_size.y);
	this->last_character_matrix.resize(this->character_matrix.size());
}

void CharacterMatrix::set_shift(const Point &shift){
	this->shift.x = euclidean_modulo(shift.x, this->matrix_size.x);
	this->shift.y = euclidean_modulo(shift.y, this->matrix_size.y);
	this->matrix_modified = this->matrix_modified || this->shift != this->last_shift;
}

void Console::initialize_background(byte_t background_alpha){
	auto surf = this->background.lock();

	auto pixels = surf.get_row(0);
	auto w = surf.get_size().x;
	auto h = surf.get_size().y;
	for (int y = 0; y < h; y++){
		auto row = pixels + y * w;
		//byte_t c = 0xFF - 0xFF * y / (h - 1);
		byte_t c = 0;
		for (int x = 0; x < w; x++){
			row[x].r = c;
			row[x].g = c;
			row[x].b = c;
			row[x].a = background_alpha;
		}
	}
}

void Console::blank_texture(SDL_Texture *texture, int width, int height){
	void *void_pixels;
	int pitch;

	if (SDL_LockTexture(texture, nullptr, &void_pixels, &pitch) < 0)
		throw std::runtime_error("Console::Console(): Failed to initialize texture.");

	assert(pitch == width * sizeof(RGB));
	memset(void_pixels, 0, height * pitch);
	SDL_UnlockTexture(texture);
}

void Console::initialize_text_layer(Texture &texture){
	auto surf = texture.lock();
	memset(surf.get_row(0), 0, surf.get_size().multiply_components() * sizeof(RGB));
}

bool Console::handle_event(const SDL_Event &event){
	if (!this->visible)
		return false;

	switch (event.type){
		case SDL_QUIT:
			return false;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym){
				case SDLK_DOWN:
					this->current_menu_position++;
					return true;
				case SDLK_UP:
					this->current_menu_position--;
					return true;
				case SDLK_PAGEDOWN:
					this->current_menu_position += 10;
					return true;
				case SDLK_PAGEUP:
					this->current_menu_position -= 10;
					return true;
				case SDLK_HOME:
					this->current_menu_position = 0;
					return true;
				case SDLK_END:
					this->current_menu_position = -1;
					return true;
				default:
					break;
			}
			if (!event.key.repeat){
				switch (event.key.keysym.sym){
					case SDLK_ESCAPE:
						this->toggle_visible();
						break;
					case SDLK_DOWN:
						this->current_menu_position++;
						break;
					case SDLK_UP:
						this->current_menu_position--;
						break;
					case SDLK_RETURN:
						this->selected = true;
						break;
					default:
						break;
				}
			}
			break;
		default:
			break;
	}
	return true;
}

void Console::render(){
	if (!this->visible && !this->log_enabled)
		return;

	this->device->render_copy(this->background);
	if (this->visible){
		this->draw_console_menu();
		this->device->render_copy(this->text_layer);
	}else{
		this->draw_console_log();
		this->device->render_copy(this->log_layer);
	}

}

bool CharacterMatrix::needs_redraw(){
	if (!this->matrix_modified)
		return false;
	if (!memcmp(&this->character_matrix[0], &this->last_character_matrix[0], this->character_matrix.size()) && this->shift == this->last_shift){
		this->matrix_modified = false;
		return false;
	}
	return true;
}

void CharacterMatrix::draw(RGB *pixels, int w, int h){
	const RGB white = {255, 255, 255, 255},
		black = {0, 0, 0, 255},
		off = {0, 0, 0, 0};
	auto matrix = &this->character_matrix[0];
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			auto x0 = x / (8 * text_scale);
			auto y0 = y / (8 * text_scale);
			x0 = euclidean_modulo(x0 - this->shift.x, this->matrix_size.x);
			y0 = euclidean_modulo(y0 - this->shift.y, this->matrix_size.y);
			auto c = matrix[x0 + y0 * this->matrix_size.x];
			if (!c){
				pixels[x + y * w] = off;
				continue;
			}
			int first_font_index = c * 8;
			pixels[x + y * w] = !!(GFX_font[first_font_index + (y / text_scale) % 8] & (0x80 >> ((x / text_scale) % 8))) ? white : off;
		}
	}
	for (int y = 0; y < h; y++){
		int y0 = std::max(y - text_scale, 0);
		int y1 = std::min(y + text_scale * 2, h);
		for (int x = 0; x < w; x++){
			if (!!pixels[x + y * w].a)
				continue;
			int x0 = std::max(x - text_scale, 0);
			int x1 = std::min(x + text_scale * 2, w);
			bool any = false;
			for (int y2 = y0; !any && y2 < y1; y2 += text_scale)
				for (int x2 = x0; !any && x2 < x1; x2 += text_scale)
					any = !!(((x2 != x) | (y2 != y)) & !!pixels[x2 + y2 * w].r);

			if (any)
				pixels[x + y * w] = black;
		}
	}

	memcpy(&this->last_character_matrix[0], &this->character_matrix[0], this->character_matrix.size());
	this->last_shift = shift;
	this->matrix_modified = false;
}

void Console::draw(Texture &dst, CharacterMatrix &src){
	if (!src.needs_redraw())
		return;

	auto w = dst.get_size().x;
	auto h = dst.get_size().y;
	TextureSurface surf;
	if (dst.try_lock(surf)){
		auto pixels = (RGB *)surf.get_row(0);
		src.draw(pixels, w, h);
	}
}

void Console::draw_console_menu(){
	this->draw(this->text_layer, this->console);
}

void Console::draw_console_log(){
	LOCK_MUTEX(this->log_mutex);
	this->draw(this->log_layer, this->log);
}

void CharacterMatrix::write_character(int x, int y, byte_t character){
	auto x0 = euclidean_modulo(x - this->shift.x, this->matrix_size.x);
	auto y0 = euclidean_modulo(y - this->shift.y, this->matrix_size.y);
	this->character_matrix[x0 + y0 * this->matrix_size.x] = character;
	this->matrix_modified = true;
}

void CharacterMatrix::write_string(int x, int y, const char *string){
	for (; *string; string++, x++)
		this->write_character(x, y, *string);
}

void CharacterMatrix::write_string2(const char *string){
	int x = 0;
	int y = this->matrix_size.y - 1;
	for (; *string; string++){
		if (*string == '\t'){
			x = x + 4 - x % 4;
			continue;
		}
		if (x == this->matrix_size.x || *string == '\n'){
			x = 0;
			this->shift.y--;
			for (int i = 0; i < this->matrix_size.x; i++)
				this->write_character(i, this->matrix_size.y - 1, 0);
			continue;
		}
		this->write_character(x, y, *string);
		x++;
	}
}

ConsoleCommunicationChannel *Console::update(){
	if (!this->visible)
		return nullptr;

	this->coroutine->get_clock().step();
	this->coroutine->resume();
	return this->ccc;
}

void Console::yield(){
	this->coroutine->yield();
}

void Console::yield(ConsoleCommunicationChannel &ccc){
	this->ccc = &ccc;
	this->yield();
	this->ccc = nullptr;
}

CppRed::AudioProgramInterface &Console::get_audio_program(){
	ConsoleCommunicationChannel ccc;
	ccc.request_id = ConsoleRequestId::GetAudioProgram;
	this->yield(ccc);
	return *ccc.audio_program;
}

void CharacterMatrix::clear(){
	std::fill(this->character_matrix.begin(), this->character_matrix.end(), 0);
}

void Console::draw_long_menu(const std::vector<std::string> &strings, int item_separation){
	auto h = this->text_layer.get_size().y;
	const auto max_visible = h / (8 * item_separation * this->console.get_text_scale()) - 2;
	auto first_visible_item = std::max(this->current_menu_position - max_visible / 2, 0);
	if (strings.size() - first_visible_item < max_visible)
		first_visible_item = (int)strings.size() - max_visible;
	this->console.clear();
	for (int i = 0; i < max_visible; i++){
		this->console.write_string(3, 1 + i * item_separation, strings[i + first_visible_item].c_str());
		if (i + first_visible_item == this->current_menu_position)
			this->console.write_character(1, 1 + i * item_separation, 0x10);
	}
}

int Console::handle_menu(const std::vector<std::string> &strings, int default_item, int item_separation){
	if (strings.size() > std::numeric_limits<int>::max())
		throw std::exception();

	auto previous_menu_position = default_item;
	int i = 1;
	auto h = this->text_layer.get_size().y;
	const auto max_visible = h / (8 * item_separation * this->console.get_text_scale()) - 2;
	this->current_menu_position = default_item;
	this->current_menu_size = (int)strings.size();
	if (strings.size() <= max_visible){
		for (auto &s : strings){
			this->console.write_string(3, i, s.c_str());
			i += item_separation;
		}
		this->console.write_character(1, 1 + previous_menu_position * item_separation, 0x10);
	}else
		this->draw_long_menu(strings, item_separation);

	this->selected = false;
	while (true){
		do{
			this->yield();
			this->current_menu_position = euclidean_modulo(this->current_menu_position, this->current_menu_size);
		}while (!this->selected && this->current_menu_position == previous_menu_position);
		if (this->selected){
			this->console.clear();
			return this->current_menu_position;
		}
		if (strings.size() <= max_visible){
			this->console.write_character(1, 1 + previous_menu_position * item_separation, 0);
			previous_menu_position = this->current_menu_position;
			this->console.write_character(1, 1 + previous_menu_position * item_separation, 0x10);
		}else{
			this->draw_long_menu(strings, item_separation);
			previous_menu_position = this->current_menu_position;
		}
	}
}

static const char *to_string(PokemonVersion version){
	switch (version){
		case PokemonVersion::Red:
			return "Red";
		case PokemonVersion::Blue:
			return "Blue";
		default:
			return "?";
	}
}

void Console::coroutine_entry_point(){
	int item = 0;
	while (true){
		std::vector<std::string> main_menu;
		main_menu.push_back("Restart");
		main_menu.push_back((std::string)"Version: " + to_string(this->get_version()));
		main_menu.push_back((std::string)"Enable console: " + (this->log_enabled ? "ON" : "OFF"));
		main_menu.push_back("Sound test");
		main_menu.push_back("Pok\x82mon cries");

		bool run = true;
		while (run){
			auto selection = this->handle_menu(main_menu, item);
			item = selection;
			switch (selection){
				case 0:
					this->restart_game();
					this->visible = false;
					break;
				case 1:
					this->flip_version();
					run = false;
					break;
				case 2:
					this->log_enabled = !this->log_enabled;
					run = false;
					break;
				case 3:
					this->sound_test();
					break;
				case 4:
					this->cry_test();
					break;
			}
		}
	}
}

void Console::sound_test(){
	this->engine->go_to_debug();
	auto &program = this->get_audio_program();
	CppRed::AudioInterface audio_interface(program);
	auto sound_strings = program.get_resource_strings();
	sound_strings.erase(sound_strings.begin());
	std::vector<int> indices;
	{
		std::vector<std::pair<std::string, int>> temp;
		indices.reserve(sound_strings.size());
		temp.reserve(sound_strings.size());
		int i = 1;
		for (auto &s : sound_strings)
			temp.emplace_back(s, i++);
		std::sort(temp.begin(), temp.end());
		sound_strings.clear();
		for (auto &pair : temp){
			sound_strings.push_back(pair.first);
			indices.push_back(pair.second);
		}

		sound_strings.push_back("Stop");
		indices.push_back((int)indices.size() + 1);
	}
	int item = 0;
	while (true){
		item = this->handle_menu(sound_strings, item);
		audio_interface.play_sound((AudioResourceId)indices[item]);
	}
}

void Console::cry_test(){
	this->engine->go_to_debug();
	auto &program = this->get_audio_program();
	CppRed::AudioInterface audio_interface(program);
	std::vector<std::string> cries;
	cries.reserve(array_length(pokemon_by_pokedex_id));
	for (size_t i = 0; i < cries.capacity(); i++){
		std::stringstream stream;
		stream << std::setw(3) << std::setfill('0') << i + 1 << " " << pokemon_by_pokedex_id[i]->internal_name;
		cries.push_back(stream.str());
	}
	int item = 0;
	audio_interface.play_sound(AudioResourceId::Music_Routes1);
	while (true){
		item = this->handle_menu(cries, item);
		audio_interface.play_cry(pokemon_by_pokedex_id[item]->species_id);
	}
}

void Console::restart_game(){
	ConsoleCommunicationChannel ccc;
	ccc.request_id = ConsoleRequestId::Restart;
	this->yield(ccc);
}

void Console::flip_version(){
	ConsoleCommunicationChannel ccc;
	ccc.request_id = ConsoleRequestId::FlipVersion;
	this->yield(ccc);
}

PokemonVersion Console::get_version(){
	ConsoleCommunicationChannel ccc;
	ccc.request_id = ConsoleRequestId::GetVersion;
	this->yield(ccc);
	return ccc.version;
}

void Console::log_string(const std::string &s){
	LOCK_MUTEX(this->log_mutex);
	this->log.write_string2(s.c_str());
}
