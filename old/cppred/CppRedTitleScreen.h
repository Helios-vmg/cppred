#pragma once
#include <utility>
#include "CppRedConstants.h"

struct TitleScrollData;
class CppRed;
class WRam;
class HRam;

enum class TitleScreenResult{
	GoToMainMenu,
	GoToClearSaveDialog,
};

class CppRedTitleScreen{
	CppRed *parent;
	WRam &wram;
	HRam &hram;

	void copy_pokemon_logo_to_wram_tilemap();
	void copy_copyright_to_wram_tilemap();
	void load_mon_sprite(SpeciesId);
	void copy_tilemap_to_vram(unsigned destination_page);
	void bounce_logo();
	void scroll_in_game_version();
	void print_game_version();
	void scroll_game_version(unsigned h, unsigned l);
	void scroll_logo(const std::pair<int, int> &amount, unsigned &y_scroll);
	void mon_scroll_loop();
	void scroll_in_mon();
	void title_scroll(bool out);
	void title_scroll2(const TitleScrollData *data, size_t data_length, unsigned d, int e);
	void animate_ball_if_starter_out();
	void pick_new_mon();
	//Loads the PC sprite to tile VRAM.
	void draw_player_character();
	void load_copyright_graphics();
	void load_gamefreak_logo();
	void load_pokemon_logo();
	void load_version_graphics();
public:
	CppRedTitleScreen(CppRed &parent);
	TitleScreenResult display();
};
