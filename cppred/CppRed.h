#pragma once

class CppRed{
	void init();
	void disable_lcd();
public:
	CppRed();
	void set_bg_scroll(int x = -1, int y = -1);
	void set_window_position(int x = -1, int y = -1);
};
