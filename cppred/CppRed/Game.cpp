#include "Game.h"
#include "Engine.h"
#include "Renderer.h"
#include "PlayerCharacter.h"
#include "Maps.h"
#include "Data.h"
#include "../CodeGeneration/output/audio.h"
#include <iostream>

namespace CppRed{

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

Game::Game(Engine &engine, PokemonVersion version, CppRed::AudioProgram &program):
		engine(&engine),
		version(version),
		audio_interface(program){
	this->engine->set_on_yield([this](){ this->update_joypad_state(); });
	this->reset_dialog_state();
}

Game::~Game(){}

void Game::clear_screen(){
	this->engine->get_renderer().clear_screen();
	this->engine->wait_frames(3);
}

void Game::fade_out_to_white(){
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

void Game::palette_whiteout(){
	auto &renderer = this->engine->get_renderer();
	renderer.clear_subpalettes(SubPaletteRegion::All);
	renderer.set_palette(PaletteRegion::Background, zero_palette);
	renderer.set_palette(PaletteRegion::Sprites0, zero_palette);
	renderer.set_palette(PaletteRegion::Sprites1, zero_palette);
}

bool Game::check_for_user_interruption_internal(bool autorepeat, double timeout, InputState *input_state){
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

bool Game::check_for_user_interruption_no_auto_repeat(double timeout, InputState *input_state){
	return this->check_for_user_interruption_internal(false, timeout, input_state);
}

bool Game::check_for_user_interruption(double timeout, InputState *input_state){
	return this->check_for_user_interruption_internal(true, timeout, input_state);
}

InputState Game::joypad_only_newly_pressed(){
	return this->joypad_pressed;
}

void Game::update_joypad_state(){
	auto old = this->joypad_held;
	this->joypad_held = this->engine->get_input_state();
	this->joypad_pressed = this->joypad_held & ~old;
}

InputState Game::joypad_auto_repeat(){
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

void Game::wait_for_sound_to_finish(){
	//TODO
}

Game::load_save_t Game::load_save(){
	//TODO
	return nullptr;
}

void Game::draw_box(const Point &corner, const Point &size, TileRegion region){
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

void Game::put_string(const Point &position, TileRegion region, const char *string){
	int i = position.x + position.y * Tilemap::w;
	auto tilemap = this->engine->get_renderer().get_tilemap(region).tiles;
	for (; *string; string++){
		tilemap[i].tile_no = (byte_t)*string;
		tilemap[i].flipped_x = false;
		tilemap[i].flipped_y = false;
		i = (i + 1) % Tilemap::size;
	}
}

int Game::handle_standard_menu_with_title(
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
			if (!ignore_b && state.get_b()){
				this->get_audio_interface().play_sound(AudioResourceId::SFX_Press_AB);
				return -1;
			}
			if (state.get_a()){
				this->get_audio_interface().play_sound(AudioResourceId::SFX_Press_AB);
				return current_item;
			}
			if (!(state.get_down() || state.get_up()))
				continue;
			addend = state.get_down() ? 1 : -1;
		}while (!addend);
		tilemap[index].tile_no = ' ';
		current_item = (current_item + n + addend) % n;
	}
	return -1;
}

int Game::handle_standard_menu(TileRegion region, const Point &position, const std::vector<std::string> &items, const Point &minimum_size, bool ignore_b){
	return this->handle_standard_menu_with_title(region, position, items, nullptr, minimum_size, ignore_b);
}

void Game::run_dialog(TextResourceId resource){
	if (!this->dialog_box_visible){
		this->draw_box(this->text_state.box_corner - Point{ 1, 1 }, this->text_state.box_size, TileRegion::Background);
		this->dialog_box_visible = true;
	}
	this->text_store.execute(*this, resource, this->text_state);
}

void Game::reset_dialog_state(){
	this->text_state.region = TileRegion::Background;
	this->text_state.first_position =
		this->text_state.position =
		this->text_state.start_of_line = { 1, Renderer::logical_screen_tile_height - 4 };
	this->text_state.box_corner = { 1, Renderer::logical_screen_tile_height - 5 };
	this->text_state.box_size = { Renderer::logical_screen_tile_width - 2, 4 };
	this->text_state.continue_location = { 18, 16 };
	this->dialog_box_visible = false;
}

void Game::text_print_delay(){
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

std::string Game::get_name_from_user(NameEntryType type, SpeciesId species, int max_length_){
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

std::string Game::get_name_from_user(NameEntryType type, int max_length){
	if (type == NameEntryType::Pokemon)
		throw std::runtime_error("CppRedEngine::get_name_from_user(): Invalid usage. type must not be NameEntryType::Pokemon.");
	auto ret = this->get_name_from_user(type, SpeciesId::None, max_length);
	std::cout << "Selected name: " << ret << std::endl;
	return ret;
}

std::string Game::get_name_from_user(SpeciesId species, int max_length){
	auto ret = this->get_name_from_user(NameEntryType::Pokemon, SpeciesId::None, max_length);
	std::cout << "Selected name: " << ret << std::endl;
	return ret;
}

void Game::create_main_characters(const std::string &player_name, const std::string &rival_name){
	this->player_character.reset(new PlayerCharacter(*this, player_name, this->engine->get_renderer()));
	this->rival.reset(new Trainer(rival_name));
}

void Game::teleport_player(Map destination, const Point &position){
	this->player_character->teleport(destination, position);
}

void Game::game_loop(){
	auto &renderer = this->engine->get_renderer();
	renderer.set_enable_bg(true);
	renderer.set_enable_sprites(true);
	renderer.set_palette(PaletteRegion::Background, default_palette);
	renderer.set_palette(PaletteRegion::Sprites0, default_world_sprite_palette);
	while (true){
		this->player_character->update();
		this->render();
		this->engine->yield();
	}
}

static bool point_in_rectangle(const Point &p, int w, int h){
	return p.x >= 0 && p.y >= 0 && p.x < w && p.y < h;
}

static bool point_in_map(const Point &p, const MapData &map_data){
	return point_in_rectangle(p, map_data.width, map_data.height);
}

int reduce_by_region(int x, int begin, int end){
	if (x < begin)
		return -1;
	if (x >= end)
		return 1;
	return 0;
}

Point reduce_by_region(const Point &p, const MapData &m){
	return {
		reduce_by_region(p.x, 0, m.width),
		reduce_by_region(p.y, 0, m.height),
	};
}

bool in_applicable_region(const Point &reduced, const Point &region){
	return (!region.x || region.x == reduced.x) && (!region.y || region.y == reduced.y);
}

std::pair<MapData *, Point> compute_map_connections(Map map, MapData &map_data, MapStore &map_store, const Point &position){
	struct SimplifiedCheck{
		Point applicable_region;
		int x_multiplier_1;
		int x_multiplier_2;
		int y_multiplier_1;
		int y_multiplier_2;
	};
	static const SimplifiedCheck checks[] = {
		{
			{0, -1},
			1, 0, 0, 1,
		},
		{
			{1, 0},
			0, -1, 1, 0,
		},
		{
			{0, 1},
			1, 0, 0, -1,
		},
		{
			{-1, 0},
			0, 1, 1, 0,
		},
	};
	auto reduced = reduce_by_region(position, map_data);
	for (size_t i = 0; i < array_length(checks); i++){
		if (!map_data.map_connections[i])
			continue;
		auto &check = checks[i];
		if (!in_applicable_region(reduced, check.applicable_region))
			continue;
		auto &mc = map_data.map_connections[i];
		auto &map_data2 = map_store.get_map_data(mc.destination);
		auto transformed = position;
		transformed.x += (mc.local_position - mc.remote_position) * check.x_multiplier_1;
		if (check.x_multiplier_2 > 0)
			transformed.x += map_data2.width * check.x_multiplier_2;
		else if (check.x_multiplier_2 < 0)
			transformed.x += map_data.width * check.x_multiplier_2;
		transformed.y += (mc.local_position - mc.remote_position) * check.y_multiplier_1;
		if (check.y_multiplier_2 > 0)
			transformed.y += map_data2.height * check.y_multiplier_2;
		else if (check.y_multiplier_2 < 0)
			transformed.y += map_data.height * check.y_multiplier_2;
		return {&map_data2, transformed};
	}
	return {nullptr, position};
}

std::pair<TilesetData *, int> Game::compute_virtual_block(Map map, const Point &position){
	auto transformed = this->remap_coordinates(map, position);
	auto &map_data = this->map_store.get_map_data(transformed.first);
	if (point_in_map(transformed.second, map_data))
		return {map_data.tileset.get(), map_data.get_block_at_map_position(transformed.second)};
	return {map_data.tileset.get(), map_data.border_block};
}

std::pair<Map, Point> Game::remap_coordinates(Map map, const Point &position_parameter){
	auto position = position_parameter;
	while (true){
		auto &map_data = this->map_store.get_map_data(map);
		if (point_in_map(position, map_data))
			return {map, position};
		auto transformed = compute_map_connections(map, map_data, this->map_store, position);
		if (!transformed.first)
			return {map, position};
		map = transformed.first->map_id;
		position = transformed.second;
	}
}


void Game::render(){
	auto &renderer = this->engine->get_renderer();
	auto &bg = renderer.get_tilemap(TileRegion::Background);
	auto current_map = this->player_character->get_current_map();
	this->player_character->set_visible_sprite();
	if (current_map == Map::Nowhere){
		renderer.fill_rectangle(TileRegion::Background, { 0, 0 }, { Tilemap::w, Tilemap::h }, Tile());
	}else{
		auto map_data = this->map_store.get_map_data(current_map);
		auto pos = this->player_character->get_map_position();
		for (int y = 0; y < Renderer::logical_screen_tile_height; y++){
			for (int x = 0; x < Renderer::logical_screen_tile_width; x++){
				auto &tile = bg.tiles[x + y * Tilemap::w];
				int x2 = x / 2 - PlayerCharacter::screen_block_offset_x + pos.x;
				int y2 = y / 2 - PlayerCharacter::screen_block_offset_y + pos.y;
				Point map_position(x2, y2);
				auto computed_block = this->compute_virtual_block(current_map, map_position);
				auto &blockset = computed_block.first->blockset->data;
				auto tileset = computed_block.first->tiles;
				auto block = computed_block.second;
				auto offset = x % 2 + y % 2 * 2;
				tile.tile_no = tileset->first_tile + blockset[block * 4 + offset];
				tile.flipped_x = false;
				tile.flipped_y = false;
				tile.palette = null_palette;
			}
		}
	}
}

MapInstance &Game::get_map_instance(Map map){
	return this->map_store.get_map_instance(map);
}

bool Game::is_passable(Map map, const Point &point){
	auto &map_data = this->map_store.get_map_data(map);
	if (!point_in_map(point, map_data))
		return false;
	auto tile = map_data.get_partial_tile_at_actor_position(point);
	auto &v = map_data.tileset->collision->data;
	auto it = std::lower_bound(v.begin(), v.end(), tile);
	return it != v.end() && *it == tile;
}

MoveQueryResult Game::can_move_to(Map map, const Point &current_position, const Point &next_position, FacingDirection direction){
	//TODO: Implement full movement check logic.
	return this->can_move_to_land(map, current_position, next_position, direction);
}

MoveQueryResult Game::can_move_to_land(Map map, const Point &current_position, const Point &next_position, FacingDirection direction){
	//TODO: Implement full movement check logic.
	auto remapped_next_position = this->remap_coordinates(map, next_position);
	auto &map_data = this->map_store.get_map_data(remapped_next_position.first);
	if (!point_in_map(remapped_next_position.second, map_data))
		return MoveQueryResult::CantMoveThere;
	if (remapped_next_position.first != map)
		return MoveQueryResult::CanMoveButWillChangeMaps;
	//if (!this->check_jumping_and_tile_pair_collisions(map, current_position, next_position, direction, &TilesetData::impassability_pairs) || !this->is_passable(map, next_position))
	//	return MoveQueryResult::CantMoveThere;
	//if (!this->is_passable(map, next_position))
	//	return MoveQueryResult::CantMoveThere;
	return MoveQueryResult::CanMoveThere;
}

bool Game::check_jumping_and_tile_pair_collisions(Map map, const Point &current_position, const Point &next_position, FacingDirection direction, pairs_t pairs){
	if (!this->check_jumping(map, current_position, next_position, direction))
		return false;
	return this->check_tile_pair_collisions(map, current_position, next_position, pairs);
}

bool Game::check_jumping(Map map, const Point &current_position, const Point &next_position, FacingDirection direction){
	auto &map_data = this->map_store.get_map_data(map);
	if (!point_in_map(next_position, map_data))
		return false;
	//TODO: See HandleLedges
	return true;
}

bool Game::check_tile_pair_collisions(Map map, const Point &current_position, const Point &next_position, pairs_t pairs){
	auto &map_data = this->map_store.get_map_data(map);
	if (!point_in_map(current_position, map_data) || !point_in_map(next_position, map_data))
		return true;
	auto tile0 = map_data.get_partial_tile_at_actor_position(current_position);
	auto tile1 = map_data.get_partial_tile_at_actor_position(next_position);
	for (auto &pair : (*map_data.tileset).*pairs){
		if (pair.first < 0)
			break;
		if (pair.first == tile0 && pair.second == tile1 || pair.first == tile1 && pair.second == tile0)
			return false;
	}
	return true;
}

}
