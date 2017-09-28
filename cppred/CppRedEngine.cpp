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
		if (held.get_value() == mask || held.get_a() || held.get_start()){
			if (input_state)
				*input_state = held;
			return true;
		}
	}while (this->engine->get_clock() < timeout);
	return false;
}

InputState CppRedEngine::joypad_low_sensitivity(){
	//TODO: There's some kind of weird bug here. This function sometimes
	//recognizes multiple button presses even if a button was pressed only once.

	auto held = this->engine->get_input_state();

	auto old = this->jls_last_state;
	this->jls_last_state = held;
	auto pressed = held & ~old;
	InputState ret = pressed;
	if (this->jls_mode != JlsMode::Normal)
		ret = held;
	if (pressed.get_value()){
		this->jls_timeout = this->engine->get_clock() + 0.5;
		return ret;
	}
	if (this->engine->get_clock() < this->jls_timeout)
		return InputState();
	if ((held.get_a() | held.get_b()) && this->jls_mode == JlsMode::HoldingRestricts)
		ret = InputState();
	this->jls_timeout = this->engine->get_clock() + 5.0/60.0;
	
	return ret;
}

void CppRedEngine::wait_for_sound_to_finish(){
	//TODO
}
