#include "StorageController.h"
#include "HostSystem.h"
#include <cassert>
#include <ctime>

bool StorageController::load_cartridge(const path_t &path){
	auto buffer = this->host->get_storage_provider()->load_file(path, 16 << 20);
	if (!buffer || !buffer->size())
		return false;
	auto new_cart = Cartridge::construct_from_buffer(*this->host, path, std::move(buffer));
	if (!new_cart)
		return false;
	this->cartridge = std::move(new_cart);
	return true;
}

int StorageController::get_current_rom_bank(){
	if (!this->cartridge)
		return -1;
	return this->cartridge->get_current_rom_bank();
}
