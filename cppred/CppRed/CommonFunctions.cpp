#include "CommonFunctions.h"
#include "Engine.h"
#include "Renderer.h"
#include "PlayerCharacter.h"
#include "Game.h"
#include "../../CodeGeneration/output/variables.h"

namespace CppRed{
namespace Scripts{

bool standard_add_pokemon(Game &game, PlayerCharacter &player, const Pokemon &pokemon){
	auto &party = player.get_party();
	bool success = party.add_pokemon(pokemon);
	if (!success){
		//TODO
	}
	auto &vs = game.get_variable_store();
	auto &data = *pokemon_by_species_id[(int)pokemon.get_species()];
	vs.set(StringVariableId::wcd6d_ActionTargetMonName, data.display_name);
	game.run_dialogue(TextResourceId::DoYouWantToNicknameText, false, false);
	if (game.run_yes_no_menu(standard_dialogue_yes_no_position)){
		auto name = game.get_name_from_user(pokemon.get_species());
		party.get_last_added_pokemon().set_nickname(name);
	}
	player.get_pokedex().set_owned(pokemon.get_species());
	return true;
}

}
}
