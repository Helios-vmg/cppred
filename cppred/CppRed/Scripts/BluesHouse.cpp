#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/audio.h"
#include "../../../CodeGeneration/output/variables.h"
#include <CppRed/Npc.h>

namespace CppRed{
namespace Scripts{

using script_f = ScriptStore::script_f;

DECLARE_SCRIPT(BluesHouseScript0);
DECLARE_SCRIPT(BluesHouseScript1);

DECLARE_SCRIPT(BluesHouseScript){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_got_pokeballs_from_oak))
		vs.set(EventId::event_pallet_after_getting_pokeballs, true);
	auto index = vs.get(IntegerVariableId::BluesHouseScriptIndex);
	static const script_f scripts[] = {
		BluesHouseScript0,
		BluesHouseScript1,
	};
	if (index >= 0 && index < array_length(scripts))
		scripts[index](parameters);
}

DECLARE_SCRIPT(BluesHouseScript0){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	vs.set(EventId::event_entered_blues_house, true);
	vs.set(IntegerVariableId::BluesHouseScriptIndex, 1);
	BluesHouseScript1(parameters);
}

DECLARE_SCRIPT(BluesHouseScript1){
	return;
}

DECLARE_SCRIPT(BluesHouseText1){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_got_town_map)){
		parameters.game->run_dialog_from_world(TextResourceId::DaisyUseMapText, *parameters.caller);
	}else if (vs.get(EventId::event_got_pokedex)){
		parameters.game->run_dialog_from_world(TextResourceId::DaisyOfferMapText, *parameters.caller);
		//TODO: Give map.
		vs.set(VisibilityFlagId::hs_town_map, false);
		parameters.game->run_dialog_from_world(TextResourceId::GotMapText, *parameters.caller);
		vs.set(EventId::event_got_town_map, true);
	}else{
		parameters.game->run_dialog_from_world(TextResourceId::DaisyInitialText, *parameters.caller);
	}
}

}
}
