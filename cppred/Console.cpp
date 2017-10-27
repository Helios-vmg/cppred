#include "Console.h"
#include "Renderer.h"
#include "Engine.h"
#include "../CodeGeneration/output/audio.h"
#include "font.inl"

#ifdef RGB
#undef RGB
#endif

static const auto w = Renderer::logical_screen_width * Engine::screen_scale;
static const auto h = Renderer::logical_screen_height * Engine::screen_scale;
static const auto matrix_w = w / 8;
static const auto matrix_h = h / 8;
static int text_scale = 2;

Console::Console(Engine &engine):
		engine(&engine),
		renderer(&engine.get_renderer()),
		visible(false),
		background(nullptr, SDL_DestroyTexture),
		text_layer(nullptr, SDL_DestroyTexture){
	this->background.reset(this->renderer->request_texture(w, h));
	this->text_layer.reset(this->renderer->request_texture(w, h));
	this->character_matrix.resize(matrix_w * matrix_h);
	this->last_character_matrix.resize(this->character_matrix.size());

	this->initialize_background(0x80);
	this->initialize_text_layer();
	this->coroutine.reset(new coroutine_t(
		[this](yielder_t &y){
			this->yielder = &y;
			this->yield();
			this->coroutine_entry_point();
		}
	));
}

void Console::initialize_background(byte_t background_alpha){
	void *void_pixels;
	int pitch;

	if (SDL_LockTexture(this->background.get(), nullptr, &void_pixels, &pitch) < 0)
		throw std::runtime_error("Console::initialize_background(): Failed to initialize texture.");

	assert(pitch == w * sizeof(RGB));
	auto pixels = (RGB *)void_pixels;
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
	SDL_UnlockTexture(this->background.get());
	SDL_SetTextureBlendMode(this->background.get(), SDL_BLENDMODE_BLEND);
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

void Console::initialize_text_layer(){
	this->blank_texture(this->text_layer.get(), w, h);
	SDL_SetTextureBlendMode(this->text_layer.get(), SDL_BLENDMODE_BLEND);
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
	if (!this->visible)
		return;

	if (this->matrix_modified && memcmp(&this->character_matrix[0], &this->last_character_matrix[0], this->character_matrix.size())){
		void *void_pixels;
		int pitch;
		if (SDL_LockTexture(this->text_layer.get(), nullptr, &void_pixels, &pitch) >= 0){
			const RGB white = { 255, 255, 255, 255 },
				black = { 0, 0, 0, 255 },
				off = { 0, 0, 0, 0 };
			auto matrix = &this->character_matrix[0];
			auto pixels = (RGB *)void_pixels;
			for (int y = 0; y < h; y++){
				for (int x = 0; x < w; x++){
					auto c = matrix[x / (8 * text_scale) + y / (8 * text_scale) * matrix_w];
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
			SDL_UnlockTexture(this->text_layer.get());
		}

		memcpy(&this->last_character_matrix[0], &this->character_matrix[0], this->character_matrix.size());
		this->matrix_modified = false;
	}

	SDL_RenderCopy(this->renderer->get_renderer(), this->background.get(), nullptr, nullptr);
	SDL_RenderCopy(this->renderer->get_renderer(), this->text_layer.get(), nullptr, nullptr);
}

void Console::write_character(int x, int y, byte_t character){
	this->character_matrix[euclidean_modulo(x + y * matrix_w, matrix_w * matrix_h)] = character;
	this->matrix_modified = true;
}

void Console::write_string(int x, int y, const char *string){
	for (; *string; string++, x++)
		this->write_character(x, y, *string);
}

void Console::update(){
	if (!this->visible)
		return;

	(*this->coroutine)();
}

void Console::yield(){
	(*this->yielder)();
}

void Console::draw_long_menu(const std::vector<std::string> &strings, int item_separation){
	const auto max_visible = h / (8 * item_separation * text_scale) - 2;
	auto first_visible_item = std::max(this->current_menu_position - max_visible / 2, 0);
	if (strings.size() - first_visible_item < max_visible)
		first_visible_item = (int)strings.size() - max_visible;
	std::fill(this->character_matrix.begin(), this->character_matrix.end(), 0);
	for (int i = 0; i < max_visible; i++){
		this->write_string(3, 1 + i * item_separation, strings[i + first_visible_item].c_str());
		if (i + first_visible_item == this->current_menu_position)
			this->write_character(1, 1 + i * item_separation, 0x10);
	}
}

int Console::handle_menu(const std::vector<std::string> &strings, int default_item, int item_separation){
	if (strings.size() > std::numeric_limits<int>::max())
		throw std::exception();

	auto previous_menu_position = default_item;
	int i = 1;
	const auto max_visible = h / (8 * item_separation * text_scale) - 2;
	this->current_menu_position = default_item;
	this->current_menu_size = (int)strings.size();
	if (strings.size() <= max_visible){
		for (auto &s : strings){
			this->write_string(3, i, s.c_str());
			i += item_separation;
		}
		this->write_character(1, 1 + previous_menu_position * item_separation, 0x10);
	}else
		this->draw_long_menu(strings, item_separation);

	this->selected = false;
	while (true){
		do{
			this->yield();
			this->current_menu_position = euclidean_modulo(this->current_menu_position, this->current_menu_size);
		}while (!this->selected && this->current_menu_position == previous_menu_position);
		if (this->selected){
			std::fill(this->character_matrix.begin(), this->character_matrix.end(), 0);
			return this->current_menu_position;
		}
		if (strings.size() <= max_visible){
			this->write_character(1, 1 + previous_menu_position * item_separation, 0);
			previous_menu_position = this->current_menu_position;
			this->write_character(1, 1 + previous_menu_position * item_separation, 0x10);
		}else{
			this->draw_long_menu(strings, item_separation);
			previous_menu_position = this->current_menu_position;
		}
	}
}

void Console::coroutine_entry_point(){
	std::vector<std::string> main_menu = {
		"Sound test",
	};

	while (true){
		auto selection = this->handle_menu(main_menu);

		switch (selection){
			case 0:
				this->sound_test();
				break;
		}
	}
}

void Console::sound_test(){
	this->engine->go_to_debug();
	auto sounds = this->engine->get_audio().get_program().get_resource_strings();
	sounds.erase(sounds.begin());
	sounds.push_back("Stop");
	int item = 0;
	while (true){
		item = this->handle_menu(sounds, item);
		this->engine->play_sound((AudioResourceId)(item + 1));
	}
}
