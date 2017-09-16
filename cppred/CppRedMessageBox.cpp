#include "CppRedMessageBox.h"
#include "CppRed.h"
#include "CppRedStructs.h"
#include "CppRedData.h"
#include <sstream>

CppRedMessageBox::CppRedMessageBox(CppRed &parent): parent(&parent){
	this->initialize1();
	this->initialize2();
	this->initialize3();
	this->initialize4();
}

void CppRedMessageBox::initialize1(){
	this->callbacks[TextBoxId::TwoOptionMenu]    = [this](TextBoxId, const tilemap_it &l, unsigned x, unsigned y){ return this->display_two_option_menu(l, x, y); };
	this->callbacks[TextBoxId::MoneyBox]         = [this](TextBoxId, const tilemap_it &l, unsigned x, unsigned y){ return this->display_money_box(l, x, y); };
	this->callbacks[TextBoxId::BuySellQuitMenu]  = [this](TextBoxId, const tilemap_it &l, unsigned x, unsigned y){ return this->display_buy_sell_quit_menu(l, x, y); };
	this->callbacks[TextBoxId::FieldMoveMonMenu] = [this](TextBoxId, const tilemap_it &, unsigned, unsigned){ return this->display_field_move_mon_menu(); };
}

void CppRedMessageBox::initialize2(){
	struct S{
		TextBoxId id;
		byte_t left;
		byte_t top;
		byte_t right;
		byte_t bottom;
	};

	static const S table[] = {
		{ TextBoxId::MessageBox,     0, 12, 19, 17 },
		{ TextBoxId::Unknown03,      0,  0, 19, 14 },
		{ TextBoxId::Unknown07,      0,  0, 11,  6 },
		{ TextBoxId::ListMenuBox,    4,  2, 19, 12 },
		{ TextBoxId::Unknown10,      7,  0, 19, 17 },
		{ TextBoxId::MonSpritePopup, 6,  4, 14, 13 },
	};
	for (auto &i : table){
		auto p = this->parent;
		byte_t x = i.left;
		byte_t y = i.top;
		byte_t w = i.right - i.left - 1;
		byte_t h = i.bottom - i.top - 1;
		this->callbacks[i.id] = [p, x, y, w, h](TextBoxId, const tilemap_it &l, unsigned, unsigned){
			auto corner = p->get_tilemap_location(x, y);
			p->text.text_box_border(corner, w, h);
			return -1;
		};
	}
}

void CppRedMessageBox::initialize3(){
	struct S{
		TextBoxId id;
		byte_t left;
		byte_t top;
		byte_t right;
		byte_t bottom;
		byte_t text_x;
		byte_t text_y;
		CppRedText::Region *text;
	};

	auto &text = this->parent->text;

	const S table[] = {
		{ TextBoxId::JpMochimonoMenuTemplate,        0,  0, 14, 17,  3,  0, nullptr },
		{ TextBoxId::UseTossMenuTemplate,           13, 10, 19, 14, 15, 11, &text.UseTossText },
		{ TextBoxId::JpSaveMessageMenuTemplate,      0,  0,  7,  5,  2,  2, nullptr },
		{ TextBoxId::JpSpeedOptionsMenuTemplate,     0,  6,  5, 10,  2,  7, nullptr },
		{ TextBoxId::BattleMenuTemplate,             8, 12, 19, 17, 10, 14, &text.BattleMenuText },
		{ TextBoxId::SafariBattleMenuTemplate,       0, 12, 19, 17,  2, 14, &text.SafariZoneBattleMenuText },
		{ TextBoxId::SwitchStatsCancelMenuTemplate,  0, 12, 19, 17,  2, 14, &text.SwitchStatsCancelText },
		{ TextBoxId::BuySellQuitMenuTemplate,        0,  0, 10,  6,  2,  1, &text.BuySellQuitText },
		{ TextBoxId::MoneyBoxTemplate,              11,  0, 19,  2, 13,  0, &text.MoneyText },
		{ TextBoxId::JpAhMenuTemplate,               7,  6, 11, 10,  8,  8, nullptr },
		{ TextBoxId::JpPokedexMenuTemplate,         11,  8, 19, 17, 12, 10, nullptr },
	};
	for (auto &i : table){
		auto p = this->parent;
		byte_t x1 = i.left;
		byte_t y1 = i.top;
		byte_t w = i.right - i.left - 1;
		byte_t h = i.bottom - i.top - 1;
		byte_t x2 = i.text_x;
		byte_t y2 = i.text_y;
		auto region = i.text;
		auto id = i.id;
		this->callbacks[id] = [p, x1, y1, w, h, x2, y2, region, id](TextBoxId, const tilemap_it &l, unsigned, unsigned){
			if (!region){
				std::stringstream stream;
				stream << "Attempt to draw unknown textbox: " << (unsigned)id;
				throw std::runtime_error(stream.str());
			}
			auto corner = p->get_tilemap_location(x1, y1);
			p->text.text_box_border(corner, w, h);
			auto old = p->wram.wMainData.wd730.get_raw_value();
			p->wram.wMainData.wd730.set_no_print_delay(true);
			corner = p->get_tilemap_location(x2, y2);
			p->text.place_string(corner, *region);
			p->wram.wMainData.wd730.set_raw_value(old);
			p->update_sprites();
			return -1;
		};
	}
}

