#include "ScriptDeclarations.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../CommonFunctions.h"
#include "../../../CodeGeneration/output/text.h"
#include "../../../CodeGeneration/output/audio.h"
#include "../../../CodeGeneration/output/variables.h"
#include "../../../CodeGeneration/output/actors.h"
#include "../../../CodeGeneration/output/trainer_parties.h"
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
		nullptr,
		OaksLabScript10,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		OaksLabScript18,
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
	auto &world = game.get_world();
	auto &player = world.get_pc();
	if (player.get_map_position().y < 6)
		return;
	if (player.is_moving())
		player.abort_movement();
	auto &blue = dynamic_cast<NpcTrainer &>(world.get_actor(ActorId::BlueInOaksLab));
	blue.set_random_facing_direction(false);
	blue.set_facing_direction(FacingDirection::Down);
	AutoIgnoreInput aii(player);
	game.run_dialogue(TextResourceId::OaksLabLeavingText, true, true);
	player.move(FacingDirection::Up);
	blue.set_random_facing_direction(true);
}

struct BallScriptData{
	int starter_index;
	int blues_starter_index;
	int blues_ball_index;
	TextResourceId question;
	VisibilityFlagId vfi;
	ActorId ball_sprite;
};

static const BallScriptData static_data[] = {
	{ 1, 2, 1, TextResourceId::OaksLabCharmanderText, VisibilityFlagId::hs_starter_ball_1, ActorId::CharmanderPokeball },
	{ 2, 0, 2, TextResourceId::OaksLabSquirtleText,   VisibilityFlagId::hs_starter_ball_2, ActorId::SquirtlePokeball   },
	{ 0, 1, 0, TextResourceId::OaksLabBulbasaurText,  VisibilityFlagId::hs_starter_ball_3, ActorId::BulbasaurPokeball  },
};

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

DECLARE_SCRIPT(OaksLabScript08){
	auto &game = *parameters.game;
	auto &vs = game.get_variable_store();
	auto &world = game.get_world();
	auto &player = world.get_pc();
	auto &blue = dynamic_cast<NpcTrainer &>(world.get_actor(ActorId::BlueInOaksLab));
	auto red_ball_index = vs.get(IntegerVariableId::red_ball_index);
	auto &red_ball_data = static_data[red_ball_index];
	auto &blue_ball_data = static_data[red_ball_data.blues_ball_index];
	auto blues_species = starter_index_to_species_id(blue_ball_data.starter_index);
	auto &blues_pokemon = *pokemon_by_species_id[(int)blues_species];

	AutoIgnoreInput aii(player);

	//Move Blue into position in front of the ball.
	Point destination(red_ball_data.blues_ball_index + 6, 4);
	auto path = blue.find_path(destination);
	blue.set_random_facing_direction(false);
	blue.follow_path(path);
	blue.set_facing_direction(FacingDirection::Up);

	game.run_dialogue(TextResourceId::OaksLabRivalPickingMonText, true, false);
	
	//Hide Blue's ball permanently.
	auto &blues_ball_sprite = world.get_actor(blue_ball_data.ball_sprite);
	blues_ball_sprite.set_visible(false);
	vs.set(blue_ball_data.vfi, 0);

	vs.set(StringVariableId::wcd6d_ReceivedItemName, blues_pokemon.display_name);
	game.reset_dialogue_state(false);
	game.run_dialogue(TextResourceId::OaksLabRivalReceivedMonText, false, false);
	auto &audio = game.get_audio_interface();
	audio.play_sound(AudioResourceId::SFX_Get_Key_Item);
	audio.wait_for_sfx_to_end();
	game.dialogue_wait();
	game.reset_dialogue_state();
	
	vs.set(EventId::event_got_starter, true);
	vs.set(IntegerVariableId::wRivalStarter, (int)blues_species);
	vs.set(IntegerVariableId::OaksLabScriptIndex, 10);
}

