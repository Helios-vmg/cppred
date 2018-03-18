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

	game.run_dialog_from_script(TextResourceId::OaksLabRivalWaitingText);
	game.reset_dialog_state();
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialog_from_script(TextResourceId::OaksLabChooseMonText);
	game.reset_dialog_state();
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialog_from_script(TextResourceId::OaksLabRivalInterjectionText);
	game.reset_dialog_state();
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialog_from_script(TextResourceId::OaksLabBePatientText);
	game.reset_dialog_state();

	vs.set(EventId::event_oak_asked_to_choose_mon, true);

	player.set_ignore_input(false);
	blue.set_random_facing_direction(true);

	vs.set(IntegerVariableId::OaksLabScriptIndex, 6);
}

DECLARE_SCRIPT(OaksLabScript06){
	auto &game = *parameters.game;
	auto &world = game.get_world();
	auto &player = world.get_pc();
	if (player.get_map_position().y < 6)
		return;
	auto &blue = dynamic_cast<NpcTrainer &>(world.get_actor(ActorId::BlueInOaksLab));
	blue.set_random_facing_direction(false);
	blue.set_facing_direction(FacingDirection::Down);
	game.run_dialog_from_script(TextResourceId::OaksLabLeavingText);
	game.reset_dialog_state();
	player.move(FacingDirection::Up);
	blue.set_random_facing_direction(true);
	return;


	auto &vs = game.get_variable_store();
	vs.set(IntegerVariableId::OaksLabScriptIndex, -1);
}

DECLARE_SCRIPT(OaksLabText1){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	TextResourceId text;
	if (!vs.get(EventId::event_followed_oak_into_lab_2))
		text = TextResourceId::OaksLabGaryText1;
	else if (vs.get(EventId::event_got_starter))
		text = TextResourceId::OaksLabText41;
	else
		text = TextResourceId::OaksLabText40;
	game.run_dialog_from_script(text);
	game.reset_dialog_state();
}

DECLARE_SCRIPT(OaksLabText4){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_pallet_after_getting_pokeballs) || vs.get(EventId::event_got_pokedex)){
		game.run_dialog_from_script(TextResourceId::OaksLabText_1d31d);
		//TODO: DisplayDexRating
	}else{
		auto &world = game.get_world();
		auto &player = world.get_pc();
		if (player.has_item_in_inventory(ItemId::PokeBall)){
			//I don't think this will ever execute.
			game.run_dialog_from_script(TextResourceId::OaksLabPleaseVisitText);
		}else if (vs.get(EventId::event_beat_route22_rival_1st_battle)){
			bool got = vs.get(EventId::event_got_pokeballs_from_oak);
			if (got)
				game.run_dialog_from_script(TextResourceId::OaksLabPleaseVisitText);
			else{
				vs.set(EventId::event_got_pokeballs_from_oak, true);
				player.receive(ItemId::PokeBall, 5);
				game.run_dialog_from_script(TextResourceId::OaksLabGivePokeballsText1);
				game.get_audio_interface().play_sound(AudioResourceId::SFX_Get_Key_Item);
				game.run_dialog_from_script(TextResourceId::OaksLabGivePokeballsText2);
			}
		}else if (vs.get(EventId::event_got_pokedex)){
			game.run_dialog_from_script(TextResourceId::OaksLabAroundWorldText);
		}else if (vs.get(EventId::event_battled_rival_in_oaks_lab)){
			if (player.has_item_in_inventory(ItemId::OaksParcel)){
				game.run_dialog_from_script(TextResourceId::OaksLabDeliverParcelText1);
				game.get_audio_interface().play_sound(AudioResourceId::SFX_Get_Key_Item);
				game.run_dialog_from_script(TextResourceId::OaksLabDeliverParcelText2);
				player.remove_all(ItemId::OaksParcel);
				vs.set(IntegerVariableId::OaksLabScriptIndex, 15);
			}else
				game.run_dialog_from_script(TextResourceId::OaksLabText_1d2fa);
		}else{
			if (vs.get(IntegerVariableId::event_received_starter))
				game.run_dialog_from_script(TextResourceId::OaksLabText_1d2f5);
			else
				game.run_dialog_from_script(TextResourceId::OaksLabText_1d2f0);
		}
	}
	game.reset_dialog_state();
}

}
}
