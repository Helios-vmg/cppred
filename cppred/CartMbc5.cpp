#include "CartMbc5.h"

Mbc5Cartridge::Mbc5Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &cc):
	StandardCartridge(host, std::move(buffer), cc){
	throw NotImplementedException();
}