void CppRedMessageBox::initialize4(){
	auto &text = this->parent->text;
	this->two_option_menus[TwoOptionMenuType::YesNo]       = { &text.YesNoMenu,       4, 3, false, false, false, };
	this->two_option_menus[TwoOptionMenuType::NorthWest]   = { &text.NorthWestMenu,   6, 3, false, false, false, };
	this->two_option_menus[TwoOptionMenuType::SouthEast]   = { &text.SouthEastMenu,   6, 3, false, false, false, };
	this->two_option_menus[TwoOptionMenuType::WideYesNo]   = { &text.YesNoMenu,       6, 3, false, false, false, };
	this->two_option_menus[TwoOptionMenuType::NorthEast]   = { &text.NorthEastMenu,   6, 3, false, false, false, };
	this->two_option_menus[TwoOptionMenuType::TradeCancel] = { &text.TradeCancelMenu, 7, 3, false, true,  false, };
	this->two_option_menus[TwoOptionMenuType::HealCancel]  = { &text.HealCancelMenu,  7, 4, true,  false, false, };
	this->two_option_menus[TwoOptionMenuType::NoYes]       = { &text.NoYesMenu,       4, 3, false, false, true,  };
}

void CppRedMessageBox::display_textbox_id(unsigned x, unsigned y){
	auto location = this->parent->get_tilemap_location(x, y);
	this->display_textbox_id(location, x + 1, y + 1);
}

void CppRedMessageBox::display_textbox_id(const tilemap_it &location, unsigned text_x, unsigned text_y){
	auto id = this->parent->wram.wTextBoxID.enum_value();
	auto it = this->callbacks.find(id);
	if (it == this->callbacks.end()){
		std::stringstream stream;
		stream << "CppRedMessageBox::display_textbox_id(): unknown textbox ID: " << (unsigned)id;
		throw std::runtime_error(stream.str());
	}
	it->second(id, location, text_x, text_y);
}

CppRedMessageBox::TwoOptionMenuData CppRedMessageBox::get_data(TwoOptionMenuType type){
	auto it = this->two_option_menus.find(type);
	if (it == this->two_option_menus.end()){
		std::stringstream stream;
		stream << "CppRedMessageBox::get_data(): Unknown menu type: " << (unsigned)type;
		throw std::runtime_error(stream.str());
	}
	return it->second;
}

void CppRedMessageBox::save_screen_tiles(const tilemap_it &src, unsigned w, unsigned h){
	auto &dst = this->parent->wram.wBuffer;
	int dst_index = 0;
	for (int y = 0; y < 5; y++)
		for (int x = 0; x < 6; x++)
			dst[dst_index++] = +src[x + y * tilemap_width];
}

void CppRedMessageBox::restore_screen_tiles(const tilemap_it &dst, unsigned w, unsigned h){
	auto &src = this->parent->wram.wBuffer;
	int dst_index = 0;
	for (int y = 0; y < 5; y++)
		for (int x = 0; x < 6; x++)
			dst[x + y * tilemap_width] = +src[dst_index++];
}

