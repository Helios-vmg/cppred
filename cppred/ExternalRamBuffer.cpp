#include "ExternalRamBuffer.h"
#include "HostSystem.h"
#include "timer.h"

ExternalRamBuffer::ExternalRamBuffer(size_t size){
	this->resize(size);
}

const ExternalRamBuffer &ExternalRamBuffer::operator=(const ExternalRamBuffer &other){
	this->internal = other.internal;
	this->write_requested = false;
	return *this;
}

const ExternalRamBuffer &ExternalRamBuffer::operator=(std::vector<byte_t> &&buffer){
	this->internal.reset(new decltype(this->internal)::element_type(std::move(buffer)));
	return *this;
}

const ExternalRamBuffer &ExternalRamBuffer::operator=(decltype(internal) &buffer){
	this->internal = buffer;
	return *this;
}

byte_t ExternalRamBuffer::read(size_t position) const{
	return (*this->internal)[position];
}

void ExternalRamBuffer::write(size_t position, byte_t data){
	if (this->internal.use_count() > 1)
		this->internal = std::make_shared<decltype(this->internal)::element_type>(*this->internal);
	auto &b = (*this->internal)[position];
	if (b != data)
		this->modified = true;
	b = data;
}

void ExternalRamBuffer::resize(size_t size){
	if (!this->internal)
		this->internal.reset(new decltype(this->internal)::element_type(size));
	else
		this->internal->resize(size);
}

void ExternalRamBuffer::request_save(Cartridge &cart){
	this->write_requested = true;
	this->write_requested_at = std::chrono::steady_clock::now();
	this->cart = &cart;
}

void ExternalRamBuffer::try_save(HostSystem &host, bool force){
	if (!this->write_requested)
		return;
	if (!force){
		auto now = std::chrono::steady_clock::now();
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now - this->write_requested_at).count();
		if (seconds < 20)
			return;
	}
	host.save_ram(*this->cart, *this->internal);
	this->write_requested = false;
}
