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


MovementStatus MovementFlags::get_movement_status() const{
	return (MovementStatus)(this->data % 4);
}

void MovementFlags::set_movement_status(MovementStatus value){
	auto temp = this->data;
	temp = temp - temp % 4;
	this->data |= (unsigned)value % 4;
}

bool MovementFlags::get_face_player() const{
	return check_flag((unsigned)this->data, 1 << 7);
}

void MovementFlags::set_face_player(bool value){
	auto temp = this->data;
	temp = temp - temp & (1 << 7);
	temp |= (1 << 7) * value;
}

#include "../CodeGeneration/output/bitmaps.inl"

void PcBoxMember::clear(){
	this->species = 0;
	this->hp = 0;
	this->box_level = 0;
	this->status = 0;
	this->type1 = 0;
	this->type2 = 0;
	this->catch_rate = 0;
	this->moves.fill_bytes(0);
	this->original_trainer_id = 0;
	this->experience = 0;
	this->hp_xp = 0;
	this->attack_xp = 0;
	this->defense_xp = 0;
	this->speed_xp = 0;
	this->special_xp = 0;
	this->dvs.fill_bytes(0);
	this->pp.fill_bytes(0);
}

void PcBox::clear(){
	for (auto &i : this->mons)
		i.clear();
}

bool TwoItemMenuType_wrapper::get_select_second_item_by_default() const{
	return check_flag<byte_t, byte_t>(this->wrapped_value, bits_from_u32<0x10000000>::value);
}

void TwoItemMenuType_wrapper::set_select_second_item_by_default(bool bit){
	byte_t val = this->wrapped_value;
	val &= bits_from_u32<0x01111111>::value;
	this->wrapped_value = val | (bit << 7);
}

TwoOptionMenuType TwoItemMenuType_wrapper::get_id() const{
	byte_t val = this->wrapped_value;
	return (TwoOptionMenuType)(val & bits_from_u32<0x00000111>::value);
}

void TwoItemMenuType_wrapper::set_id(TwoOptionMenuType type){
	byte_t val = this->wrapped_value;
	val &= bits_from_u32<0x11111000>::value;
	this->wrapped_value = val | ((byte_t)type & bits_from_u32<0x00000111>::value);
}
