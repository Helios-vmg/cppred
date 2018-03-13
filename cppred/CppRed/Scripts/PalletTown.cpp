#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/audio.h"
#include "Events.h"

namespace CppRed{
namespace Scripts{

static const char * const PalletTownScriptIndex = "PalletTownScriptIndex";
static const char * const wcf0d = "wcf0d";
using script_f = ScriptStore::script_f;

DECLARE_SCRIPT(PalletTownScript0);
DECLARE_SCRIPT(PalletTownScript1);
DECLARE_SCRIPT(PalletTownScript2);
DECLARE_SCRIPT(PalletTownScript3);
DECLARE_SCRIPT(PalletTownScript4);
DECLARE_SCRIPT(PalletTownScript5);
DECLARE_SCRIPT(PalletTownScript6);

DECLARE_SCRIPT(PalletTown_onload){
	auto &vs = parameters.game->get_variable_store();
	auto &oak = parameters.game->get_world().get_actor("PalletOak");
	oak.set_visible(false);
}

DECLARE_SCRIPT(PalletTownScript){
	auto &vs = parameters.game->get_variable_store();
	if (vs.get_number_default(event_got_pokeballs_from_oak))
		vs.set_number(event_pallet_after_getting_pokeballs, 1);
	auto index = vs.get_number_default(PalletTownScriptIndex);
	static const script_f scripts[] = {
		PalletTownScript0,
		PalletTownScript1,
		PalletTownScript2,
		PalletTownScript3,
		PalletTownScript4,
		PalletTownScript5,
		PalletTownScript6,
	};
	if (index >= 0 && index < array_length(scripts))
		scripts[index](parameters);
}

DECLARE_SCRIPT(PalletTownScript0){
	auto &vs = parameters.game->get_variable_store();
	if (vs.get_number_default(event_followed_oak_into_lab))
		return;
	auto &player = parameters.game->get_world().get_pc();
	auto position = player.get_map_position();
	if (position.y > 1)
		return;
	player.set_facing_direction(FacingDirection::Down);
	auto &ai = parameters.game->get_audio_interface();
	ai.play_sound(AudioResourceId::Stop);
	ai.play_sound(AudioResourceId::Music_MeetProfOak);
	vs.set_number(event_oak_appeared_in_pallet, 1);
	vs.set_number(PalletTownScriptIndex, 1);
	PalletTownScript1(parameters);
}

DECLARE_SCRIPT(PalletTownScript1){
	auto &vs = parameters.game->get_variable_store();
	vs.set_number(wcf0d, 0);
	auto &world = parameters.game->get_world();
	//auto &oak = world.get_actor("PalletOak");
	auto &player = world.get_pc();
	parameters.game->run_dialog_from_world(TextResourceId::OakAppearsText, player);
	vs.set_number(PalletTownScriptIndex, -1);
}

DECLARE_SCRIPT(PalletTownScript2){
}

DECLARE_SCRIPT(PalletTownScript3){
}

DECLARE_SCRIPT(PalletTownScript4){
}

DECLARE_SCRIPT(PalletTownScript5){
}

DECLARE_SCRIPT(PalletTownScript6){
}

}
}