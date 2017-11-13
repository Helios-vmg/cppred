#pragma once

class Engine;
class CppRedGame;

namespace CppRedScripts{

enum class MainMenuResult{
	GoToTitleScreen,
	ContinueGame,
	NewGame,
};

MainMenuResult main_menu(CppRedGame &cppred);

}
