#pragma once
#include "StorageController.h"

class Mbc5Cartridge : public StandardCartridge{
protected:
public:
	Mbc5Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
};
