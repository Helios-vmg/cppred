#include "CppRedClearSave.h"
#include "CppRed.h"

CppRedClearSaveDialog::CppRedClearSaveDialog(CppRed &parent): parent(&parent){
}

void CppRedClearSaveDialog::display(){
	this->parent->clear_screen();
	this->parent->run_default_palette_command();
	this->parent->load_font_tile_patterns();
	this->parent->load_textbox_tile_patterns();
	//TODO:
	/*
	this->parent->print_text();
	ld hl, ClearSaveDataText
	call PrintText
	coord hl, 14, 7
	lb bc, 8, 15
	ld a, NO_YES_MENU
	ld [wTwoOptionMenuID], a
	ld a, TWO_OPTION_MENU
	ld [wTextBoxID], a
	call DisplayTextBoxID
	ld a, [wCurrentMenuItem]
	and a
	jp z, Init
	callba ClearSAV
	jp Init
	*/
}
