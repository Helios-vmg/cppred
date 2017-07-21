#include "HostSystem.h"
#include "DisplayController.h"
#include "Gameboy.h"
#include "StorageController.h"
#include <iostream>
#include <iomanip>
#include <SDL.h>

HostSystem::HostSystem(
			StorageProvider *storage_provider,
			TimingProvider *timing_provider,
			GraphicsOutputProvider *graphics_provider,
			AudioOutputProvider *audio_provider,
			EventProvider *event_provider,
			DateTimeProvider *datetime_provider):
		storage_provider(storage_provider),
		timing_provider(timing_provider),
		graphics_provider(graphics_provider),
		audio_provider(audio_provider),
		event_provider(event_provider),
		datetime_provider(datetime_provider){

	if (!this->storage_provider){
		this->storage_provider = new StdStorageProvider;
		this->owned_storage_provider.reset(this->storage_provider);
	}
	if (this->event_provider)
		this->event_provider->set_host(*this);
	this->reinit();
	if (this->audio_provider)
		this->audio_provider->set_callbacks(
			[this](){ return this->gameboy->get_sound_controller().get_current_frame(); },
			[this](AudioFrame *frame){ this->gameboy->get_sound_controller().return_used_frame(frame); }
		);
}

HostSystem::~HostSystem(){
	this->audio_provider->stop_audio();
	this->gameboy.reset();
}

void HostSystem::reinit(){
	this->gameboy.reset(new Gameboy(*this));
}

void HostSystem::run(){
	this->gameboy->run();
	try{
#ifdef BENCHMARKING
		auto start = SDL_GetTicks();
#endif
		while (this->handle_events()){
#ifdef BENCHMARKING
			if (SDL_GetTicks() - start >= 20000)
				break;
#endif
			this->check_exceptions();
			this->render();
		}
	}catch (std::exception &e){
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}


void HostSystem::stop_and_dump_vram(){
	this->gameboy->stop_and_dump_vram("vram.bin");
}

void HostSystem::check_exceptions(){
	std::lock_guard<std::mutex> lg(this->thrown_exception_mutex);
	if (this->thrown_exception)
		throw *this->thrown_exception;
}

void HostSystem::throw_exception(const std::shared_ptr<std::exception> &ex){
	std::lock_guard<std::mutex> lg(this->thrown_exception_mutex);
	this->thrown_exception = ex;
}

void HostSystem::render(){
	if (!this->graphics_provider)
		return;
	auto frame = this->gameboy->get_current_frame();
	this->graphics_provider->render(frame);
	if (frame)
		this->gameboy->return_used_frame(frame);
}

bool HostSystem::handle_events(){
	if (!this->event_provider)
		return false;
	EventProvider::HandleEventsResult result;
	auto ret = this->event_provider->handle_events(result);
	if (result.input_state)
		this->gameboy->get_input_controller().set_input_state(result.input_state, result.button_down, result.button_up);
	return ret;
}

path_t get_ram_location(Cartridge &cart, StorageProvider &storage_provider){
	return storage_provider.get_save_location(cart, SaveFileType::Ram);
}

path_t get_rtc_location(Cartridge &cart, StorageProvider &storage_provider){
	return storage_provider.get_save_location(cart, SaveFileType::Rtc);
}

void HostSystem::save_ram(Cartridge &cart, const std::vector<byte_t> &ram){
	std::cout << "Requested RAM save. " << ram.size() << " bytes.\n";
	
	auto path = get_ram_location(cart, *this->storage_provider);
	if (!this->storage_provider->save_file(path, ram))
		std::cout << "RAM save failed.\n";
}

std::unique_ptr<std::vector<byte_t>> HostSystem::load_ram(Cartridge &cart, size_t expected_size){
	std::cout << "Requested RAM load.\n";

	auto path = get_ram_location(cart, *this->storage_provider);
	auto ret = this->storage_provider->load_file(path, expected_size);
	if (!ret)
		std::cout << "RAM load failed.\n";
	return ret;
}

void HostSystem::save_rtc(Cartridge &cart, posix_time_t time){
	static_assert(std::numeric_limits<double>::is_iec559, "Only iec559 float/doubles supported!");

	std::cout << "Requested RTC save.\n";
	auto path = get_rtc_location(cart, *this->storage_provider);
	double timestamp = this->datetime_provider->date_to_double_timestamp(DateTime::from_posix(time));
	byte_t buffer[sizeof(double) + 4];
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, &timestamp, sizeof(double));
	if (!this->storage_provider->save_file(path, buffer, sizeof(buffer)))
		std::cout << "RTC save failed.\n";
}

posix_time_t HostSystem::load_rtc(Cartridge &cart){
	static_assert(std::numeric_limits<double>::is_iec559, "Only iec559 float/doubles supported!");
	std::cout << "Requested RTC load.\n";
	auto path = get_rtc_location(cart, *this->storage_provider);
	auto ret = this->storage_provider->load_file(path, sizeof(double) + 4);
	if (!ret){
		std::cout << "RTC load failed.\n";
		return -1;
	}
	double timestamp;
	memcpy(&timestamp, &(*ret)[0], sizeof(double));
	return this->datetime_provider->double_timestamp_to_posix(timestamp);
}

void HostSystem::toggle_fastforward(bool on) NOEXCEPT{
	if (!this->gameboy->toggle_pause(true))
		return;
	double speed = on ? 5.0 : 1.0;
	this->gameboy->set_speed_multiplier(speed);
	this->gameboy->toggle_pause(false);
}

void HostSystem::toggle_slowdown(bool on) NOEXCEPT{
	if (!this->gameboy->toggle_pause(true))
		return;
	double speed = on ? 0.1 : 1.0;
	this->gameboy->set_speed_multiplier(speed);
	this->gameboy->toggle_pause(false);
}

void HostSystem::toggle_pause(int pause){
	this->gameboy->toggle_pause(pause);
}

void HostSystem::write_frame_to_disk(std::string &path, const RenderedFrame &frame){
	this->graphics_provider->write_frame_to_disk(path, frame);
}
