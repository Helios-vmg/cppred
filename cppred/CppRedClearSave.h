#pragma once

class CppRed;

class CppRedClearSaveDialog{
	CppRed *parent;
public:
	CppRedClearSaveDialog(CppRed &parent);
	void display();
};