int CppRedMessageBox::display_two_option_menu(const tilemap_it &location, unsigned text_x, unsigned text_y){
	auto &wram = this->parent->wram;
	wram.wMainData.wd730.set_no_print_delay(true);
	wram.wMenuWatchedKeys.clear();
	wram.wMenuWatchedKeys.set_button_a(true);
	wram.wMenuWatchedKeys.set_button_b(true);
	wram.wMaxMenuItem = 1;
	wram.wTopMenuItemX = text_x;
	wram.wTopMenuItemY = text_y;
	wram.wLastMenuItem = 0;
	wram.wMenuWatchMovingOutOfBounds = 0;
	auto menu_type = wram.wTwoOptionMenuID.get_id();
	auto select_second_item = wram.wTwoOptionMenuID.get_select_second_item_by_default();
	wram.wTwoOptionMenuID.set_select_second_item_by_default(false);
	wram.wCurrentMenuItem = (int)select_second_item;

	auto data = this->get_data(menu_type);
#ifndef BUG_FIX
	this->save_screen_tiles(location, 6, 5);
#else
	this->save_screen_tiles(location, data.width, data.height);
#endif
	
	auto &tiles = data.special_border ? CppRedText::trade_center_border_tiles : CppRedText::default_border_tiles;
	this->parent->text.text_box_border(location, data.width, data.height, tiles);

	this->parent->update_sprites();

	auto text_location = this->parent->get_tilemap_location(text_x + 1, text_y + data.first_line_is_blank);
	this->parent->text.place_string(text_location, *data.text);

	this->parent->wram.wMainData.wd730.set_no_print_delay(false);

	int selection = -1;
	wram.wTwoOptionMenuID.clear();
	if (data.ignore_b_button){
		auto old_value = wram.wFlags_0xcd60.get_raw_value();
		wram.wFlags_0xcd60.set_no_menu_sound(true);
		InputBitmap_struct input;
		do
			input = this->parent->handle_menu_input();
		while (!input.button_a);
		selection = wram.wCurrentMenuItem;
		wram.wFlags_0xcd60.set_raw_value(old_value);
		this->parent->play_sound(Sound::SFX_Press_AB_1);
	}else{
		auto input = this->parent->handle_menu_input();
		selection = input.button_b ? 1 : wram.wCurrentMenuItem;
	}
	assert(selection >= 0);
	wram.wChosenMenuItem = selection;

	wram.wMenuExitMethod = !selection ? MenuExitMethod::SelectedFirstOption : MenuExitMethod::SelectedSecondOption;
	this->parent->delay_frames(15);

#ifndef BUG_FIX
	this->restore_screen_tiles(location, 6, 5);
#else
	this->restore_screen_tiles(location, data.width, data.height);
#endif

	return selection;
}

int CppRedMessageBox::display_money_box(const tilemap_it &location, unsigned text_x, unsigned text_y){
	auto &parent = *this->parent;
	auto &wram = parent.wram;

	wram.wMainData.wd730.set_no_print_delay(true);
	wram.wTextBoxID = TextBoxId::MoneyBoxTemplate;
	this->display_textbox_id(location, text_x, text_y);
	parent.clear_screen_area(6, 1, parent.get_tilemap_location(13, 1));
	auto money = +wram.wMainData.wPlayerMoney;
	std::stringstream stream;
	stream << money;
	parent.text.place_string(parent.get_tilemap_location(12, 1), stream.str());
	wram.wMainData.wd730.set_no_print_delay(false);
	return -1;
}

