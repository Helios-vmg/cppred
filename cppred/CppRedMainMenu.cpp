#include "CppRedMainMenu.h"
#include "CppRed.h"

CppRedMainMenu::CppRedMainMenu(CppRed &parent): parent(&parent){
}

MainMenuResult CppRedMainMenu::display(){
	auto &wram = this->parent->wram;
	auto &hram = this->parent->hram;

	wram.wLetterPrintingDelayFlags = 1;
	wram.wOptions = { true, BattleStyle::Shift, TextSpeed::Medium };

	wram.wOptionsInitialized = 0;
	wram.wSaveFileStatus = SaveFileStatus::NoSave;

	if (this->check_for_player_name_in_sram())
		this->load_save();

	auto &text = this->parent->text;

	while (true){
		this->parent->delay_frames(20);
		wram.wLinkState = LinkState::None;
		wram.wPartyAndBillsPCSavedMenuItem = 0;
		wram.wBagSavedMenuItem = 0;
		wram.wBattleAndStartSavedMenuItem = 0;
		wram.wPlayerMoveListIndex = 0;
		wram.wDefaultMap = 0;
		wram.wd72e &= ~(1 << 6);
		this->parent->prepare_menu();
		{
			wd730Tuple temp = wram.wd730;
			temp.no_print_delay = true;
			wram.wd730 = temp;
		}

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

		{
			wd730Tuple temp = wram.wd730;
			temp.no_print_delay = false;
			wram.wd730 = temp;
		}
		this->parent->update_sprites();

		wram.wCurrentMenuItem = 0;
		wram.wLastMenuItem = 0;
		wram.wMenuJoypadPollCount = 0;
		wram.wTopMenuItemX = 1;
		wram.wTopMenuItemY = 2;
		wram.wMenuWatchedKeys = input_a | input_b | input_start;
		wram.wMaxMenuItem = wram.wSaveFileStatus.value;

		auto input = this->parent->handle_menu_input();
		if (check_flag(input, input_b))
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
			hram.hJoyPressed = 0;
			hram.hJoyReleased = 0;
			hram.hJoyHeld = 0;
			this->parent->joypad();
			held = hram.hJoyHeld;
			if (check_flag(held, input_a))
				return MainMenuResult::ContinueGame;
		}while (!check_flag(held, input_b));
	}
}
