#pragma once
#include "CommonTypes.h"
#include "GeneralString.h"
#include <memory>
#include "HostSystemServiceProviders.h"
#include "Cart.h"

class Gameboy;
class HostSystem;

class StorageController{
	Gameboy *system;
	HostSystem *host;
	std::unique_ptr<Cartridge> cartridge;
public:
	StorageController(Gameboy &system, HostSystem &host): system(&system), host(&host){}
	bool load_cartridge(const path_t &path);
	void write8(main_integer_t address, byte_t value){
		this->cartridge->write8(address, value);
	}
	byte_t read8(main_integer_t address){
		return this->cartridge->read8(address);
	}
	int get_current_rom_bank();
	Cartridge &get_cart(){
		return *this->cartridge;
	}
};
