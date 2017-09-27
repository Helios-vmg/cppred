#pragma once

class Engine;
class CppRedEngine;

namespace CppRedScripts{

enum class TitleScreenResult{
	GoToMainMenu,
	GoToClearSaveDialog,
};

TitleScreenResult title_screen(CppRedEngine &cppred);

}
