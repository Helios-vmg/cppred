#include "CartMbc2.h"

Mbc2Cartridge::Mbc2Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
	StandardCartridge(host, std::move(buffer), cc){
	throw NotImplementedException();
}
