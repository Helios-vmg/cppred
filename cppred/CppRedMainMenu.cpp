#include "CppRedMainMenu.h"
#include "CppRed.h"
#include "CppRedSRam.h"
#include "CppRedText.h"
#include "MemoryOperations.h"

CppRedMainMenu::CppRedMainMenu(CppRed &parent): parent(&parent){
}

MainMenuResult CppRedMainMenu::display(){
	auto &wram = this->parent->wram;
	auto &hram = this->parent->hram;

	wram.wMainData.wLetterPrintingDelayFlags.set_raw_value(1);
	wram.wMainData.wOptions = { true, BattleStyle::Shift, TextSpeed::Medium };

	wram.wOptionsInitialized = 0;
	wram.wSaveFileStatus = SaveFileStatus::NoSave;

	if (this->check_for_player_name_in_sram())
		this->parent->call_predef(Predef::LoadSAV);

	auto &text = this->parent->text;

	while (true){
		this->parent->delay_frames(20);
		wram.wLinkState = LinkState::None;
		wram.wPartyAndBillsPCSavedMenuItem = 0;
		wram.wBagSavedMenuItem = 0;
		wram.wBattleAndStartSavedMenuItem = 0;
		wram.wPlayerMoveListIndex = 0;
		wram.wDefaultMap = MapId::PalletTown;
		wram.wMainData.wd72e.set_using_link_cable(false);
		this->parent->prepare_menu();
		wram.wMainData.wd730.set_no_print_delay(true);

		{
			unsigned y;
			CppRedText::Region *region;
			if (wram.wSaveFileStatus == SaveFileStatus::NoSave){
				y = 6;
				region = &text.ContinueText;
			}else{
				y = 4;
				region = &text.NewGameText;
			}
			auto pos = this->parent->get_tilemap_location(0, 0);
			text.text_box_border(pos, 13, y);
			pos = this->parent->get_tilemap_location(2, 2);
			text.place_string(pos, *region);
		}

		wram.wMainData.wd730.set_no_print_delay(false);
		this->parent->update_sprites();

		wram.wCurrentMenuItem = 0;
		wram.wLastMenuItem = 0;
		wram.wMenuJoypadPollCount = 0;
		wram.wTopMenuItemX = 1;
		wram.wTopMenuItemY = 2;
		wram.wMenuWatchedKeys.clear();
		wram.wMenuWatchedKeys.set_button_a(true);
		wram.wMenuWatchedKeys.set_button_b(true);
		wram.wMenuWatchedKeys.set_button_start(true);
		wram.wMaxMenuItem = wram.wSaveFileStatus.value;

		auto input = this->parent->handle_menu_input();
		if (input.button_b)
			return MainMenuResult::Cancelled;

		this->parent->delay_frames(20);

		byte_t status = wram.wCurrentMenuItem;
		if (wram.wSaveFileStatus != SaveFileStatus::SaveExists)
			status++;
		if (status){
			if (status == 1)
				return MainMenuResult::NewGame;
			this->display_options_menu();
			wram.wOptionsInitialized = 1;
			continue;
		}

		//Selected continue.

		this->display_continue_game_info();
		wram.wCurrentMapScriptFlags |= 1 << 5;

		byte_t held;
		do{
			hram.hJoyPressed.clear();
			hram.hJoyReleased.clear();
			hram.hJoyHeld.clear();
			this->parent->joypad();
			held = hram.hJoyHeld.get_raw_value();
			if (check_flag(held, input_a))
				return MainMenuResult::ContinueGame;
		}while (!check_flag(held, input_b));
	}
}

bool CppRedMainMenu::check_for_player_name_in_sram(){
	auto memory = this->parent->load_sram();
	SRam sram(memory.data(), { { read_memory_u8, write_memory_u8 } });
	for (auto &c : sram.player_name)
		if (c == SpecialCharacters::terminator)
			return true;
	return false;
}

void CppRedMainMenu::display_continue_game_info(){
	auto &parent = *this->parent;
	auto &text = parent.text;
	parent.hram.H_AUTOBGTRANSFERENABLED = 0;
	text.text_box_border(parent.get_tilemap_location(4, 7), 14, 8);
	text.place_string(parent.get_tilemap_location(5, 9), text.SaveScreenInfoText);
	text.place_string(parent.get_tilemap_location(12, 9), parent.wram.wPlayerName.to_string());
	this->print_num_badges(parent.get_tilemap_location(18, 11));
	this->print_num_owned_mons(parent.get_tilemap_location(16, 13));
	this->print_play_time(parent.get_tilemap_location(13, 15));
	parent.hram.H_AUTOBGTRANSFERENABLED = 1;
	parent.delay_frames(30);
}

