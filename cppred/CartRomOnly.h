#pragma once
#include "StorageController.h"

//Overrides usage of function arrays.
class RomOnlyCartridge : public StandardCartridge{
protected:
public:
	RomOnlyCartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
	void write8(main_integer_t, byte_t) override{}
	byte_t read8(main_integer_t) override;
};
