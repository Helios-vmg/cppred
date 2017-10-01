#include "CppRedEngine.h"
#include "Engine.h"
#include "Renderer.h"

struct FadePaletteData{
	Palette background_palette;
	Palette obp0_palette;
	Palette obp1_palette;
};

//Note: Palettes 3 and 4 are identical and equal to the default palette. Palettes >= 4 are used for fade-outs
//      to white, while palettes <= 3 are used for fade-outs to black.
const FadePaletteData fade_palettes[8] = {
	{ BITMAP(11111111), BITMAP(11111111), BITMAP(11111111) },
	{ BITMAP(11111110), BITMAP(11111110), BITMAP(11111000) },
	{ BITMAP(11111001), BITMAP(11100100), BITMAP(11100100) },
	{ BITMAP(11100100), BITMAP(11010000), BITMAP(11100000) },
	{ BITMAP(11100100), BITMAP(11010000), BITMAP(11100000) },
	{ BITMAP(10010000), BITMAP(10000000), BITMAP(10010000) },
	{ BITMAP(01000000), BITMAP(01000000), BITMAP(01000000) },
	{ BITMAP(00000000), BITMAP(00000000), BITMAP(00000000) },
};

CppRedEngine::CppRedEngine(Engine &engine): engine(&engine){
	this->engine->set_on_yield([this](){ this->update_joypad_state(); });
	this->reset_dialog_state();
}

void CppRedEngine::clear_screen(){
	this->engine->get_renderer().clear_screen();
	this->engine->wait_frames(3);
}

void CppRedEngine::play_sound(SoundId sound){
	//TODO
}

void CppRedEngine::play_cry(SpeciesId){
	//TODO
}

void CppRedEngine::fade_out_to_white(){
	auto &engine = *this->engine;
	auto &renderer = engine.get_renderer();
	for (int i = 0; i < 3; i++){
		auto &palette = fade_palettes[5 + i];
		renderer.set_palette(PaletteRegion::Background, palette.background_palette);
		renderer.set_palette(PaletteRegion::Sprites0, palette.obp0_palette);
		renderer.set_palette(PaletteRegion::Sprites1, palette.obp1_palette);
		engine.wait_frames(8);
	}
}

void CppRedEngine::palette_whiteout(){
	auto &renderer = this->engine->get_renderer();
	renderer.clear_subpalettes(SubPaletteRegion::All);
	renderer.set_palette(PaletteRegion::Background, zero_palette);
	renderer.set_palette(PaletteRegion::Sprites0, zero_palette);
	renderer.set_palette(PaletteRegion::Sprites1, zero_palette);
}

bool CppRedEngine::check_for_user_interruption_internal(bool autorepeat, double timeout, InputState *input_state){
	timeout += this->engine->get_clock();
	do{
		this->engine->wait_exactly_one_frame();
		auto input = autorepeat ? this->joypad_auto_repeat() : this->joypad_only_newly_pressed();
		auto held = this->joypad_held;
		const auto mask = InputState::mask_up | InputState::mask_select | InputState::mask_b;
		if (held.get_value() == mask){
			if (input_state)
				*input_state = held;
			return true;
		}
		if (input.get_a() || input.get_start()){
			if (input_state)
				*input_state = input;
			return true;
		}
	}while (this->engine->get_clock() < timeout);
	return false;
}

bool CppRedEngine::check_for_user_interruption_no_auto_repeat(double timeout, InputState *input_state){
	return this->check_for_user_interruption_internal(false, timeout, input_state);
}

bool CppRedEngine::check_for_user_interruption(double timeout, InputState *input_state){
	return this->check_for_user_interruption_internal(true, timeout, input_state);
}

InputState CppRedEngine::joypad_only_newly_pressed(){
	return this->joypad_pressed;
}

void CppRedEngine::update_joypad_state(){
	auto old = this->joypad_held;
	this->joypad_held = this->engine->get_input_state();
	this->joypad_pressed = this->joypad_held & ~old;
}

InputState CppRedEngine::joypad_auto_repeat(){
	auto held = this->joypad_held;

	auto pressed = this->joypad_pressed;
	if (pressed.get_value()){
		this->jls_timeout = this->engine->get_clock() + 0.5;
		return held;
	}
	if (this->engine->get_clock() < this->jls_timeout)
		return InputState();
	if (held.get_value())
		this->jls_timeout = this->engine->get_clock() + 5.0/60.0;
	else
		this->jls_timeout = std::numeric_limits<double>::max();
	
	return held;
}

void CppRedEngine::wait_for_sound_to_finish(){
	//TODO
}

CppRedEngine::load_save_t CppRedEngine::load_save(){
	//TODO
	return nullptr;
}

void CppRedEngine::draw_box(const Point &corner, const Point &size, TileRegion region){
	if (corner.x < 0 || corner.y < 0)
		throw std::runtime_error("CppRedEngine::handle_standard_menu(): invalid position.");
	if (size.x > Renderer::logical_screen_tile_width - 2 || size.y > Renderer::logical_screen_tile_height - 2)
		throw std::runtime_error("CppRedEngine::handle_standard_menu(): invalid dimensions.");
	const std::uint16_t w = TextBoxGraphics.width;
	const std::uint16_t f = TextBoxGraphics.first_tile + 1 + 6 * w;
	const std::uint16_t tiles[] = {
		(std::uint16_t)(f + 0),
		(std::uint16_t)(f + 1),
		(std::uint16_t)(f + 2),
		(std::uint16_t)(f + 3),
		(std::uint16_t)' ',
		(std::uint16_t)(f + 3),
		(std::uint16_t)(f + 4),
		(std::uint16_t)(f + 1),
		(std::uint16_t)(f + 5),
	};

	auto dst = this->engine->get_renderer().get_tilemap(region).tiles + corner.x + corner.y * Tilemap::w;
	dst[0].tile_no = tiles[0];
	for (int i = 0; i < size.x; i++)
		dst[1 + i].tile_no = tiles[1];
	dst[1 + size.x].tile_no = tiles[2];
	for (int y = 0; y < size.y; y++){
		dst += Tilemap::w;
		dst[0].tile_no = tiles[3];
		for (int x = 0; x < size.x; x++)
			dst[1 + x].tile_no = tiles[4];
		dst[1 + size.x].tile_no = tiles[5];
	}
	dst += Tilemap::w;
	dst[0].tile_no = tiles[6];
	for (int i = 0; i < size.x; i++)
		dst[1 + i].tile_no = tiles[7];
	dst[1 + size.x].tile_no = tiles[8];
}

