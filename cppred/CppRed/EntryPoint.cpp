#include "EntryPoint.h"
#include "Engine.h"
#include "Renderer.h"
#include "Game.h"
#include "Intro.h"
#include "TitleScreen.h"
#include "MainMenu.h"
#include "ClearSave.h"
#include "OakSpeech.h"

namespace CppRedScripts{

static MainMenuResult initial_sequence(CppRedGame &cppred){
	auto &engine = cppred.get_engine();
	while (true){
		intro(cppred);
		while (title_screen(cppred) == TitleScreenResult::GoToMainMenu){
			auto main_menu_result = main_menu(cppred);
			if (main_menu_result != MainMenuResult::GoToTitleScreen)
				return main_menu_result;
		}
		//TODO
		//assert(title_screen_result == TitleScreenResult::GoToClearSaveDialog);
		//clear_save_dialog(cppred);
	}
}

void entry_point(Engine &engine, PokemonVersion version, CppRedAudioProgram &program){
	CppRedGame cppred(engine, version, program);
	if (initial_sequence(cppred) == MainMenuResult::ContinueGame){
		//Continue game.
	}else{
		auto names = oak_speech(cppred);
		cppred.create_main_characters(names.player_name, names.rival_name);
		cppred.game_loop();
		assert(false);
	}
}

}
