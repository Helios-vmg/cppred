#pragma once
#include "exceptions.h"
#include "utility.h"
#include "CommonTypes.h"
#include "GeneralString.h"
#include "ExternalRamBuffer.h"
#include <vector>
#include <memory>

class HostSystem;

enum class CartridgeMemoryType{
	ROM,
	MBC1,
	MBC2,
	MBC3,
	MBC4,
	MBC5,
	MBC6,
	MBC7,
	MMM01,
	Other,
};

#define DEFINE_HEADER_REGION(name, begin, end) byte_t name[end - begin]

struct CartridgeHeaderTitleRegionDmg{
	DEFINE_HEADER_REGION(game_title, 0x134, 0x144);
};

struct CartridgeHeaderTitleRegionCgb{
	DEFINE_HEADER_REGION(game_title, 0x134, 0x13F);
	DEFINE_HEADER_REGION(manufacturer_code, 0x13F, 0x143);
	DEFINE_HEADER_REGION(cbg_flag, 0x143, 0x144);
};

template <typename HeaderTitle>
struct CartridgeHeader{
	DEFINE_HEADER_REGION(entry_point, 0x100, 0x104);
	DEFINE_HEADER_REGION(nintendo_logo, 0x104, 0x134);
	HeaderTitle title_region;
	DEFINE_HEADER_REGION(new_licensee_code, 0x144, 0x146);
	DEFINE_HEADER_REGION(sgb_flag, 0x146, 0x147);
	DEFINE_HEADER_REGION(cartridge_type, 0x147, 0x148);
	DEFINE_HEADER_REGION(rom_size, 0x148, 0x149);
	DEFINE_HEADER_REGION(ram_size, 0x149, 0x14A);
	DEFINE_HEADER_REGION(destination_code, 0x14A, 0x14B);
	DEFINE_HEADER_REGION(old_licensee_code, 0x14B, 0x14C);
	DEFINE_HEADER_REGION(version_number, 0x14C, 0x14D);
	DEFINE_HEADER_REGION(header_checksum, 0x14D, 0x14E);
	DEFINE_HEADER_REGION(global_checksum, 0x14E, 0x150);
};

typedef CartridgeHeader<CartridgeHeaderTitleRegionDmg> CartridgeHeaderDmg;
typedef CartridgeHeader<CartridgeHeaderTitleRegionCgb> CartridgeHeaderCgb;

struct CartridgeCapabilities{
	byte_t cartridge_type;
	CartridgeMemoryType memory_type;
	bool
		has_ram,
		has_battery,
		has_timer,
		has_rumble,
		has_sensor,
		has_special_features;
	unsigned ram_size;
	unsigned rom_bank_count;
};

class Cartridge{
	static bool determine_capabilities(CartridgeCapabilities &, const std::vector<byte_t> &);
	static std::unique_ptr<Cartridge> construct_from_buffer_inner(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&);
protected:
	HostSystem *host;
	path_t path;
public:
	Cartridge(HostSystem &host);
	virtual ~Cartridge() = 0;
	static std::unique_ptr<Cartridge> construct_from_buffer(HostSystem &host, const path_t &, std::unique_ptr<std::vector<byte_t>> &&);
	virtual void write8(main_integer_t address, byte_t value) = 0;
	virtual byte_t read8(main_integer_t address) = 0;
	virtual void post_initialization(){}
	virtual void try_save(){}
	path_t get_path() const{
		return this->path;
	}
	virtual int get_current_rom_bank(){
		return -1;
	}
};

#define DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(x, base) \
	class x : public base{ \
	public: \
		x(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&): base(host){ \
			throw NotImplementedException(); \
		} \
		void write8(main_integer_t, byte_t) override{} \
		byte_t read8(main_integer_t address) override{ \
			return 0; \
		} \
	}

#define DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(x) \
	class x : public StandardCartridge{ \
	public: \
		x(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&v, const CartridgeCapabilities &cc): StandardCartridge(host, std::move(v), cc){ \
			throw NotImplementedException(); \
		} \
		void write8(main_integer_t, byte_t) override{} \
		byte_t read8(main_integer_t) override{ \
			return 0; \
		} \
	}

DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(PocketCameraCartridge, Cartridge);
DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(BandaiTama5Cartridge, Cartridge);
DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(Hudson3Cartridge, Cartridge);
DECLARE_UNSUPPORTED_CARTRIDGE_CLASS(Hudson1Cartridge, Cartridge);

class StandardCartridge : public Cartridge{
protected:
	typedef void(*write8_f)(StandardCartridge *, main_integer_t, byte_t);
	typedef byte_t(*read8_f)(StandardCartridge *, main_integer_t);
private:
	std::unique_ptr<write8_f[]> write_callbacks_unique;
	std::unique_ptr<read8_f[]> read_callbacks_unique;

	void initialize_cartridge_properties();
	void init_functions();

protected:
	CartridgeCapabilities capabilities;
	std::string title;
	bool supports_cgb = false;
	bool requires_cgb = false;
	std::vector<byte_t> buffer;
	size_t size;
	byte_t *data;
	unsigned rom_bank_count = 0;
	unsigned current_rom_bank = 0;
	unsigned current_ram_bank = 0;
	unsigned ram_bank_bits_copy = 0;
	write8_f *write_callbacks;
	read8_f *read_callbacks;
	ExternalRamBuffer ram;
	bool ram_banking_mode = false;
	bool ram_enabled = false;

	static void write8_do_nothing(StandardCartridge *, main_integer_t, byte_t){}
	static byte_t read8_do_nothing(StandardCartridge *, main_integer_t){ return 0; }
	virtual void init_functions_derived(){}
public:
	StandardCartridge(HostSystem &host, std::unique_ptr<std::vector<byte_t>> &&, const CartridgeCapabilities &);
	virtual ~StandardCartridge() = 0;
	void post_initialization() override;
	virtual void write8(main_integer_t, byte_t) override;
	virtual byte_t read8(main_integer_t) override;
	void commit_ram();
	void load_ram();
	int get_current_rom_bank() override{
		return this->current_rom_bank;
	}
};

DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mbc4Cartridge);
DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mbc6Cartridge);
DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mbc7Cartridge);
DECLARE_UNSUPPORTED_STANDARD_CARTRIDGE_CLASS(Mmm01Cartridge);