void CppRedEngine::put_string(const Point &position, TileRegion region, const char *string){
	int i = position.x + position.y * Tilemap::w;
	auto tilemap = this->engine->get_renderer().get_tilemap(region).tiles;
	for (; *string; string++){
		tilemap[i].tile_no = (byte_t)*string;
		tilemap[i].flipped_x = false;
		tilemap[i].flipped_y = false;
		i = (i + 1) % Tilemap::size;
	}
}

int CppRedEngine::handle_standard_menu_with_title(
		TileRegion region,
		const Point &position_,
		const std::vector<std::string> &items,
		const char *title,
		const Point &minimum_size,
		bool ignore_b){

	auto position = position_;
	auto width = minimum_size.x;
	auto n = (int)items.size();
	for (auto &s : items)
		width = std::max((int)s.size() + 1, width);
	if (position.x + width + 2 > Renderer::logical_screen_tile_width)
		position.x = Renderer::logical_screen_tile_width - (width + 2);
	if (position.y + n * 2 + 2 > Renderer::logical_screen_tile_height)
		position.y = Renderer::logical_screen_tile_height - (n * 2 + 2);

	this->draw_box(position, { width, std::max(n * 2, minimum_size.y) }, region);
	if (title){
		auto l = (int)strlen(title);
		this->put_string(position + Point{ (width - l) / 2 + 1, 0 }, region, title);
	}

	int y = 1;
	for (auto &s : items)
		this->put_string(position + Point{ 2, y++ * 2 }, region, s.c_str());

	int current_item = 0;
	auto tilemap = this->engine->get_renderer().get_tilemap(region).tiles;
	while (true){
		auto index = position.x + 1 + (position.y + (current_item + 1) * 2) * Tilemap::w;
		tilemap[index].tile_no = black_arrow;
		int addend = 0;
		do{
			this->engine->wait_exactly_one_frame();
			auto state = this->joypad_auto_repeat();
			if (!ignore_b && state.get_b())
				return -1;
			if (state.get_a())
				return current_item;
			if (!(state.get_down() || state.get_up()))
				continue;
			addend = state.get_down() ? 1 : -1;
		}while (!addend);
		tilemap[index].tile_no = ' ';
		current_item = (current_item + n + addend) % n;
	}
	return -1;
}

int CppRedEngine::handle_standard_menu(TileRegion region, const Point &position, const std::vector<std::string> &items, const Point &minimum_size, bool ignore_b){
	return this->handle_standard_menu_with_title(region, position, items, nullptr, minimum_size, ignore_b);
}

void CppRedEngine::run_dialog(TextResourceId resource){
	if (!this->dialog_box_visible){
		this->draw_box(this->text_state.box_corner - Point{ 1, 1 }, this->text_state.box_size, TileRegion::Background);
		this->dialog_box_visible = true;
	}
	this->text_store.execute(*this, resource, this->text_state);
}

void CppRedEngine::reset_dialog_state(){
	this->text_state.region = TileRegion::Background;
	this->text_state.first_position =
		this->text_state.position =
		this->text_state.start_of_line = { 1, Renderer::logical_screen_tile_height - 4 };
	this->text_state.box_corner = { 1, Renderer::logical_screen_tile_height - 5 };
	this->text_state.box_size = { Renderer::logical_screen_tile_width - 2, 4 };
	this->text_state.continue_location = { 18, 16 };
	this->dialog_box_visible = false;
}

void CppRedEngine::text_print_delay(){
	if (!this->engine->get_input_state().get_a())
		this->engine->wait_frames((int)this->options.text_speed);
}

void VariableStore::set_string(const std::string &key, std::string *value){
	this->string_variables[key] = value;
}

void VariableStore::set_string(const std::string &key, const std::string &value){
	this->strings.push_back(value);
	this->set_string(key, &this->strings.back());
}

void VariableStore::set_number(const std::string &key, int *value){
	this->number_variables[key] = value;
}

void VariableStore::set_number(const std::string &key, int value){
	this->numbers.push_back(value);
	this->set_number(key, &this->numbers.back());
}

const std::string &VariableStore::get_string(const std::string &key){
	auto it = this->string_variables.find(key);
	if (it == this->string_variables.end())
		throw std::runtime_error("Variable not found: " + key);
	return *it->second;
}

int VariableStore::get_number(const std::string &key){
	auto it = this->number_variables.find(key);
	if (it == this->number_variables.end())
		throw std::runtime_error("Variable not found: " + key);
	return *it->second;
}

void VariableStore::delete_string(const std::string &key){
	auto it = this->string_variables.find(key);
	if (it == this->string_variables.end())
		return;
	this->string_variables.erase(it);
}

void VariableStore::delete_number(const std::string &key){
	auto it = this->number_variables.find(key);
	if (it == this->number_variables.end())
		return;
	this->number_variables.erase(it);
}
