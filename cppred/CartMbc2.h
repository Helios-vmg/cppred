#pragma once
#include "StorageController.h"

class Mbc2Cartridge : public StandardCartridge{
protected:
public:
	Mbc2Cartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
};
