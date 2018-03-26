#include "EntryPoint.h"
#include "Engine.h"
#include "Renderer.h"
#include "Game.h"
#include "Intro.h"
#include "TitleScreen.h"
#include "MainMenu.h"
#include "ClearSave.h"
#include "OakSpeech.h"
#include "Maps.h"
#include "World.h"
#include "PlayerCharacter.h"
#include "../utility.h"
#include <cassert>

namespace CppRed{
namespace Scripts{

//#undef CPPRED_TESTING

static MainMenuResult initial_sequence(Game &game){
#ifndef CPPRED_TESTING
	auto &engine = game.get_engine();
	while (true){
		intro(game);
		while (title_screen(game) == TitleScreenResult::GoToMainMenu){
			auto main_menu_result = main_menu(game);
			if (main_menu_result != MainMenuResult::GoToTitleScreen)
				return main_menu_result;
		}
		//TODO
		//assert(title_screen_result == TitleScreenResult::GoToClearSaveDialog);
		//clear_save_dialog(game);
	}
#else
	return MainMenuResult::NewGame;
#endif
}

void entry_point(Game &game){
#ifndef CPPRED_TESTING
	if (initial_sequence(game) == MainMenuResult::ContinueGame){
#else
	if (false){
#endif
		//Continue game.
	}else{
		auto names = oak_speech(game);
		game.create_main_characters(names.player_name, names.rival_name);
		game.get_variable_store().load_initial_visibility_flags();
		auto &player = game.get_world().get_pc();
		player.set_facing_direction(FacingDirection::Up);
		player.give_item(ItemId::MasterBall, 39);
		player.give_item(ItemId::Hm04, 15);
		player.give_item(ItemId::Bicycle, 1);
		player.give_item(ItemId::CardKey, 42);
		game.teleport_player({Map::RedsHouse2F, Point(3, 6)});
		game.game_loop();
		assert(false);
	}
}

}
}