void CppRedMainMenu::print_num_badges(const tilemap_it &location){
	byte_t badge_bitmap = this->parent->wram.wMainData.wObtainedBadges;
	unsigned badge_count = count_set_bits(&badge_bitmap, 1);
	std::stringstream stream;
	stream << badge_count;
	this->parent->text.place_string(location, stream.str());
}

void CppRedMainMenu::print_num_owned_mons(const tilemap_it &location){
	auto &wPokedexOwned = this->parent->wram.wMainData.wPokedexOwned;
	byte_t owned[std::remove_reference<decltype(wPokedexOwned)>::type::size];
	std::copy(wPokedexOwned.begin(), wPokedexOwned.end(), owned);
	unsigned pokedex_count = count_set_bits(owned, array_length(owned));
	std::stringstream stream;
	stream << std::setw(3) << std::setfill(' ') << pokedex_count;
	this->parent->text.place_string(location, stream.str());
}

void CppRedMainMenu::print_play_time(const tilemap_it &location){
	int hours = this->parent->wram.wMainData.wPlayTimeHours;
	int minutes = this->parent->wram.wMainData.wPlayTimeMinutes;
	std::stringstream stream;
	stream
		<< std::setw(3) << std::setfill(' ') << hours << ':'
		<< std::setw(2) << std::setfill('0') << minutes;
	this->parent->text.place_string(location, stream.str());
}

const unsigned speed_y = 3;
const unsigned animation_y = speed_y + 5;
const unsigned style_y = animation_y + 5;
const unsigned cancel_y = style_y + 3;

const unsigned speed_x_fast = 1;
const unsigned speed_x_medium = 7;
const unsigned speed_x_slow = 14;

const unsigned animation_x_on = 1;
const unsigned animation_x_off = 10;

const unsigned style_x_shift = 1;
const unsigned style_x_set = 10;

void CppRedMainMenu::display_options_menu(){
	auto &parent = *this->parent;
	auto &text = parent.text;

	CppRedText::Region *strings[] = {
		&text.TextSpeedOptionText,
		&text.BattleAnimationOptionText,
		&text.BattleStyleOptionText,
	};

	for (int i = 0; i < array_length(strings); i++){
		unsigned y = i * 5;
		text.text_box_border(parent.get_tilemap_location(0, y), 18, 3);
		text.place_string(parent.get_tilemap_location(1, y + 1), *strings[i]);
	}

	text.place_string(parent.get_tilemap_location(2, 3 * 5 + 1), text.OptionMenuCancelText);

	auto &speed_pos = parent.wram.wOptionsTextSpeedCursorX;
	auto &anim_pos = parent.wram.wOptionsBattleAnimCursorX;
	auto &style_pos = parent.wram.wOptionsBattleStyleCursorX;
	auto &cancel_pos = parent.wram.wOptionsCancelCursorX;

	parent.wram.wCurrentMenuItem = 0;
	parent.wram.wLastMenuItem = 0;
	cancel_pos = 1;
	parent.wram.wMainData.wLetterPrintingDelayFlags.clear();
	parent.wram.wMainData.wLetterPrintingDelayFlags.set_long_delay_enable(true);
	auto &menu_position = parent.wram.wTopMenuItemY;
	auto &xpos = parent.wram.wTopMenuItemX;
	menu_position = 3;

	this->set_cursor_positions_from_options();
	xpos = +speed_pos;
	parent.hram.H_AUTOBGTRANSFERENABLED = 1;
	parent.delay3();

	while (true){
		parent.place_menu_cursor();
		this->set_options_from_cursor_positions();
		InputBitmap_struct input;
		do{
			input = parent.joypad_low_sensitivity();
			//TODO: Is this a spinlock?
		}while (!input.button_a & !input.button_b & !input.button_start & !input.button_down & !input.button_up & !input.button_left & !input.button_right);
		if (input.button_b || input.button_start)
			break;
		if (input.button_a){
			if (menu_position == cancel_y)
				break;
			continue;
		}

		//Check direction buttons.
		if (input.button_down){
			if (menu_position == speed_y){
				menu_position = animation_y;
				xpos = +anim_pos;
			}else if (menu_position == animation_y){
				menu_position = style_y;
				xpos = +style_pos;
			}else if (menu_position == style_y){
				menu_position = cancel_y;
				xpos = +cancel_pos;
			}else{
				assert(menu_position == cancel_y);
				menu_position = speed_y;
				xpos = +speed_pos;
			}
			parent.place_unfilled_arrow_menu_cursor();
			continue;
		}
		if (input.button_up){
			if (menu_position == speed_y){
				menu_position = cancel_y;
				xpos = +cancel_pos;
			}else if (menu_position == animation_y){
				menu_position = speed_y;
				xpos = +speed_pos;
			}else if (menu_position == style_y){
				menu_position = animation_y;
				xpos = +anim_pos;
			}else{
				assert(menu_position == cancel_y);
				menu_position = style_y;
				xpos = +style_pos;
			}
			parent.place_unfilled_arrow_menu_cursor();
			continue;
		}
			
		if (menu_position == cancel_y)
			continue;

		if (menu_position == speed_y){
			if (input.button_left){
				if (speed_pos == speed_x_fast){
					//Do nothing
				}else if (speed_pos == speed_x_medium){
					speed_pos = speed_x_fast;
				}else{
					assert(speed_pos == speed_x_slow);
					speed_pos = speed_x_medium;
				}
			}else{
				assert(input.button_right);
				if (speed_pos == speed_x_fast){
					speed_pos = speed_x_medium;
				}else if (speed_pos == speed_x_medium){
					speed_pos = speed_x_slow;
				}else{
					assert(speed_pos == speed_x_slow);
					//Do nothing
				}
			}
			parent.wram.wTopMenuItemX = +speed_pos;
			parent.erase_menu_cursor();
			continue;
		}

		if (menu_position == animation_y){
			//(1 ^ 11) == 10, and (10 ^ 11) == 1
			anim_pos ^= 0x0B;
			parent.wram.wTopMenuItemX = +anim_pos;
			parent.erase_menu_cursor();
			continue;
		}

		assert(menu_position == style_y);
		//(1 ^ 11) == 10, and (10 ^ 11) == 1
		style_pos ^= 0x0B;
		parent.wram.wTopMenuItemX = +style_pos;
		parent.erase_menu_cursor();
	}
	parent.play_sound(Sound::SFX_Press_AB_1);
}

