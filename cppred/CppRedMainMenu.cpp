#include "CppRedMainMenu.h"
#include "CppRed.h"
#include "CppRedSRam.h"
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
	this->parent->hram.H_AUTOBGTRANSFERENABLED = 0;
	auto &text = this->parent->text;
	text.text_box_border(this->parent->get_tilemap_location(4, 7), 14, 8);
	text.place_string(this->parent->get_tilemap_location(5, 9), text.SaveScreenInfoText);
	text.place_string(this->parent->get_tilemap_location(12, 9), this->parent->wram.wPlayerName.to_string());
	this->print_num_badges(this->parent->get_tilemap_location(18, 11));
	this->print_num_owned_mons(this->parent->get_tilemap_location(16, 13));
	this->print_play_time(this->parent->get_tilemap_location(13, 15));
	this->parent->hram.H_AUTOBGTRANSFERENABLED = 1;
	this->parent->delay_frames(30);
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
