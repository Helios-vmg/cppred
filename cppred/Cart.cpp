#include "Cart.h"
#include "HostSystem.h"
#include "CartRomOnly.h"
#include "CartMbc1.h"
#include "CartMbc2.h"
#include "CartMbc3.h"
#include "CartMbc5.h"
#include <cassert>

Cartridge::Cartridge(HostSystem &host): host(&host){
}

Cartridge::~Cartridge(){
}

std::unique_ptr<Cartridge> Cartridge::construct_from_buffer_inner(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer){
	CartridgeCapabilities capabilities;
	std::unique_ptr<Cartridge> ret;
	if (!Cartridge::determine_capabilities(capabilities, *buffer))
		return ret;

	if (capabilities.has_special_features){
		switch (capabilities.cartridge_type){
			case 0xFC:
				ret = std::make_unique<PocketCameraCartridge>(host, std::move(buffer));
				break;
			case 0xFD:
				ret = std::make_unique<BandaiTama5Cartridge>(host, std::move(buffer));
				break;
			case 0xFE:
				ret = std::make_unique<Hudson3Cartridge>(host, std::move(buffer));
				break;
			case 0xFF:
				ret = std::make_unique<Hudson1Cartridge>(host, std::move(buffer));
				break;
			default:
				throw GenericException("Internal error: control flow inside the emulator has reached a point that should have never been reached.");
		}
	}

	if (!ret){
		switch (capabilities.memory_type){
			case CartridgeMemoryType::ROM:
				ret = std::make_unique<RomOnlyCartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MBC1:
				ret = std::make_unique<Mbc1Cartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MBC2:
				ret = std::make_unique<Mbc2Cartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MBC3:
				ret = std::make_unique<Mbc3Cartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MBC4:
				ret = std::make_unique<Mbc4Cartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MBC5:
				ret = std::make_unique<Mbc5Cartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MBC6:
				ret = std::make_unique<Mbc6Cartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MBC7:
				ret = std::make_unique<Mbc7Cartridge>(host, std::move(buffer), capabilities);
				break;
			case CartridgeMemoryType::MMM01:
				ret = std::make_unique<Mmm01Cartridge>(host, std::move(buffer), capabilities);
				break;
			default:
				throw GenericException("Internal error: control flow inside the emulator has reached a point that should have never been reached.");
		}
	}

	if (ret){

	}

	return ret;
}

std::unique_ptr<Cartridge> Cartridge::construct_from_buffer(HostSystem &host, const path_t &path, std::unique_ptr<std::vector<byte_t>> &&buffer){
	auto ret = Cartridge::construct_from_buffer_inner(host, std::move(buffer));
	if (ret){
		ret->path = path;
		ret->post_initialization();
	}
	return ret;
}

template <size_t N>
static bool any_match(byte_t n, const byte_t(&array)[N]){
	for (auto c : array)
		if (n == c)
			return true;
	return false;
}

