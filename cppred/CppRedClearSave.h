#pragma once

class CppRed;

class CppRedClearSaveDialog{
	CppRed *parent;
public:
	CppRedClearSaveDialog(CppRed &parent);
	// Returns true if save must be cleared.
	bool display();
};