int CppRedMessageBox::display_buy_sell_quit_menu(const tilemap_it &location, unsigned text_x, unsigned text_y){
	auto &parent = *this->parent;
	auto &wram = parent.wram;

	wram.wMainData.wd730.set_no_print_delay(true);
	wram.wChosenMenuItem = 0;
	wram.wTextBoxID = TextBoxId::BuySellQuitMenuTemplate;
	this->display_textbox_id(location, text_x, text_y);
	wram.wMenuWatchedKeys.clear();
	wram.wMenuWatchedKeys.set_button_a(true);
	wram.wMenuWatchedKeys.set_button_b(true);
	const int max_menu_item = 2;
	wram.wMaxMenuItem = max_menu_item;
	wram.wTopMenuItemX = 1;
	wram.wTopMenuItemY = 1;
	wram.wCurrentMenuItem = 0;
	wram.wLastMenuItem = 0;
	wram.wMenuWatchMovingOutOfBounds = 0;
	wram.wMainData.wd730.set_no_print_delay(false);
	auto input = parent.handle_menu_input();
	parent.place_unfilled_arrow_menu_cursor();
	int selection = -1;
	if (input.button_a || !input.button_b){
		selection = wram.wCurrentMenuItem;
		if (selection != max_menu_item){
			wram.wMenuExitMethod = MenuExitMethod::SelectedAnItem;
			wram.wChosenMenuItem = selection;
			return selection;
		}
	}
	wram.wMenuExitMethod = MenuExitMethod::MenuCancelled;
	selection = wram.wCurrentMenuItem;
	wram.wChosenMenuItem = selection;
	return -1;
}

int CppRedMessageBox::display_field_move_mon_menu(){
	auto &parent = *this->parent;
	auto &wram = parent.wram;

	std::fill(wram.wFieldMoves.begin(), wram.wFieldMoves.end(), MoveId::None);
	wram.wNumFieldMoves = 0;
	wram.wFieldMovesLeftmostXCoord = 12;
	auto n = this->get_mon_field_moves();;
	wram.wNumFieldMoves = n;
	if (!n){
		parent.text.text_box_border(parent.get_tilemap_location(11, 11), 7, 5);
		parent.update_sprites();
		parent.hram.hFieldMoveMonMenuTopMenuItemX = 12;
		parent.text.place_string(parent.get_tilemap_location(13, 12), parent.text.PokemonMenuEntries);
		return -1;
	}

	unsigned textbox_x = wram.wFieldMovesLeftmostXCoord - 1;
	unsigned textbox_width = (tilemap_width - 2) - textbox_x;
	unsigned textbox_height = (n + 3) * 2;
	unsigned textbox_y = (tilemap_height - 2) - textbox_height;
	auto textbox_location = parent.get_tilemap_location(textbox_x, textbox_y);
	parent.text.text_box_border(parent.get_tilemap_location(textbox_x, textbox_y), textbox_width, textbox_height);
	parent.update_sprites();

	unsigned print_location_y = textbox_y + 2;
	unsigned print_location_x = textbox_x + 2;
	for (unsigned i = 0; i < n; i++){
		auto move = wram.wFieldMoves[i].enum_value();
		auto &display_name = moves_by_move_id[(int)move]->display_name;
		parent.text.place_string(parent.get_tilemap_location(print_location_x, print_location_y), display_name);
		print_location_y += 2;
	}
	parent.text.place_string(parent.get_tilemap_location(print_location_x, print_location_y), parent.text.PokemonMenuEntries);
	return -1;
}

int get_leftmost_tile(const MoveInfo &info){
	return 19 - std::max(7, (int)info.display_name.size() + 1);
}

unsigned CppRedMessageBox::get_mon_field_moves(){
	auto &parent = *this->parent;
	auto &wram = parent.wram;

	const MoveInfo *field_moves[4];
	int field_moves_size = 0;
	auto moves = wram.wPartyData.wPartyMons[wram.wWhichPokemon].moves;
	int leftmost = std::numeric_limits<unsigned>::max();
	for (auto &move : moves){
		auto info = moves_by_move_id[(int)move];
		if (!info->field_move_index)
			continue;
		field_moves[field_moves_size] = info;
		wram.wFieldMoves[field_moves_size] = info->move_id;
		leftmost = std::min(leftmost, get_leftmost_tile(*info));
	}
	//std::sort(field_moves, field_moves + field_moves_size, [](const MoveInfo *a, const MoveInfo *b){ return a->field_move_index < b->field_move_index; });
	if (!field_moves_size)
		return 0;
	wram.wLastFieldMoveID = field_moves[field_moves_size - 1]->move_id;
	wram.wFieldMovesLeftmostXCoord = leftmost;
	return field_moves_size;
}
