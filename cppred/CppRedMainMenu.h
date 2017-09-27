#pragma once

class Engine;
class CppRedEngine;

namespace CppRedScripts{

enum class MainMenuResult{
	GoToTitleScreen,
	ContinueGame,
	NewGame,
};

MainMenuResult main_menu(CppRedEngine &cppred);

}