bool Cartridge::determine_capabilities(CartridgeCapabilities &capabilities, const std::vector<byte_t> &buffer){
	memset(&capabilities, 0, sizeof(capabilities));
	if (buffer.size() < 0x100 + sizeof(CartridgeHeaderDmg))
		return false;
	auto cgb = (CartridgeHeaderDmg *)&buffer[0x100];

	auto type = cgb->cartridge_type[0];

	auto rom_banks_value = cgb->rom_size[0];
	if (rom_banks_value < 8)
		capabilities.rom_bank_count = 1 << (rom_banks_value + 1);
	else if (rom_banks_value == 0x52)
		capabilities.rom_bank_count = 72;
	else if (rom_banks_value == 0x53)
		capabilities.rom_bank_count = 80;
	else if (rom_banks_value == 0x54)
		capabilities.rom_bank_count = 96;
	else
		return false;

	capabilities.cartridge_type = type;

	if (type >= 0xFC){
		capabilities.has_special_features = true;
		return true;
	}

	static const byte_t rom_only_types[] = { 0x00, 0x08, 0x09 };
	static const byte_t mbc1_types[] = { 0x01, 0x02, 0x03 };
	static const byte_t mbc2_types[] = { 0x05, 0x06 };
	static const byte_t mbc3_types[] = { 0x0F, 0x10, 0x11, 0x12, 0x13 };
	static const byte_t mbc4_types[] = { 0x15, 0x16, 0x17 };
	static const byte_t mbc5_types[] = { 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E };
	static const byte_t mmm01_types[] = { 0x0B, 0x0C, 0x0D };
	static const byte_t ram_havers[] = { 0x02, 0x03, 0x08, 0x09, 0x0C, 0x0D, 0x10, 0x12, 0x13, 0x16, 0x17, 0x19, 0x1A, 0x1B, 0x1D, 0x1E, 0x22 };
	static const byte_t battery_havers[] = { 0x03, 0x06, 0x09, 0x0D, 0x0F, 0x10, 0x13, 0x17, 0x1B, 0x1E, 0x22 };
	static const byte_t timer_havers[] = { 0x0F, 0x10 };
	static const byte_t rumble_havers[] = { 0x0C, 0x0D, 0x0E, 0x22 };
	static const byte_t sensor_havers[] = { 0x022 };

	//Set memory type.

	if (any_match(type, rom_only_types))
		capabilities.memory_type = CartridgeMemoryType::ROM;
	else if (any_match(type, mbc1_types))
		capabilities.memory_type = CartridgeMemoryType::MBC1;
	else if (any_match(type, mbc2_types))
		capabilities.memory_type = CartridgeMemoryType::MBC2;
	else if (any_match(type, mbc3_types))
		capabilities.memory_type = CartridgeMemoryType::MBC3;
	else if (any_match(type, mbc4_types))
		capabilities.memory_type = CartridgeMemoryType::MBC4;
	else if (any_match(type, mbc5_types))
		capabilities.memory_type = CartridgeMemoryType::MBC5;
	else if (type == 0x20){
		capabilities = { type, CartridgeMemoryType::MBC6, false, false, false, false, false, false };
		return true;
	}else if (type == 0x22){
		capabilities.memory_type = CartridgeMemoryType::MBC7;
		capabilities.has_sensor = true;
		capabilities.has_ram = true;
		capabilities.has_rumble = true;
		capabilities.has_battery = true;
		capabilities.has_timer = false;
		capabilities.has_special_features = false;
		return true;
	}else if (any_match(type, mmm01_types))
		capabilities.memory_type = CartridgeMemoryType::MMM01;
	else
		return false;

	//Set haves:
	capabilities.has_ram = any_match(type, ram_havers);
	capabilities.has_battery = any_match(type, battery_havers);
	capabilities.has_timer = any_match(type, timer_havers);
	capabilities.has_rumble = any_match(type, rumble_havers);
	assert(!any_match(type, sensor_havers));
	//capabilities.has_sensor = any_match(type, sensor_havers);
	capabilities.has_sensor = false;
	capabilities.has_special_features = false;

	if (capabilities.has_ram){
		auto size = cgb->ram_size[0];
		static const unsigned ram_sizes[] = {
			0,
			1 << 11,
			1 << 13,
			1 << 15,
			1 << 17,
			1 << 16,
		};
		capabilities.ram_size = ram_sizes[size % 6];
	}
	return true;
}

StandardCartridge::StandardCartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&buffer, const CartridgeCapabilities &capabilities):
	Cartridge(host){
	this->capabilities = capabilities;
	this->buffer = std::move(*buffer);
	buffer.reset();
	this->initialize_cartridge_properties();
	this->size = this->buffer.size();
	assert(this->size);
	this->data = &this->buffer[0];
	if (this->capabilities.has_ram)
		this->ram.resize(this->capabilities.ram_size);
	this->write_callbacks_unique.reset(new write8_f[0x100]);
	this->read_callbacks_unique.reset(new read8_f[0x100]);
	this->write_callbacks = this->write_callbacks_unique.get();
	this->read_callbacks = this->read_callbacks_unique.get();
	this->rom_bank_count = capabilities.rom_bank_count;
}

StandardCartridge::~StandardCartridge(){
}

void StandardCartridge::post_initialization(){
	this->init_functions();
}

template <size_t N>
std::string byte_array_to_string(const byte_t(&array)[N]){
	std::string ret(array, array + N);
	while (ret.size() && !ret.back())
		ret.pop_back();
	return ret;
}

void StandardCartridge::initialize_cartridge_properties(){
	static_assert(sizeof(CartridgeHeaderDmg) == sizeof(CartridgeHeaderCgb), "Error in header structure definitions.");
	auto dmg = (CartridgeHeaderDmg *)&this->buffer[0];
	auto cgb = (CartridgeHeaderCgb *)&this->buffer[0];
	this->supports_cgb = cgb->title_region.cbg_flag[0] == 0x80 || cgb->title_region.cbg_flag[0] == 0xC0;
	if (this->supports_cgb){
		this->title = byte_array_to_string(cgb->title_region.game_title);
		this->requires_cgb = cgb->title_region.cbg_flag[0] == 0xC0;
	}else
		this->title = byte_array_to_string(dmg->title_region.game_title);
}

void StandardCartridge::init_functions(){
	std::fill(this->write_callbacks, this->write_callbacks + 0x100, write8_do_nothing);
	std::fill(this->read_callbacks, this->read_callbacks + 0x100, read8_do_nothing);
	this->init_functions_derived();
}

void StandardCartridge::write8(main_integer_t address, byte_t value){
	this->write_callbacks[address >> 8](this, address, value);
}

byte_t StandardCartridge::read8(main_integer_t address){
	return this->read_callbacks[address >> 8](this, address);
}

void StandardCartridge::commit_ram(){
	if (this->ram.is_modified()){
		this->host->get_guest().save_ram(this->ram);
		this->ram.reset_modified();
	}
}

void StandardCartridge::load_ram(){
	auto ram = this->host->load_ram(*this, this->ram.size());
	if (!ram || ram->size() < this->ram.size())
		return;
	this->ram = std::move(*ram);
}
