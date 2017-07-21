#pragma once
#include "GameboyCpu.h"
#include "DisplayController.h"
#include "UserInputController.h"
#include "SystemClock.h"
#include "StorageController.h"
#include "SoundController.h"
#include "threads.h"
#include "HostSystemServiceProviders.h"
#include "ExternalRamBuffer.h"

class HostSystem;

enum class GameboyMode{
	DMG,
	CGB,
};

class Gameboy{
	HostSystem *host;
	GameboyCpu cpu;
	DisplayController display_controller;
	UserInputController input_controller;
	StorageController storage_controller;
	SoundController sound_controller;
	SystemClock clock;
	double accumulated_time = -1;
	std::uint64_t current_timer_start;
	std::uint64_t realtime_counter_frequency = 0;
	std::uint64_t realtime_execution = 0;
	std::uint64_t time_running = 0;
	std::uint64_t time_waiting = 0;
	double real_time_multiplier;
	double speed_multiplier = 1;
	bool speed_changed = false;
	GameboyMode mode = GameboyMode::DMG;
	std::atomic<bool> continue_running,
		paused;
	std::unique_ptr<std::thread> interpreter_thread;
	Event periodic_notification;
	bool registered = false;
	Event pause_requested;
	Event pause_accepted;
	//Stores a timestamp of the first time interpreter_thread_function() was called.
	Maybe<posix_time_t> start_time;
	ExternalRamBuffer ram_to_save;

	void interpreter_thread_function();
	void sync_with_real_time();
	double get_real_time();
	void report_time_statistics();
	//Blocks until unpaused.
	void execute_pause();
public:
	Gameboy(HostSystem &host);
	~Gameboy();

	DisplayController &get_display_controller(){
		return this->display_controller;
	}
	UserInputController &get_input_controller(){
		return this->input_controller;
	}
	StorageController &get_storage_controller(){
		return this->storage_controller;
	}
	SoundController &get_sound_controller(){
		return this->sound_controller;
	}
	GameboyCpu &get_cpu(){
		return this->cpu;
	}
	SystemClock &get_system_clock(){
		return this->clock;
	}
	GameboyMode get_mode() const{
		return this->mode;
	}

	void run();
	//When running from the main thread, set force = true to make the function
	//ignore the value of Gameboy::continue_running.
	void run_until_next_frame(bool force = false);
	void stop();
	//Note: May return nullptr! In which case, no frame is currently ready, and
	//white should be drawn.
	RenderedFrame *get_current_frame();
	void return_used_frame(RenderedFrame *);
	void stop_and_dump_vram(const char *path);
	bool toggle_pause(int) NOEXCEPT;
	//Must be called while the CPU is paused!
	double get_speed_multiplier() const NOEXCEPT{
		return this->speed_multiplier;
	}
	void set_speed_multiplier(double speed) NOEXCEPT{
		this->speed_multiplier = speed;
		this->speed_changed = true;
	}
	HostSystem *get_host() const{
		return this->host;
	}
	posix_time_t get_start_time() const{
		return *this->start_time;
	}
	void save_ram(const ExternalRamBuffer &);
};
