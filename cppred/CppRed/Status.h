#pragma once

namespace CppRed{

enum class StatusCondition{
	Normal = 0,
	Poisoned = 3,
	Burned = 4,
	Frozen = 5,
	Paralized = 6,
};

enum class StatusCondition2{
	Normal = 0,
	Poisoned = 3,
	Burned = 4,
	Frozen = 5,
	Paralized = 6,
	Fainted = 16,
};

const char *to_string(StatusCondition);
const char *to_string(StatusCondition2);

}
