#pragma once
#include <functional>
#include <map>
#include "CppRedConstants.h"
#include "CppRedRam.h"
#include "CppRedText.h"

class CppRed;

class CppRedMessageBox{
	typedef decltype(WRam::wTileMap)::iterator tilemap_it;
	typedef std::function<int(TextBoxId, const tilemap_it &, unsigned, unsigned)> callback_t;
	
	struct TwoOptionMenuData{
		CppRedText::Region *text;
		unsigned width, height;
		bool first_line_is_blank;
		bool special_border;
		bool ignore_b_button;
	};
	
	CppRed *parent;
	std::map<TextBoxId, callback_t> callbacks;
	std::map<TwoOptionMenuType, TwoOptionMenuData> two_option_menus;


	void initialize1();
	void initialize2();
	void initialize3();
	void initialize4();
	int display_two_option_menu(const tilemap_it &, unsigned x, unsigned y);
	int display_money_box(const tilemap_it &location, unsigned text_x, unsigned text_y);
	int display_buy_sell_quit_menu(const tilemap_it &location, unsigned text_x, unsigned text_y);
	int display_field_move_mon_menu();
	TwoOptionMenuData get_data(TwoOptionMenuType);
	void save_screen_tiles(const tilemap_it &location, unsigned w, unsigned h);
	void restore_screen_tiles(const tilemap_it &location, unsigned w, unsigned h);
	unsigned get_mon_field_moves();
public:
	CppRedMessageBox(CppRed &parent);
	void display_textbox_id(unsigned x, unsigned y);
	void display_textbox_id(const tilemap_it &location, unsigned text_x, unsigned text_y);
};