DECLARE_SCRIPT(OaksLabScript10){
	auto &game = *parameters.game;
	auto &world = game.get_world();
	auto &player = world.get_pc();
	if (player.get_map_position().y < 6)
		return;
	auto &vs = game.get_variable_store();
	AutoIgnoreInput aii(player);
	player.abort_movement();
	auto red_ball_index = vs.get(IntegerVariableId::red_ball_index);
	auto &red_ball_data = static_data[red_ball_index];

	auto &blue = dynamic_cast<NpcTrainer &>(world.get_actor(ActorId::BlueInOaksLab));
	blue.set_facing_direction(FacingDirection::Down);
	player.set_facing_direction(FacingDirection::Up);

	game.get_audio_interface().play_sound(AudioResourceId::Music_MeetRival);
	game.run_dialogue(TextResourceId::OaksLabRivalChallengeText, true, true);

	auto destination = player.get_map_position() + Point(0, -1);
	auto path = blue.find_path(destination);
	blue.set_random_facing_direction(false);
	blue.follow_path(path);
	blue.set_facing_direction(FacingDirection::Down);

	auto blue_party = (red_ball_data.starter_index + 2) % 3 + 1;
	game.run_trainer_battle(
		TextResourceId::FirstBlueBattlePlayerWon,
		TextResourceId::FirstBlueBattlePlayerLost,
		blue,
		blue_party
	);
	world.play_current_map_music();
	player.get_party().heal();
	vs.set(EventId::event_battled_rival_in_oaks_lab, true);
	game.run_dialogue(TextResourceId::OaksLabRivalToughenUpText, true, true);
	
	destination = {4 + 5 - player.get_map_position().x, 11};
	path = blue.find_path(destination);
	auto &audio = game.get_audio_interface();
	audio.play_sound(AudioResourceId::Music_MeetRivalAlternative);
	{
		bool done = false;
		auto &coroutine = Coroutine::get_current_coroutine();
		Coroutine blue_moving("Blue's moving coroutine temp", coroutine.get_clock(), [&done, &blue, &path](Coroutine &){
			blue.follow_path(path);
			done = true;
		});
		while (!done){
			player.look_towards_actor(blue);
			blue_moving.get_clock().step();
			blue_moving.resume();
			coroutine.yield();
		}
	}
	blue.set_visible(false);
	vs.set(VisibilityFlagId::hs_oaks_lab_rival, false);
	world.play_current_map_music();
	vs.set(IntegerVariableId::OaksLabScriptIndex, 18);
}

DECLARE_SCRIPT(OaksLabScript18){
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

static void ball_script(const script_parameters &parameters, int index, bool run = false){
	auto &game = *parameters.game;
	if (!run){
		game.run_in_own_coroutine([parameters, index](){
			ball_script(parameters, index, true);
		}, false);
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
	vs.set(IntegerVariableId::wPlayerStarter, (int)reds_species);
	vs.set(data.vfi, false);
	vs.set(StringVariableId::wcd6d_ReceivedItemName, reds_pokemon.display_name);
	world.get_actor(data.ball_sprite).set_visible(false);

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

DECLARE_SCRIPT(DisplayOakLabEmailText){
	parameters.game->get_audio_interface().play_sound(AudioResourceId::SFX_Turn_On_PC);
	parameters.game->run_dialogue(TextResourceId::OakLabEmailText, true, true);
	parameters.game->get_audio_interface().play_sound(AudioResourceId::SFX_Turn_Off_PC);
}

DECLARE_SCRIPT(DisplayOakLabLeftPoster){
	parameters.game->run_dialogue(TextResourceId::PushStartText, true, true);
}

DECLARE_SCRIPT(DisplayOakLabRightPoster){
	bool less_than_two = parameters.game->get_world().get_pc().get_pokedex().get_owned_count() < 2;
	TextResourceId id = less_than_two ? TextResourceId::SaveOptionText : TextResourceId::StrengthsAndWeaknessesText;
	parameters.game->run_dialogue(id, true, true);
}

}
}
