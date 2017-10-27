#include "CppRedEngine.h"
#include "Engine.h"
#include "Renderer.h"
#include <iostream>

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

CppRedEngine::CppRedEngine(Engine &engine, PokemonVersion version): engine(&engine), version(version){
	this->engine->set_on_yield([this](){ this->update_joypad_state(); });
	this->reset_dialog_state();
}

void CppRedEngine::clear_screen(){
	this->engine->get_renderer().clear_screen();
	this->engine->wait_frames(3);
}

void CppRedEngine::play_sound(AudioResourceId sound){
	this->engine->play_sound(sound);
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

std::string CppRedEngine::get_name_from_user(NameEntryType type, SpeciesId species, int max_length_){
	if (!max_length_)
		return "";
	size_t max_display_length = type == NameEntryType::Pokemon ? 10 : 7;
	size_t max_length;
	if (max_length_ < 0)
		max_length = max_display_length;
	else
		max_length = max_length_;

	auto &renderer = this->engine->get_renderer();
	renderer.clear_screen();
	std::string query_string;
	query_string.reserve(Tilemap::w);
	switch (type){
		case NameEntryType::Player:
			query_string = "YOUR NAME?";
			break;
		case NameEntryType::Rival:
			query_string = "RIVAL";
			query_string += make_apostrophe('s');
			query_string += " NAME?";
			break;
		case NameEntryType::Pokemon:
			query_string = "    ";
			query_string += pokemon_by_species_id[(int)species]->display_name;
			break;
		default:
			throw std::runtime_error("CppRedEngine::get_name_from_user(): Invalid switch.");
	}
	this->put_string({ 0, 1 }, TileRegion::Background, query_string.c_str());
	if (type == NameEntryType::Pokemon)
		this->put_string({ 1, 3 }, TileRegion::Background, "NICKNAME?");

	Point box_location = { 0, 4 };
	const Point box_size = { 18, 9 };
	this->draw_box(box_location, box_size, TileRegion::Background);
	const char *selection_sheet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ *():;[]{}-?!%+/.,\xFF";
	const char *mode_select_lower = "lower case";
	const char *mode_select_upper = "UPPER CASE";
	const Point mode_select_cursor_location = box_location + Point{ 1, box_size.y + 2 };
	const Point mode_select_location = mode_select_cursor_location + Point{ 1, 0 };
	box_location += Point{ 1, 1 };
	bool redraw_alphabet = true;
	bool redraw_name = true;
	bool lower_case = false;
	auto tilemap = renderer.get_tilemap(TileRegion::Background).tiles;
	Point cursor_position = { 0, 0 };
	const int grid_w = 9;
	const int grid_h = 5;
	std::string ret;
	while (true){
		if (redraw_alphabet){
			for (int y = 0; y < grid_h; y++){
				auto dst_y = box_location.y + y * 2;
				for (int x = 0; x < grid_w; x++){
					auto src = (byte_t)selection_sheet[x + y * grid_w];
					auto dst_x = box_location.x + x * 2 + 1;
					if (lower_case)
						src = (byte_t)tolower(src);
					tilemap[dst_x + dst_y * Tilemap::w].tile_no = src;
				}
			}
			this->put_string(mode_select_location, TileRegion::Background, lower_case ? mode_select_upper : mode_select_lower);
			redraw_alphabet = false;
		}

		if (redraw_name){
			auto name_location = tilemap + 10 + 2 * Tilemap::w;
			auto dash_location = name_location + Tilemap::w;
			auto first_index = ret.size() > max_display_length ? ret.size() - max_display_length : 0;
			auto text_cursor_position = std::min(ret.size(), max_display_length - 1);
			auto low_dash = HpBarAndStatusGraphics.first_tile + HpBarAndStatusGraphics.width * 4;
			for (size_t i = 0; i < max_display_length; i++){
				name_location[i].tile_no = first_index + i < ret.size() ? (byte_t)ret[first_index + i] : ' ';
				dash_location[i].tile_no = low_dash + (i == text_cursor_position);
			}
			redraw_name = false;
		}

		for (int y = 0; y < grid_h; y++){
			auto dst_y = box_location.y + y * 2;
			for (int x = 0; x < grid_w; x++){
				auto dst_x = box_location.x + x * 2;
				tilemap[dst_x + dst_y * Tilemap::w].tile_no = ((x == cursor_position.x) & (y == cursor_position.y)) ? black_arrow : ' ';
			}
		}
		tilemap[mode_select_cursor_location.x + mode_select_cursor_location.y * Tilemap::w].tile_no = cursor_position.y == grid_h ? black_arrow : ' ';

		if (type == NameEntryType::Pokemon){
			//TODO: Draw animated pokemon sprite.
		}

		while (true){
			this->engine->wait_exactly_one_frame();
			auto input = this->joypad_only_newly_pressed();
			if (input.get_up()){
				cursor_position.y = (cursor_position.y + grid_h) % (grid_h + 1);
				break;
			}
			if (input.get_down()){
				cursor_position.y = (cursor_position.y + 1) % (grid_h + 1);
				break;
			}
			if (input.get_left() && cursor_position.y != grid_h){
				cursor_position.x = (cursor_position.x + (grid_w - 1)) % grid_w;
				break;
			}
			if (input.get_right() && cursor_position.y != grid_h){
				cursor_position.x = (cursor_position.x + 1) % grid_w;
				break;
			}
			if (input.get_select()){
				lower_case = !lower_case;
				redraw_alphabet = true;
				break;
			}
			if (input.get_start()){
				return ret;
			}
			if (input.get_a()){
				if (cursor_position.y == grid_h){
					lower_case = !lower_case;
					redraw_alphabet = true;
					break;
				}
				auto character = (byte_t)selection_sheet[cursor_position.x + cursor_position.y * grid_w];
				if (character == 0xFF)
					return ret;
				if (ret.size() >= max_length)
					continue;
				if (lower_case)
					character = (byte_t)tolower(character);
				ret.push_back((char)character);
				if (ret.size() >= max_length){
					cursor_position.x = grid_w - 1;
					cursor_position.y = grid_h - 1;
				}
				redraw_name = true;
				break;
			}
			if (input.get_b()){
				if (!ret.size())
					continue;
				ret.pop_back();
				redraw_name = true;
				break;
			}
		}
	}
}

std::string CppRedEngine::get_name_from_user(NameEntryType type, int max_length){
	if (type == NameEntryType::Pokemon)
		throw std::runtime_error("CppRedEngine::get_name_from_user(): Invalid usage. type must not be NameEntryType::Pokemon.");
	auto ret = this->get_name_from_user(type, SpeciesId::None, max_length);
	std::cout << "Selected name: " << ret << std::endl;
	return ret;
}

std::string CppRedEngine::get_name_from_user(SpeciesId species, int max_length){
	auto ret = this->get_name_from_user(NameEntryType::Pokemon, SpeciesId::None, max_length);
	std::cout << "Selected name: " << ret << std::endl;
	return ret;
}
