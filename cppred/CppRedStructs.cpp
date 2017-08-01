#include "CppRedStructs.h"
#include "MemoryOperations.h"
#include "utility.h"

const byte_t battle_animation_enabled_mask = 1 << 7;
const byte_t battle_style_mask = 1 << 6;
const byte_t text_speed_mask = bits_from_u32<0x00001111>::value;

bool UserOptions::get_battle_animation_enabled() const{
	return ((OptionsTuple)*this).battle_animations_enabled;
}

void UserOptions::set_battle_animation_enabled(bool enabled){
	auto o = (OptionsTuple)*this;
	o.battle_animations_enabled = enabled;
	*this = o;
}

BattleStyle UserOptions::get_battle_style() const{
	return ((OptionsTuple)*this).battle_style;
}

void UserOptions::set_battle_style(BattleStyle style){
	auto o = (OptionsTuple)*this;
	o.battle_style = style;
	*this = o;
}

TextSpeed UserOptions::get_text_speed() const{
	return ((OptionsTuple)*this).text_speed;
}

void UserOptions::set_text_speed(TextSpeed speed){
	auto o = (OptionsTuple)*this;
	o.text_speed = speed;
	*this = o;
}

UserOptions::operator OptionsTuple() const{
	byte_t val = this->options;
	OptionsTuple ret;
	//Bit 7: 1 -> off, 0 -> on
	ret.battle_animations_enabled = !check_flag<byte_t>(val, battle_animation_enabled_mask);
	ret.battle_style = (BattleStyle)((val >> 6) & 1);
	auto speed = val & text_speed_mask;
	switch (speed){
		case 1:
		case 3:
		case 5:
			break;
		default:
			speed = 3;
	}
	ret.text_speed = (TextSpeed)speed;
	return ret;
}

void UserOptions::operator=(const OptionsTuple &o){
	byte_t val = 0;
	//Bit 7: 1 -> off, 0 -> on
	if (!o.battle_animations_enabled)
		val |= battle_animation_enabled_mask;
	val |= battle_style_mask * ((byte_t)o.battle_style & 1);
	auto speed = (byte_t)o.text_speed;
	switch (speed){
		case 1:
		case 3:
		case 5:
			break;
		default:
			speed = 3;
	}
	val |= speed & text_speed_mask;
	this->options = val;

}

#include "../CodeGeneration/output/bitmaps.inl"