void CppRedMainMenu::set_cursor_positions_from_options(){
	auto &parent = *this->parent;
	auto &speed_pos = parent.wram.wOptionsTextSpeedCursorX;
	auto &anim_pos = parent.wram.wOptionsBattleAnimCursorX;
	auto &style_pos = parent.wram.wOptionsBattleStyleCursorX;
	auto &options = parent.wram.wMainData.wOptions;

	switch (options.get_text_speed()){
		case TextSpeed::Fast:
			speed_pos = speed_x_fast;
			break;
		case TextSpeed::Medium:
			speed_pos = speed_x_medium;
			break;
		case TextSpeed::Slow:
			speed_pos = speed_x_slow;
			break;
		default:
			throw std::runtime_error("CppRedMainMenu::set_cursor_positions_from_options(): Invalid switch.");
	}
	
	anim_pos = options.get_battle_animation_enabled() ? animation_x_on : animation_x_off;

	switch (options.get_battle_style()){
		case BattleStyle::Shift:
			style_pos = style_x_shift;
			break;
		case BattleStyle::Set:
			style_pos = style_x_set;
			break;
		default:
			throw std::runtime_error("CppRedMainMenu::set_cursor_positions_from_options(): Invalid switch.");
	}

	*parent.get_tilemap_location(speed_pos, speed_y) = SpecialCharacters::arrow_white_right;
	*parent.get_tilemap_location(anim_pos, animation_y) = SpecialCharacters::arrow_white_right;
	*parent.get_tilemap_location(style_pos, style_y) = SpecialCharacters::arrow_white_right;
	*parent.get_tilemap_location(1, cancel_y) = SpecialCharacters::arrow_white_right;
}

void CppRedMainMenu::set_options_from_cursor_positions(){
	auto &parent = *this->parent;
	auto &speed_pos = parent.wram.wOptionsTextSpeedCursorX;
	auto &anim_pos = parent.wram.wOptionsBattleAnimCursorX;
	auto &style_pos = parent.wram.wOptionsBattleStyleCursorX;
	auto &options = parent.wram.wMainData.wOptions;

	switch (speed_pos){
		case speed_x_fast:
			options.set_text_speed(TextSpeed::Fast);
			break;
		case speed_x_medium:
			options.set_text_speed(TextSpeed::Medium);
			break;
		case speed_x_slow:
			options.set_text_speed(TextSpeed::Slow);
			break;
		default:
			throw std::runtime_error("CppRedMainMenu::set_options_from_cursor_positions(): Invalid switch.");
	}

	switch (anim_pos){
		case animation_x_off:
			options.set_battle_animation_enabled(false);
			break;
		case animation_x_on:
			options.set_battle_animation_enabled(true);
			break;
		default:
			throw std::runtime_error("CppRedMainMenu::set_options_from_cursor_positions(): Invalid switch.");
	}

	switch (style_pos){
		case style_x_shift:
			options.set_battle_style(BattleStyle::Shift);
			break;
		case style_x_set:
			options.set_battle_style(BattleStyle::Set);
			break;
		default:
			throw std::runtime_error("CppRedMainMenu::set_options_from_cursor_positions(): Invalid switch.");
	}
}
