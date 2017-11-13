#pragma once

class Engine;
class CppRedGame;

namespace CppRedScripts{

enum class TitleScreenResult{
	GoToMainMenu,
	GoToClearSaveDialog,
};

TitleScreenResult title_screen(CppRedGame &cppred);

}
