#pragma once

class CppRed;

enum class MainMenuResult{
	Cancelled,
	NewGame,
	ContinueGame,
};

class CppRedMainMenu{
	CppRed *parent;

	bool check_for_player_name_in_sram();
	void load_save();
	void display_options_menu();
	void display_continue_game_info();
public:
	CppRedMainMenu(CppRed &);
	MainMenuResult display();
};
