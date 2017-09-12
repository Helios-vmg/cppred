#pragma once
#include "CppRedRam.h"

class CppRed;

enum class MainMenuResult{
	Cancelled,
	NewGame,
	ContinueGame,
};

class CppRedMainMenu{
	CppRed *parent;

	typedef decltype(WRam::wTileMap)::iterator tilemap_it;

	bool check_for_player_name_in_sram();
	void display_options_menu();
	void display_continue_game_info();
	void print_num_badges(const tilemap_it &);
	void print_num_owned_mons(const tilemap_it &);
	void print_play_time(const tilemap_it &);
public:
	CppRedMainMenu(CppRed &);
	MainMenuResult display();
};
