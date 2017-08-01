#include "CppRedClearSave.h"
#include "CppRed.h"

CppRedClearSaveDialog::CppRedClearSaveDialog(CppRed &parent): parent(&parent){
}

bool CppRedClearSaveDialog::display(){
	this->parent->prepare_menu();
	this->parent->print_text(this->parent->text.ClearSaveDataText);
	auto location = this->parent->get_tilemap_location(14, 7);
	this->parent->wram.wTwoOptionMenuID = MenuType::NoYes;
	this->parent->wram.wTextBoxID = TextBoxId::TwoOptionMenu;
	this->parent->display_textbox_id(location, 8, 15);
	return this->parent->wram.wCurrentMenuItem != 0;
}
