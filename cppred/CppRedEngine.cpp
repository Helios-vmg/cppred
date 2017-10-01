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
	this->text_state.region = TileRegion::Background;
	this->text_state.first_position =
		this->text_state.position =
		this->text_state.start_of_line = { 1, Renderer::logical_screen_tile_height - 4 };
	this->text_state.box_corner = { 1, Renderer::logical_screen_tile_height - 5 };
	this->text_state.box_size = { Renderer::logical_screen_tile_width - 2, 4};
	this->text_state.continue_location = { 18, 16 };
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

bool CppRedEngine::check_for_user_interruption(double timeout, InputState *input_state){
	timeout += this->engine->get_clock();
	do{
		this->engine->wait_exactly_one_frame();
		auto input = this->joypad_low_sensitivity();
		auto held = this->engine->get_input_state();
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

InputState CppRedEngine::joypad_low_sensitivity(){
	auto held = this->engine->get_input_state();

	auto old = this->jls_last_state;
	this->jls_last_state = held;
	auto pressed = held & ~old;
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

int CppRedEngine::handle_standard_menu(TileRegion region, const Point &position_, const std::vector<std::string> &items, int minimum_width, bool ignore_b){
	auto position = position_;
	auto width = minimum_width;
	auto n = (int)items.size();
	for (auto &s : items)
		width = std::max((int)s.size() + 1, width);
	if (position.x + width + 2 > Renderer::logical_screen_tile_width)
		position.x = Renderer::logical_screen_tile_width - (width + 2);
	if (position.y + n * 2 + 2 > Renderer::logical_screen_tile_height)
		position.y = Renderer::logical_screen_tile_height - (n * 2 + 2);

	this->draw_box(position, { width, n * 2 }, region);

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
			auto state = this->joypad_low_sensitivity();
			if (!ignore_b && state.get_b())
				return -1;
			if (state.get_a())
				return current_item;
			if (!(state.get_down() || state.get_up()))
				continue;
			addend = state.get_down() ? 1 : -1;
		}while (!addend);
		tilemap[index].tile_no = ' ';
		current_item = (current_item + addend) % n;
	}
	return -1;
}

void CppRedEngine::run_dialog(TextResourceId resource){
	if (!this->dialog_box_visible){
		this->draw_box(this->text_state.box_corner - Point{ 1, 1 }, this->text_state.box_size, TileRegion::Background);
		this->dialog_box_visible = true;
	}
	this->text_store.execute(*this, resource, this->text_state);
}

void CppRedEngine::text_print_delay(){
	this->engine->wait_frames((int)this->options.text_speed);
}
