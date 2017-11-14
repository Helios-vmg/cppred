#pragma once

class Engine;

namespace CppRed{
class Game;
namespace Scripts{

enum class MainMenuResult{
	GoToTitleScreen,
	ContinueGame,
	NewGame,
};

MainMenuResult main_menu(Game &game);

}
}
