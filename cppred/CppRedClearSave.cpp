#include "CppRedClearSave.h"
#include "CppRed.h"

CppRedClearSaveDialog::CppRedClearSaveDialog(CppRed &parent): parent(&parent){
}

bool CppRedClearSaveDialog::display(){
	this->parent->prepare_menu();
	this->parent->print_text(this->parent->text.ClearSaveDataText);
	this->parent->display_two_option_menu(TwoOptionMenuType::NoYes, 14, 7);
	return this->parent->wram.wCurrentMenuItem != 0;
}
