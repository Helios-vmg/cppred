#include "CartRomOnly.h"

RomOnlyCartridge::RomOnlyCartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
	StandardCartridge(host, std::move(buffer), cc){
}

byte_t RomOnlyCartridge::read8(main_integer_t address){
	return this->data[address & 0x7FFF];
}
