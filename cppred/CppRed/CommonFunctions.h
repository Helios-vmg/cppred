#pragma once

class Engine;

namespace CppRed{
class PlayerCharacter;
class Game;
class Pokemon;
namespace Scripts{

bool standard_add_pokemon(Game &game, PlayerCharacter &player, const Pokemon &pokemon);

}
}
