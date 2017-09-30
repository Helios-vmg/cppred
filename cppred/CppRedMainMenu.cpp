#include "CppRedMainMenu.h"
#include "CppRedEngine.h"
#include "CppRedMiscClasses.h"
#include "Engine.h"
#include "Renderer.h"

static CppRedScripts::MainMenuResult no_save_file(CppRedEngine &cppred){
	//TODO
	return CppRedScripts::MainMenuResult::GoToTitleScreen;
}

static CppRedScripts::MainMenuResult valid_save_file(CppRedEngine &cppred, SavableData &save){
	//TODO
	return CppRedScripts::MainMenuResult::GoToTitleScreen;
}

static CppRedScripts::MainMenuResult corrupted_save_file(CppRedEngine &cppred){
	//TODO: Report corruption
	return no_save_file(cppred);
}

namespace CppRedScripts{

MainMenuResult main_menu(CppRedEngine &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	GameOptions options;
	bool options_initialized = false;
	auto save = cppred.load_save();

	renderer.set_default_palettes();


	if (!save)
		return no_save_file(cppred);
	if (save->valid)
		return valid_save_file(cppred, *save);
	return corrupted_save_file(cppred);
}

}
