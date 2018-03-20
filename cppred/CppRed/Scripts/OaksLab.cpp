#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/audio.h"
#include "../../../CodeGeneration/output/variables.h"
#include "../../../CodeGeneration/output/actors.h"
#include <CppRed/Npc.h>
#include <sstream>

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

	game.run_dialogue_from_script(TextResourceId::OaksLabRivalWaitingText);
	game.reset_dialogue_state();
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialogue_from_script(TextResourceId::OaksLabChooseMonText);
	game.reset_dialogue_state();
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialogue_from_script(TextResourceId::OaksLabRivalInterjectionText);
	game.reset_dialogue_state();
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialogue_from_script(TextResourceId::OaksLabBePatientText);
	game.reset_dialogue_state();

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
	if (player.is_moving())
		player.abort_movement();
	auto &blue = dynamic_cast<NpcTrainer &>(world.get_actor(ActorId::BlueInOaksLab));
	blue.set_random_facing_direction(false);
	blue.set_facing_direction(FacingDirection::Down);
	player.set_ignore_input(true);
	game.run_dialogue_from_script(TextResourceId::OaksLabLeavingText);
	game.reset_dialogue_state();
	player.move(FacingDirection::Up);
	blue.set_random_facing_direction(true);
	player.set_ignore_input(false);
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
	game.run_dialogue_from_world(text, *parameters.caller);
}

static SpeciesId starter_index_to_species_id(int starter_index){
	const BasePokemonInfo *pokemon = nullptr;
	for (auto &p : pokemon_by_pokedex_id){
		if (p->starter_index == starter_index){
			pokemon = p;
			break;
		}
	}
	if (!pokemon){
		std::stringstream stream;
		stream << "Invalid started index: " << starter_index;
		throw std::runtime_error(stream.str());
	}
	return pokemon->species_id;
}

static void display_dex(const script_parameters &parameters, int starter_index){
	
}

struct BallScriptData{
	int starter_index;
	int blues_starter_index;
	int blues_ball_index;
	TextResourceId question;
	VisibilityFlagId vfi;
	ActorId reds_ball;
};

static void ball_script(const script_parameters &parameters, int index){
	static const BallScriptData static_data[] = {
		{ 1, 2, 1, TextResourceId::OaksLabCharmanderText, VisibilityFlagId::hs_starter_ball_1, ActorId::CharmanderPokeball },
		{ 2, 0, 2, TextResourceId::OaksLabSquirtleText,   VisibilityFlagId::hs_starter_ball_2, ActorId::SquirtlePokeball   },
		{ 0, 1, 0, TextResourceId::OaksLabBulbasaurText,  VisibilityFlagId::hs_starter_ball_3, ActorId::BulbasaurPokeball  },
	};
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_got_starter)){
		game.run_dialogue_from_script(TextResourceId::OaksLabLastMonText);
		game.reset_dialogue_state();
		return;
	}
	if (!vs.get(EventId::event_oak_asked_to_choose_mon)){
		game.run_dialogue_from_script(TextResourceId::OaksLabText39);
		game.reset_dialogue_state();
		return;
	}

	auto &data = static_data[index];
	auto reds_species = starter_index_to_species_id(data.starter_index);
	auto &reds_pokemon = *pokemon_by_species_id[(int)reds_species];
	parameters.game->display_pokedex_page(reds_pokemon.pokedex_id, *parameters.caller);
	game.run_dialogue_from_script(data.question, false);
	bool answer = game.run_yes_no_menu({14, 7});
	if (!answer){
		game.reset_dialogue_state();
		return;
	}
	auto blues_species = starter_index_to_species_id(data.blues_starter_index);
	vs.set(IntegerVariableId::wPlayerStarter, (int)reds_species);
	vs.set(data.vfi, false);
	vs.set(StringVariableId::wcd6d_ReceivedItemName, reds_pokemon.display_name);
	auto &world = game.get_world();
	world.get_actor(data.reds_ball).set_visible(false);

	game.run_dialogue_from_script(TextResourceId::OaksLabMonEnergeticText, false);
	game.run_dialogue_from_script(TextResourceId::OaksLabReceivedMonText, false);
	auto &audio = game.get_audio_interface();
	audio.play_sound(AudioResourceId::SFX_Get_Key_Item);
	audio.wait_for_sfx_to_end();
	auto &player = world.get_pc();
	player.get_party().add_pokemon(reds_pokemon.species_id, 5, player.get_trainer_id(), game.get_engine().get_prng());
	game.reset_dialogue_state();
}

DECLARE_SCRIPT(OaksLabText2){
	ball_script(parameters, 0);
}

DECLARE_SCRIPT(OaksLabText3){
	ball_script(parameters, 1);
}

DECLARE_SCRIPT(OaksLabText4){
	ball_script(parameters, 2);
}

DECLARE_SCRIPT(OaksLabText5){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_pallet_after_getting_pokeballs) || vs.get(EventId::event_got_pokedex)){
		game.run_dialogue_from_world(TextResourceId::OaksLabText_1d31d, *parameters.caller);
		//TODO: DisplayDexRating
	}else{
		auto &world = game.get_world();
		auto &player = world.get_pc();
		if (player.has_item_in_inventory(ItemId::PokeBall)){
			//I don't think this will ever execute.
			game.run_dialogue_from_world(TextResourceId::OaksLabPleaseVisitText, *parameters.caller);
		}else if (vs.get(EventId::event_beat_route22_rival_1st_battle)){
			bool got = vs.get(EventId::event_got_pokeballs_from_oak);
			if (got)
				game.run_dialogue_from_world(TextResourceId::OaksLabPleaseVisitText, *parameters.caller);
			else{
				vs.set(EventId::event_got_pokeballs_from_oak, true);
				player.receive(ItemId::PokeBall, 5);
				game.run_dialogue_from_world(TextResourceId::OaksLabGivePokeballsText1, *parameters.caller);
				game.get_audio_interface().play_sound(AudioResourceId::SFX_Get_Key_Item);
				game.run_dialogue_from_world(TextResourceId::OaksLabGivePokeballsText2, *parameters.caller);
			}
		}else if (vs.get(EventId::event_got_pokedex)){
			game.run_dialogue_from_world(TextResourceId::OaksLabAroundWorldText, *parameters.caller);
		}else if (vs.get(EventId::event_battled_rival_in_oaks_lab)){
			if (player.has_item_in_inventory(ItemId::OaksParcel)){
				game.run_dialogue_from_world(TextResourceId::OaksLabDeliverParcelText1, *parameters.caller);
				game.get_audio_interface().play_sound(AudioResourceId::SFX_Get_Key_Item);
				game.run_dialogue_from_world(TextResourceId::OaksLabDeliverParcelText2, *parameters.caller);
				player.remove_all(ItemId::OaksParcel);
				vs.set(IntegerVariableId::OaksLabScriptIndex, 15);
			}else
				game.run_dialogue_from_world(TextResourceId::OaksLabText_1d2fa, *parameters.caller);
		}else{
			if (vs.get(IntegerVariableId::event_received_starter))
				game.run_dialogue_from_world(TextResourceId::OaksLabText_1d2f5, *parameters.caller);
			else
				game.run_dialogue_from_world(TextResourceId::OaksLabText_1d2f0, *parameters.caller);
		}
	}
	game.reset_dialogue_state();
}

}
}
