#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/audio.h"
#include "../../../CodeGeneration/output/variables.h"
#include "../../../CodeGeneration/output/actors.h"
#include <CppRed/Npc.h>

namespace CppRed{
namespace Scripts{

using script_f = ScriptStore::script_f;

DECLARE_SCRIPT(OaksLabScript00);
DECLARE_SCRIPT(OaksLabScript06);
DECLARE_SCRIPT(OaksLabScript07);
DECLARE_SCRIPT(OaksLabScript08);
DECLARE_SCRIPT(OaksLabScript09);
DECLARE_SCRIPT(OaksLabScript10);
DECLARE_SCRIPT(OaksLabScript11);
DECLARE_SCRIPT(OaksLabScript12);
DECLARE_SCRIPT(OaksLabScript13);
DECLARE_SCRIPT(OaksLabScript14);
DECLARE_SCRIPT(OaksLabScript15);
DECLARE_SCRIPT(OaksLabScript16);
DECLARE_SCRIPT(OaksLabScript17);
DECLARE_SCRIPT(OaksLabScript18);

DECLARE_SCRIPT(OaksLabScript){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_pallet_after_getting_pokeballs_2)){
		//Apparently event_pallet_after_getting_pokeballs_2 is never set, so this should never happen.
		throw std::runtime_error("Not implemented.");
	}
	auto index = vs.get(IntegerVariableId::OaksLabScriptIndex);
	static const script_f scripts[] = {
		OaksLabScript00,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		OaksLabScript06,
		//OaksLabScript07,
		//OaksLabScript08,
		//OaksLabScript09,
		//OaksLabScript10,
		//OaksLabScript11,
		//OaksLabScript12,
		//OaksLabScript13,
		//OaksLabScript14,
		//OaksLabScript15,
		//OaksLabScript16,
		//OaksLabScript17,
		//OaksLabScript18,
	};
	if (index >= 0 && index < array_length(scripts))
		scripts[index](parameters);
}

DECLARE_SCRIPT(OaksLabScript00){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (!vs.get(EventId::event_oak_appeared_in_pallet))
		return;
	auto &world = game.get_world();

	{
		auto &oak = world.get_actor(ActorId::OakNearDoor);
		oak.set_visible(true);

		//Make Oak walk off-screen
		for (int i = 4; i--;)
			oak.move(FacingDirection::Up);

		oak.set_visible(false);
	}
	{
		auto &oak = world.get_actor(ActorId::OakInHisLab);
		oak.set_visible(true);
		vs.set(VisibilityFlagId::hs_oaks_lab_oak_1, true);
	}

	auto &player = world.get_pc();

	for (int i = 8; i--;)
		player.move(FacingDirection::Up);

	world.set_automatic_music_transition(true);
	world.play_current_map_music();
	vs.set(EventId::event_followed_oak_into_lab, true);
	vs.set(EventId::event_followed_oak_into_lab_2, true);

	auto &blue = dynamic_cast<NpcTrainer &>(world.get_actor(ActorId::BlueInOaksLab));
	blue.set_random_facing_direction(false);
	blue.set_facing_direction(FacingDirection::Up);

	game.run_dialog(TextResourceId::OaksLabRivalWaitingText, TileRegion::Window, true);
	game.run_dialog(TextResourceId::OaksLabChooseMonText, TileRegion::Window, true);
	game.run_dialog(TextResourceId::OaksLabRivalInterjectionText, TileRegion::Window, true);
	game.run_dialog(TextResourceId::OaksLabBePatientText, TileRegion::Window, true);
	game.get_engine().get_renderer().set_enable_window(false);
	game.reset_dialog_state();

	vs.set(EventId::event_oak_asked_to_choose_mon, true);

	player.set_ignore_input(false);

	vs.set(IntegerVariableId::OaksLabScriptIndex, 6);
}

DECLARE_SCRIPT(OaksLabScript06){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	vs.set(IntegerVariableId::OaksLabScriptIndex, -1);
}

}
}
