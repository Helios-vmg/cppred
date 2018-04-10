#include "stdafx.h"
#include "Game.h"
#include "Engine.h"
#include "Renderer.h"
#include "PlayerCharacter.h"
#include "Maps.h"
#include "Data.h"
#include "World.h"
#include "TextDisplay.h"
#include "EntryPoint.h"
#include "Scripts/Scripts.h"
#include "../../CodeGeneration/output/audio.h"
#include "../../CodeGeneration/output/variables.h"
#include "../Console.h"
#include "PokedexPageDisplay.h"
#include "CoroutineExecuter.h"
#include "BattleOwner.h"
#ifndef HAVE_PCH
#include <iostream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <array>
#endif

namespace CppRed{

struct FadePaletteData{
	Palette background_palette;
	Palette obp0_palette;
	Palette obp1_palette;
};

//Note: Palettes 3 and 4 are identical and equal to the default palette. Palettes >= 4 are used for fade-outs
//      to white, while palettes <= 3 are used for fade-outs to black.
const FadePaletteData fade_palettes[8] = {
	/* 0 */ { BITMAP(11111111), BITMAP(11111111), BITMAP(11111111) },
	/* 1 */ { BITMAP(11111110), BITMAP(11111110), BITMAP(11111000) },
	/* 2 */ { BITMAP(11111001), BITMAP(11100100), BITMAP(11100100) },
	/* 3 */ { BITMAP(11100100), BITMAP(11010000), BITMAP(11100000) },
	/* 4 */ { BITMAP(11100100), BITMAP(11010000), BITMAP(11100000) },
	/* 5 */ { BITMAP(10010000), BITMAP(10000000), BITMAP(10010000) },
	/* 6 */ { BITMAP(01000000), BITMAP(01000000), BITMAP(01000000) },
	/* 7 */ { BITMAP(00000000), BITMAP(00000000), BITMAP(00000000) },
};

Game::Game(Engine &engine, PokemonVersion version, CppRed::AudioProgramInterface &program):
		engine(&engine),
		version(version),
		audio_interface(program){
	this->world.reset(new World(*this));
	this->coroutine.reset(new Coroutine("Game coroutine", this->engine->get_stepping_clock(), [this](Coroutine &){ Scripts::entry_point(*this); }));
	this->coroutine->set_on_yield([this](){ this->update_joypad_state(); });
	this->reset_dialogue_state();
}

Game::~Game(){
}

void Game::update(){
	this->coroutine->get_clock().step();
	this->coroutine->resume();
}

void Game::clear_screen(){
	this->engine->get_renderer().clear_screen();
	Coroutine::get_current_coroutine().wait_frames(3);
}

void Game::fade_out_to_white(){
	auto &engine = *this->engine;
	auto &renderer = engine.get_renderer();
	auto &coroutine = Coroutine::get_current_coroutine();
	for (int i = 0; i < 3; i++){
		auto &palette = fade_palettes[5 + i];
		renderer.set_palette(PaletteRegion::Background, palette.background_palette);
		renderer.set_palette(PaletteRegion::Sprites0, palette.obp0_palette);
		renderer.set_palette(PaletteRegion::Sprites1, palette.obp1_palette);
		coroutine.wait_frames(8);
	}
}

void Game::fade_out_to_black(){
	auto &engine = *this->engine;
	auto &renderer = engine.get_renderer();
	auto &coroutine = Coroutine::get_current_coroutine();
	for (int i = 0; i < 3; i++){
		auto &palette = fade_palettes[2 - i];
		renderer.set_palette(PaletteRegion::Background, palette.background_palette);
		renderer.set_palette(PaletteRegion::Sprites0, palette.obp0_palette);
		renderer.set_palette(PaletteRegion::Sprites1, palette.obp1_palette);
		coroutine.wait_frames(8);
	}
}

void Game::palette_whiteout(){
	auto &renderer = this->engine->get_renderer();
	renderer.clear_subpalettes(SubPaletteRegion::All);
	renderer.set_palette(PaletteRegion::Background, zero_palette);
	renderer.set_palette(PaletteRegion::Sprites0, zero_palette);
	renderer.set_palette(PaletteRegion::Sprites1, zero_palette);
}

void Game::palette_blackout(){
	static const Palette black_palette = { 3, 3, 3, 3 };
	
	auto &renderer = this->engine->get_renderer();
	renderer.clear_subpalettes(SubPaletteRegion::All);
	renderer.set_palette(PaletteRegion::Background, black_palette);
	renderer.set_palette(PaletteRegion::Sprites0, black_palette);
	renderer.set_palette(PaletteRegion::Sprites1, black_palette);
}

bool Game::check_for_user_interruption_internal(bool autorepeat, double timeout, InputState *input_state){
	auto coroutine = Coroutine::get_current_coroutine_ptr();
	if (!coroutine)
		throw std::runtime_error("Internal error: check_for_user_interruption_internal() must be called while a coroutine is running!");
	auto &c = coroutine->get_clock();
	timeout += c.get();
	do{
		coroutine->yield();
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
	}while (c.get() < timeout);
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
	auto &clock = Coroutine::get_current_coroutine().get_clock();
	if (pressed.get_value()){
		this->jls_timeout = clock.get() + 0.5;
		return held;
	}
	if (clock.get() < this->jls_timeout)
		return InputState();
	if (held.get_value())
		this->jls_timeout = clock.get() + 5.0/60.0;
	else
		this->jls_timeout = std::numeric_limits<double>::max();
	
	return held;
}

void Game::reset_joypad_state(){
	this->joypad_pressed = InputState();
	this->jls_timeout = std::numeric_limits<double>::max();
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
		throw std::runtime_error("CppRedEngine::draw_box(): invalid position.");
	//if (size.x > Renderer::logical_screen_tile_width - 2 || size.y > Renderer::logical_screen_tile_height - 2)
	//	throw std::runtime_error("CppRedEngine::draw_box(): invalid dimensions.");
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

static void write_menu_strings(Game &game, StandardMenuOptions &options, const Point &position, int window_position, int window_size){
	auto string_position = position;
	string_position.x += 2;
	string_position.y += options.initial_padding ? 2 : 1;
	auto &items = *options.items;
	int limit = std::min(window_size, (int)items.size() - window_position);
	auto &renderer = game.get_engine().get_renderer();
	for (int i = 0; i < limit; i++){
		auto index = i + window_position;
		auto &s = items[index];
		renderer.put_string(string_position, TileRegion::Window, s.c_str());
		string_position.y++;
		if (options.item_spacing > 1){
			if (options.extra_data)
				renderer.put_string(string_position, TileRegion::Window, (*options.extra_data)[index].c_str());
			string_position.y += options.item_spacing - 1;
		}
	}
}

bool Game::handle_standard_menu(StandardMenuOptions &options, const std::vector<std::function<void()>> &callbacks){
	if (options.items->size() != callbacks.size()){
		std::stringstream stream;
		stream << "Game::handle_standard_menu(): Internal error. Incorrect usage. items.size() = "
			<< options.items->size() << ", callbacks.size() = " << callbacks.size();
		throw std::runtime_error(stream.str());
	}
	auto input = this->handle_standard_menu(options);
	if (input < 0)
		return false;
	callbacks[input]();
	return true;
}

int Game::handle_standard_menu(StandardMenuOptions &options){
	auto position = options.position;
	auto width = options.minimum_size.x;
	auto n = (int)options.items->size();
	for (auto &s : *options.items)
		width = std::max((int)s.size() + 1, width);
	
	Point size(width, std::max(n * 2 - !options.initial_padding, options.minimum_size.y));

	size.x = std::min(size.x, options.maximum_size.x);
	size.y = std::min(size.y, options.maximum_size.y);
	
	position.x = std::min(position.x, Renderer::logical_screen_tile_width  - (size.x + 2));
	position.y = std::min(position.y, Renderer::logical_screen_tile_height - (size.y + 2));
	
	this->draw_box(position, size, TileRegion::Window);

	auto text_region_position = position + Point(1, 1);
	auto text_region_size = size;
	size += {2, 2};

	auto &renderer = this->engine->get_renderer();
	AutoRendererWindowPusher pusher; 
	if (options.push_window)
		pusher = AutoRendererWindowPusher(renderer);
	renderer.set_window_origin({0, 0});
	renderer.set_window_region_start(position * Renderer::tile_size);
	renderer.set_window_region_size(size * Renderer::tile_size);
	auto old_window = renderer.get_enable_window();
	renderer.set_enable_window(true);

	if (options.before_item_display)
		options.before_item_display();

	if (options.title){
		auto l = (int)strlen(options.title);
		renderer.put_string(position + Point((width - l) / 2 + 1, 0), TileRegion::Window, options.title);
	}

	bool redraw_options = true;
	int &current_item = options.initial_item;
	int &window_position = options.initial_window_position;
	int window_size = options.window_size;
	if (window_size == StandardMenuOptions::int_max){
		window_size = (int)options.items->size();
		write_menu_strings(*this, options, position, window_position, window_size);
		redraw_options = false;
	}

	auto tilemap = renderer.get_tilemap(TileRegion::Window).tiles;
	this->reset_joypad_state();
	while (true){
		if (redraw_options){
			renderer.fill_rectangle(TileRegion::Window, text_region_position, text_region_size, Tile());
			write_menu_strings(*this, options, position, window_position, window_size);
			redraw_options = false;
		}
		
		auto index = position.x + 1 + (position.y + (current_item + 1 - window_position) * 2 - !options.initial_padding) * Tilemap::w;
		tilemap[index].tile_no = black_arrow;
		if (options.on_item_hover)
			options.on_item_hover(current_item);
		int addend = 0;
		do{
			Coroutine::get_current_coroutine().yield();
			auto state = this->joypad_auto_repeat();
			if (!options.ignore_b && !!(state.get_value() & options.cancel_mask)){
				if (state.get_b())
					this->get_audio_interface().play_sound(AudioResourceId::SFX_Press_AB);
				renderer.set_enable_window(old_window);
				this->reset_joypad_state();
				return -1;
			}
			if (state.get_a()){
				this->get_audio_interface().play_sound(AudioResourceId::SFX_Press_AB);
				renderer.set_enable_window(old_window);
				this->reset_joypad_state();
				return current_item;
			}
			if (!(state.get_down() || state.get_up()))
				continue;
			addend = state.get_down() ? 1 : -1;
		}while (!addend);
		tilemap[index].tile_no = ' ';
		current_item = (current_item + n + addend) % n;
		if (current_item >= window_position + window_size){
			window_position = current_item - (window_size - 1);
			redraw_options = true;
		}else if (current_item < window_position){
			window_position = current_item;
			redraw_options = true;
		}
	}
	assert(false);
	this->reset_joypad_state();
	return -1;
}

void Game::dialogue_wait(){
	PromptCommand::wait_for_continue(*this, this->text_state, false);
}

void Game::run_dialogue(TextResourceId resource, bool wait_at_end, bool hide_dialogue_at_end){
	if (this->check_screen_owner<TextDisplay>(true, resource, wait_at_end, hide_dialogue_at_end))
		return;
	if (!this->dialogue_box_visible || this->dialogue_box_clear_required){
		auto dialogue_state = Game::get_default_dialogue_state();
		auto &renderer = this->engine->get_renderer();
		renderer.set_window_region_start((dialogue_state.box_corner - Point(1, 1)) * Renderer::tile_size);
		renderer.set_window_region_size((dialogue_state.box_size + Point(2, 2)) * Renderer::tile_size);
		renderer.set_enable_window(true);
		this->draw_box(this->text_state.box_corner - Point{ 1, 1 }, this->text_state.box_size, TileRegion::Window);
		this->dialogue_box_visible = true;
		this->dialogue_box_clear_required = false;
	}
	this->text_store.execute(*this, resource, this->text_state);
	if (wait_at_end)
		this->dialogue_wait();
	if (this->reset_dialogue_was_delayed || hide_dialogue_at_end){
		this->reset_dialogue_state();
		this->reset_dialogue_was_delayed = false;
	}
	this->reset_joypad_state();
}

void Game::run_dex_entry(TextResourceId id){
	auto dialogue_state = Game::get_default_dialogue_state();
	dialogue_state.box_corner.y -= 2;
	dialogue_state.box_size.y += 2;
	dialogue_state.first_position.y -= 3;
	dialogue_state.position =
		dialogue_state.start_of_line =
			dialogue_state.first_position;
	auto &renderer = this->engine->get_renderer();
	renderer.set_window_region_start((dialogue_state.box_corner) * Renderer::tile_size);
	renderer.set_window_region_size((dialogue_state.box_size) * Renderer::tile_size);
	renderer.set_enable_window(true);
	auto old = this->no_text_delay;
	this->no_text_delay = true;
	this->text_store.execute(*this, id, dialogue_state);
	this->no_text_delay = old;
	this->dialogue_wait();
	this->reset_joypad_state();
}

TextState Game::get_default_dialogue_state(){
	TextState ret;
	ret.first_position =
		ret.position =
		ret.start_of_line = { 1, Renderer::logical_screen_tile_height - 4 };
	ret.box_corner = standard_dialogue_box_position + Point(1, 1);
	ret.box_size = standard_dialogue_box_size;
	ret.continue_location = { 18, 16 };
	return ret;
}

void Game::reset_dialogue_state(bool also_hide_window){
	this->text_state = this->get_default_dialogue_state();
	this->dialogue_box_clear_required = true;
	if (also_hide_window){
		this->engine->get_renderer().set_enable_window(false);
		this->engine->get_renderer().set_window_region_size(Point());
		this->dialogue_box_visible = false;
	}
}

void Game::text_print_delay(){
	if (this->no_text_delay)
		return;
	Coroutine::get_current_coroutine().wait_frames((int)this->options.text_speed);
}

void VariableStore::set(StringVariableId id, const std::string &val){
	set_vector<0>(this->strings, id, val);
}

void VariableStore::set(IntegerVariableId id, int val){
	set_vector<0>(this->integers, id, val);
}

void VariableStore::set(EventId id, bool val){
	set_vector<1>(this->events, id, val);
}

void VariableStore::set(VisibilityFlagId id, bool val){
	set_vector<1>(this->vibility_flags, id, val);
}

const std::string &VariableStore::get(StringVariableId id){
	return get_vector<0>(this->strings, id);
}

int VariableStore::get(IntegerVariableId id){
	return get_vector<0>(this->integers, id);
}

bool VariableStore::get(EventId id){
	return get_vector<1>(this->events, id);
}

bool VariableStore::get(VisibilityFlagId id){
	return get_vector<1>(this->vibility_flags, id);
}

void VariableStore::load_initial_visibility_flags(){
	this->vibility_flags.resize(default_sprite_vibisilities_size * 8);
	for (size_t i = 0; i < default_sprite_vibisilities_size; i++)
		for (int j = 0; j < 8; j++)
			this->vibility_flags[i * 8 + j] = !!(default_sprite_vibisilities[i] & (1 << j));
}

std::string Game::get_name_from_user(NameEntryType type, SpeciesId species, int max_length_){
	std::string ret;
	if (this->check_screen_owner<CoroutineExecuter>(true, [this, &ret, type, species, max_length_](){
		ret = this->get_name_from_user(type, species, max_length_);
	}))
		return ret;

	if (!max_length_)
		return "";
	size_t max_display_length = type == NameEntryType::Pokemon ? max_pokemon_name_size : max_character_name_size;
	size_t max_length;
	if (max_length_ < 0)
		max_length = max_display_length;
	else
		max_length = max_length_;

	auto &renderer = this->engine->get_renderer();
	AutoRendererPusher pusher(renderer);
	renderer.clear_screen();
	renderer.clear_sprites();
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
	renderer.put_string({ 0, 1 }, TileRegion::Background, query_string.c_str());
	if (type == NameEntryType::Pokemon)
		renderer.put_string({ 1, 3 }, TileRegion::Background, "NICKNAME?");

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
	auto &coroutine = Coroutine::get_current_coroutine();
	std::array<std::shared_ptr<Sprite>, 2> sprites;
	if (type == NameEntryType::Pokemon){
		sprites = this->load_mon_sprites(species);
		sprites[0]->set_position({8, 0});
		sprites[1]->set_position({8, 0});
		renderer.set_palette(PaletteRegion::Sprites0, default_world_sprite_palette);
	}
	auto &clock = coroutine.get_clock();
	auto initial_time = clock.get();
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
			renderer.put_string(mode_select_location, TileRegion::Background, lower_case ? mode_select_upper : mode_select_lower);
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

		while (true){
			coroutine.yield();

			if (type == NameEntryType::Pokemon){
				auto current_time = clock.get();
				auto frame = (int)((current_time - initial_time) * (60.0 / 17.0));
				frame %= 2;
				sprites[frame]->set_visible(true);
				frame = (frame + 1) % 2;
				sprites[frame]->set_visible(false);
			}

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
	Logger() << "Selected name: " << ret << '\n';
	return ret;
}

std::string Game::get_name_from_user(SpeciesId species, int max_length){
	auto ret = this->get_name_from_user(NameEntryType::Pokemon, species, max_length);
	Logger() << "Selected name: " << ret << '\n';
	return ret;
}

void Game::create_main_characters(const std::string &player_name, const std::string &rival_name){
	this->world->create_main_characters(player_name, rival_name);
	this->variable_store.set(StringVariableId::player_name, player_name);
	this->variable_store.set(StringVariableId::rival_name, rival_name);
}

void Game::game_loop(){
	while (this->update_internal())
		this->coroutine->yield();
}

ScreenOwner *Game::get_current_owner(){
	return !this->owners_stack.size() ? this->world.get() : this->owners_stack.back().get();
}

bool Game::update_internal(){
	auto current_owner = this->get_current_owner();
	switch (current_owner->run()){
		case ScreenOwner::RunResult::Continue:
			break;
		case ScreenOwner::RunResult::Terminate:
			if (!this->owners_stack.size())
				return false;
			this->owners_stack.pop_back();
			current_owner = nullptr;
			break;
	}
	auto new_owner = this->get_current_owner();
	if (new_owner != current_owner && current_owner)
		current_owner->pause();
	return true;
}

void Game::entered_map(Map old_map, Map new_map, bool warped){
	auto &ms = this->world->get_map_store();
	bool reset = false;
	if (warped && (old_map != Map::Nowhere && new_map != Map::Nowhere)){
		auto &old_md = ms.get_map_data(old_map);
		auto &new_md = ms.get_map_data(new_map);
		bool go_inside = old_md.tileset->type == TilesetType::Outdoor && new_md.tileset->type == TilesetType::Indoor;
		this->audio_interface.play_sound(go_inside ? AudioResourceId::SFX_Go_Inside : AudioResourceId::SFX_Go_Outside);
		this->fade_out_to_black();
		reset = true;
	}
	this->world->entered_map(old_map, new_map, warped);
	if (reset){
		auto &c = Coroutine::get_current_coroutine();
		//this->palette_whiteout();
		//c.wait_frames(3);
		//this->palette_blackout();
		c.wait_frames(3);
		this->world->set_default_palettes();
	}
}

void Game::teleport_player(const WorldCoordinates &wc){
	this->world->teleport_player(wc);
}

void Game::execute(const char *script_name, Actor &caller, const char *parameter){
	Scripts::script_parameters params;
	params.script_name = script_name;
	params.game = this;
	params.caller = &caller;
	params.parameter = parameter;
	this->engine->execute_script(params);
}

void Game::display_pokedex_page(PokedexId id){
	this->check_screen_owner<PokedexPageDisplay>(false, id);
}

void Game::draw_portrait(const GraphicsAsset &graphics, TileRegion region, const Point &corner, bool flipped){
	auto point = corner;
	if (graphics.height < 7)
		point.y += 7 - graphics.height;
	if (graphics.width)
		point.x += (7 - graphics.width) / 2;
	if (!flipped)
		this->engine->get_renderer().draw_image_to_tilemap(point, graphics, region);
	else
		this->engine->get_renderer().draw_image_to_tilemap_flipped(point, graphics, region);
}

bool Game::run_yes_no_menu(const Point &point){
	std::vector<std::string> items = {
		"YES",
		"NO",
	};
	AutoRendererWindowPusher pusher(this->engine->get_renderer());
	StandardMenuOptions options;
	options.position = point;
	options.items = &items;
	options.initial_padding = false;
	return this->handle_standard_menu(options) == 0;
}

bool Game::run_in_own_coroutine(std::function<void()> &&f, bool synchronous){
	this->switch_screen_owner<CoroutineExecuter>(std::move(f));
	auto c = Coroutine::get_current_coroutine_ptr();
	if (c != this->coroutine.get()){
		if (synchronous)
			c->yield();
		return true;
	}
	return !synchronous;
}

std::array<std::shared_ptr<Sprite>, 2> Game::load_mon_sprites(SpeciesId species){
	auto &data = *pokemon_by_species_id[(int)species];
	auto &renderer = this->engine->get_renderer();
	std::array<std::shared_ptr<Sprite>, 2> ret;
	ret[0] = renderer.create_sprite(2, 2);
	ret[1] = renderer.create_sprite(2, 2);
	auto first_tile = MonPartySprites2.first_tile + (int)data.overworld_sprite;
	for (int j = 0; j < 2; j++){
		int i = 0;
		for (SpriteTile &tile : ret[j]->iterate_tiles()){
			tile.tile_no = first_tile + i % 2 + i / 2 * MonPartySprites2.width + j * MonPartySprites2.width * 2;
			tile.has_priority = true;
			i++;
		}
	}
	return ret;
}

BattleResult Game::run_trainer_battle(TextResourceId player_victory_text, TextResourceId player_defeat_text, FullTrainerClass &&ftc){
	auto battle = this->switch_screen_owner<BattleOwner>(std::move(ftc));
	Coroutine::get_current_coroutine().yield();
	return battle->get_result();
}

static std::array<char, 64> generate_quantity_string(int q, int digits){
	std::array<char, 64> ret;
	int size = 0;
	const int n = 64 - 1;
	ret[n - size] = 0;
	if (q){
		int temp = q;
		while (temp){
			ret[n - ++size] = '0' + temp % 10;
			temp /= 10;
		}
	}
	for (int i = size; i < digits; i++)
		ret[n - ++size] = '0';
	ret[n - ++size] = '*';
	for (int i = 0; i < size + 1; i++)
		ret[i] = ret[n - size + i];
	return ret;
}

int Game::get_quantity_from_user(const GetQuantityFromUserOptions &options){
	auto position = options.position;
	Point size(0, 1);
	if (options.max)
		size.x = std::max(options.minimum_digits, (int)floor(log10(options.max) + 1));
	size.x++;

	if (position.x + (size.x + 2) > Renderer::logical_screen_tile_width)
		position.x = Renderer::logical_screen_tile_width - (size.x + 2);

	this->draw_box(position, size, TileRegion::Window);

	auto text_region_position = position + Point(1, 1);
	auto text_region_size = size;
	size += {2, 2};

	auto &renderer = this->engine->get_renderer();
	AutoRendererWindowPusher pusher; 
	if (options.push_window)
		pusher = AutoRendererWindowPusher(renderer);
	renderer.set_window_origin({0, 0});
	renderer.set_window_region_start(position * Renderer::tile_size);
	renderer.set_window_region_size(size * Renderer::tile_size);
	auto old_window = renderer.get_enable_window();
	renderer.set_enable_window(true);

	int ret = options.min;

	this->reset_joypad_state();
	while (true){
		renderer.put_string(text_region_position, TileRegion::Window, generate_quantity_string(ret, text_region_size.x - 1).data());

		while (true){
			Coroutine::get_current_coroutine().yield();
			auto state = this->joypad_auto_repeat();
			if (!options.ignore_b && state.get_b()){
				renderer.set_enable_window(old_window);
				this->reset_joypad_state();
				return options.min - 1;
			}
			if (state.get_a()){
				renderer.set_enable_window(old_window);
				this->reset_joypad_state();
				return ret;
			}
			if (state.get_up()){
				ret++;
				break;
			}
			if (state.get_down()){
				ret--;
				break;
			}
			if (state.get_right()){
				ret += 10;
				break;
			}
			if (state.get_left()){
				ret -= 10;
				break;
			}
		}
		ret = euclidean_modulo(ret - options.min, options.max - options.min + 1) + options.min;
	}
	assert(false);
	this->reset_joypad_state();
	return -1;
}

}
