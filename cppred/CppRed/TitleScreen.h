#pragma once

class Engine;

namespace CppRed{
class Game;
namespace Scripts{

enum class TitleScreenResult{
	GoToMainMenu,
	GoToClearSaveDialog,
};

TitleScreenResult title_screen(Game &game);

}
}
