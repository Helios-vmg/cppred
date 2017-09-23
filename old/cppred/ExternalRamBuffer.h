#pragma once
#include "CommonTypes.h"
#include <memory>
#include <vector>
#include <chrono>

class HostSystem;
class Cartridge;

class ExternalRamBuffer{
	std::shared_ptr<std::vector<byte_t>> internal;
	bool modified = false;
	bool write_requested = false;
	std::chrono::time_point<std::chrono::steady_clock> write_requested_at;
	Cartridge *cart = nullptr;
public:
	ExternalRamBuffer(){}
	ExternalRamBuffer(size_t);
	const ExternalRamBuffer &operator=(const ExternalRamBuffer &);
	const ExternalRamBuffer &operator=(std::vector<byte_t> &&);
	const ExternalRamBuffer &operator=(decltype(internal) &);
	byte_t read(size_t position) const;
	void write(size_t position, byte_t data);
	void resize(size_t);
	void request_save(Cartridge &cart);
	void try_save(HostSystem &, bool force = false);
	size_t size() const{
		if (!this->internal)
			return 0;
		return this->internal->size();
	}
	bool is_modified() const{
		return this->modified;
	}
	void reset_modified(){
		this->modified = false;
	}
};
