#include "Scripts.h"
#include "../Game.h"
#include "../World.h"
#include "../PlayerCharacter.h"
#include "../../../CodeGeneration/output/text.h"
#include "Events.h"

namespace CppRed{
namespace Scripts{

DECLARE_SCRIPT(PrintRedSNESText){
	parameters.game->run_dialog_from_world(TextResourceId::RedBedroomSNESText, *parameters.caller);
}

DECLARE_SCRIPT(RedsHouse1FText1){
	if (parameters.game->get_variable_store().get_number_default(event_received_starter)){
		throw std::runtime_error("Not implemented.");
	}else
		parameters.game->run_dialog_from_world(TextResourceId::MomWakeUpText, *parameters.caller);
}

DECLARE_SCRIPT(RedsHouse1FText2){
	auto id = parameters.caller->get_facing_direction() == FacingDirection::Up ? TextResourceId::StandByMeText : TextResourceId::TVWrongSideText;
	parameters.game->run_dialog_from_world(id, *parameters.caller);
}

}
}