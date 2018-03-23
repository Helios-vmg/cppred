#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../CommonFunctions.h"
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
		nullptr,
		OaksLabScript08,
		OaksLabScript09,
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

	game.run_dialogue(TextResourceId::OaksLabRivalWaitingText, true, true);
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialogue(TextResourceId::OaksLabChooseMonText, true, true);
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialogue(TextResourceId::OaksLabRivalInterjectionText, true, true);
	Coroutine::get_current_coroutine().wait(0.63);
	game.run_dialogue(TextResourceId::OaksLabBePatientText, true, true);

	vs.set(EventId::event_oak_asked_to_choose_mon, true);

	player.set_ignore_input(false);
	blue.set_random_facing_direction(true);

	vs.set(IntegerVariableId::OaksLabScriptIndex, 6);
}

DECLARE_SCRIPT(OaksLabScript06){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
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
	game.run_dialogue(TextResourceId::OaksLabLeavingText, true, true);
	player.move(FacingDirection::Up);
	blue.set_random_facing_direction(true);
	player.set_ignore_input(false);
}

struct BallScriptData{
	int starter_index;
	int blues_starter_index;
	int blues_ball_index;
	TextResourceId question;
	VisibilityFlagId vfi;
	ActorId reds_ball;
};

static const BallScriptData static_data[] = {
	{ 1, 2, 1, TextResourceId::OaksLabCharmanderText, VisibilityFlagId::hs_starter_ball_1, ActorId::CharmanderPokeball },
	{ 2, 0, 2, TextResourceId::OaksLabSquirtleText,   VisibilityFlagId::hs_starter_ball_2, ActorId::SquirtlePokeball   },
	{ 0, 1, 0, TextResourceId::OaksLabBulbasaurText,  VisibilityFlagId::hs_starter_ball_3, ActorId::BulbasaurPokeball  },
};

DECLARE_SCRIPT(OaksLabScript08){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	auto &world = game.get_world();
	auto &player = world.get_pc();
	auto &blue = dynamic_cast<NpcTrainer &>(world.get_actor(ActorId::BlueInOaksLab));
	auto red_ball_index = vs.get(IntegerVariableId::red_ball_index);
	Point destination(static_data[red_ball_index].blues_ball_index + 6, 4);
	auto path = blue.find_path(destination);
	player.set_ignore_input(true);
	blue.set_random_facing_direction(false);
	blue.follow_path(path);
	blue.set_facing_direction(FacingDirection::Up);
	/**/
	vs.set(IntegerVariableId::OaksLabScriptIndex, 9);
}

DECLARE_SCRIPT(OaksLabScript09){
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
	game.run_dialogue(text, true, true);
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

static void ball_script(const script_parameters &parameters, int index, bool run = false){
	auto &game = *parameters.game;
	if (!run){
		game.run_in_own_coroutine([&parameters, index](){
			ball_script(parameters, index, true);
		});
		return;
	}
	auto &world = game.get_world();
	auto &player = world.get_pc();
	auto &vs = game.get_variable_store();
	if (vs.get(EventId::event_got_starter)){
		game.run_dialogue(TextResourceId::OaksLabLastMonText, true, true);
		return;
	}
	if (!vs.get(EventId::event_oak_asked_to_choose_mon)){
		game.run_dialogue(TextResourceId::OaksLabText39, true, true);
		return;
	}

	auto &data = static_data[index];
	auto reds_species = starter_index_to_species_id(data.starter_index);
	auto &reds_pokemon = *pokemon_by_species_id[(int)reds_species];
	game.display_pokedex_page(reds_pokemon.pokedex_id);
	game.run_dialogue(data.question, false, false);
	bool answer = game.run_yes_no_menu(standard_dialogue_yes_no_position);
	if (!answer){
		game.reset_dialogue_state();
		return;
	}
	//auto blues_species = starter_index_to_species_id(data.blues_starter_index);
	vs.set(IntegerVariableId::wPlayerStarter, (int)reds_species);
	vs.set(data.vfi, false);
	vs.set(StringVariableId::wcd6d_ReceivedItemName, reds_pokemon.display_name);
	world.get_actor(data.reds_ball).set_visible(false);

	game.reset_dialogue_state(false);
	game.run_dialogue(TextResourceId::OaksLabMonEnergeticText, false, true);
	game.run_dialogue(TextResourceId::OaksLabReceivedMonText, false, false);
	auto &audio = game.get_audio_interface();
	audio.play_sound(AudioResourceId::SFX_Get_Key_Item);
	audio.wait_for_sfx_to_end();
	Pokemon new_pokemon(reds_pokemon.species_id, 5, player.get_trainer_id(), game.get_engine().get_prng());
	game.reset_dialogue_state(false);
	standard_add_pokemon(game, player, new_pokemon);
	vs.set(IntegerVariableId::event_received_starter, 1);
	game.reset_dialogue_state();
	vs.set(IntegerVariableId::OaksLabScriptIndex, 8);
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
		game.run_dialogue(TextResourceId::OaksLabText_1d31d, true, false);
		//TODO: DisplayDexRating
	}else{
		auto &world = game.get_world();
		auto &player = world.get_pc();
		if (player.has_item_in_inventory(ItemId::PokeBall)){
			//I don't think this will ever execute.
			game.run_dialogue(TextResourceId::OaksLabPleaseVisitText, true, true);
		}else if (vs.get(EventId::event_beat_route22_rival_1st_battle)){
			bool got = vs.get(EventId::event_got_pokeballs_from_oak);
			if (got)
				game.run_dialogue(TextResourceId::OaksLabPleaseVisitText, true, true);
			else{
				vs.set(EventId::event_got_pokeballs_from_oak, true);
				player.receive(ItemId::PokeBall, 5);
				game.run_dialogue(TextResourceId::OaksLabGivePokeballsText1, false, false);
				auto &audio = game.get_audio_interface();
				audio.play_sound(AudioResourceId::SFX_Get_Key_Item);
				audio.wait_for_sfx_to_end();
				game.run_dialogue(TextResourceId::OaksLabGivePokeballsText2, true, true);
			}
		}else if (vs.get(EventId::event_got_pokedex)){
			game.run_dialogue(TextResourceId::OaksLabAroundWorldText, true, true);
		}else if (vs.get(EventId::event_battled_rival_in_oaks_lab)){
			if (player.has_item_in_inventory(ItemId::OaksParcel)){
				game.run_dialogue(TextResourceId::OaksLabDeliverParcelText1, false, false);
				auto &audio = game.get_audio_interface();
				audio.play_sound(AudioResourceId::SFX_Get_Key_Item);
				audio.wait_for_sfx_to_end();
				game.run_dialogue(TextResourceId::OaksLabDeliverParcelText2, true, true);
				player.remove_all(ItemId::OaksParcel);
				vs.set(IntegerVariableId::OaksLabScriptIndex, 15);
			}else
				game.run_dialogue(TextResourceId::OaksLabText_1d2fa, true, true);
		}else{
			if (vs.get(IntegerVariableId::event_received_starter))
				game.run_dialogue(TextResourceId::OaksLabText_1d2f5, true, true);
			else
				game.run_dialogue(TextResourceId::OaksLabText_1d2f0, true, true);
		}
	}
	game.reset_dialogue_state();
}

}
}
