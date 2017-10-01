#pragma once

enum class BattleStyle{
	Shift = 0,
	Set = 1,
};

enum class TextSpeed{
	Fast = 1,
	Medium = 3,
	Slow = 5,
};

class GameOptions{
public:
	bool battle_animations_enabled = true;
	BattleStyle battle_style = BattleStyle::Shift;
	TextSpeed text_speed = TextSpeed::Fast;
};
