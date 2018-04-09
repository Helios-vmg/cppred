#include "stdafx.h"
#include "Status.h"

namespace CppRed{

const char *to_string(StatusCondition status){
	switch (status){
		case StatusCondition::Normal:
			return "OK";
		case StatusCondition::Poisoned:
			return "PSN";
		case StatusCondition::Burned:
			return "BRN";
		case StatusCondition::Frozen:
			return "FRZ";
		case StatusCondition::Paralized:
			return "PAR";
	}
}

const char *to_string(StatusCondition2 status){
	if (status == StatusCondition2::Fainted)
		return "FNT";
	return to_string((StatusCondition)status);
}

}
