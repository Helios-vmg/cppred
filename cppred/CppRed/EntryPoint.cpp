#include "EntryPoint.h"
#include "Engine.h"
#include "Renderer.h"
#include "Game.h"
#include "Intro.h"
#include "TitleScreen.h"
#include "MainMenu.h"
#include "ClearSave.h"
#include "OakSpeech.h"

namespace CppRed{
namespace Scripts{

static MainMenuResult initial_sequence(Game &game){
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
}

void entry_point(Engine &engine, PokemonVersion version, CppRed::AudioProgram &program){
	Game game(engine, version, program);
	if (initial_sequence(game) == MainMenuResult::ContinueGame){
		//Continue game.
	}else{
		auto names = oak_speech(game);
		game.create_main_characters(names.player_name, names.rival_name);
		game.game_loop();
		assert(false);
	}
}

}
}
