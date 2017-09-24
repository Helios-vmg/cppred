#include "CppRedEngine.h"
#include "Engine.h"
#include "Renderer.h"

CppRedEngine::CppRedEngine(Engine &engine): engine(&engine){
}

void CppRedEngine::clear_screen(){
	this->engine->get_renderer().clear_screen();
	this->engine->wait_frames(3);
}

void CppRedEngine::play_sound(SoundId sound){
	//TODO
}

void CppRedEngine::fade_out_to_white(){
	//TODO
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
