#include "HostSystem.h"
#include "Gameboy.h"
#include "UserInputController.h"
#include "StorageController.h"
#include "HostSystemServiceProviders.h"
#include "timer.h"
#include "exceptions.h"
#include <iostream>
#include <fstream>

Gameboy::Gameboy(HostSystem &host):
		host(&host),
		cpu(*this),
		display_controller(*this),
		sound_controller(*this),
		input_controller(*this),
		storage_controller(*this, host),
		clock(*this),
		continue_running(false),
		paused(false){
	this->cpu.initialize();
	this->realtime_counter_frequency = get_timer_resolution();
}

Gameboy::~Gameboy(){
	if (this->registered)
		this->host->get_timing_provider()->unregister_periodic_notification();
	this->stop();
	this->report_time_statistics();
	this->ram_to_save.try_save(*this->host, true);
}

void Gameboy::report_time_statistics(){
	double time_running = (double)this->time_running;
	double time_waiting = (double)this->time_waiting;
	double realtime_counter_frequency = (double)this->realtime_counter_frequency;
	std::cout
		<< "Time spent running: " << time_running / realtime_counter_frequency << " s.\n"
		<< "Time spent waiting: " << time_waiting / realtime_counter_frequency << " s.\n"
		<< "CPU usage:          " << time_running / (time_running + time_waiting) * 100 << " %\n"
		<< "Speed:              " << (time_running + time_waiting) / time_running << "x\n"
		<< "Speed 2:            " << (this->clock.get_clock_value() / (double)gb_cpu_frequency) / ((time_running + time_waiting) / realtime_counter_frequency) << "x\n";
}

RenderedFrame *Gameboy::get_current_frame(){
	return this->display_controller.get_current_frame();
}

void Gameboy::return_used_frame(RenderedFrame *frame){
	this->display_controller.return_used_frame(frame);
}

void Gameboy::run(){
	if (this->continue_running)
		return;
	if (!this->registered){
		this->registered = true;
		this->host->get_timing_provider()->register_periodic_notification(this->periodic_notification);
	}
	this->continue_running = true;
	auto This = this;
	this->interpreter_thread.reset(new std::thread([This](){ This->interpreter_thread_function(); }));
}

void Gameboy::interpreter_thread_function(){
	if (this->accumulated_time < 0)
		this->accumulated_time = 0;
	if (!this->start_time.is_initialized())
		this->start_time = this->host->get_datetime_provider()->local_now().to_posix();

	std::shared_ptr<std::exception> thrown;

	try{
		while (true){
			bool continue_running;
			bool paused;
			this->real_time_multiplier = this->speed_multiplier / (double)this->realtime_counter_frequency;
			this->current_timer_start = get_timer_count();
			while (true){
				continue_running = this->continue_running;
				paused = this->paused;
				if (!continue_running || paused)
					break;

				auto t0 = get_timer_count();
				this->run_until_next_frame();
				this->ram_to_save.try_save(*this->host);
				auto t1 = get_timer_count();
#ifndef BENCHMARKING
				this->sync_with_real_time();
#endif
				auto t2 = get_timer_count();

				this->time_running += t1 - t0;
				this->time_waiting += t2 - t1;
			}
			if (!continue_running)
				break;
			this->accumulated_time = this->get_real_time();
			if (paused)
				this->execute_pause();
		}
	}catch (GameBoyException &ex){
		thrown.reset(ex.clone());
	}catch (...){
		thrown.reset(new GenericException("Unknown exception."));
	}

	this->host->throw_exception(thrown);
}

void Gameboy::run_until_next_frame(bool force){
	do{
		this->cpu.run_one_instruction();
		if (this->input_controller.get_button_down())
			this->cpu.joystick_irq();
		this->sound_controller.update(this->speed_multiplier, this->speed_changed);
		this->speed_changed = false;
	}while (!this->display_controller.update() && (this->continue_running || force));
}

void Gameboy::sync_with_real_time(){
	double emulated_time = (double)this->clock.get_clock_value() / (double)gb_cpu_frequency;
	while (this->get_real_time() < emulated_time)
		this->periodic_notification.reset_and_wait_for(250);
}

double Gameboy::get_real_time(){
	auto now = get_timer_count();
	return this->accumulated_time + (double)(now - this->current_timer_start) * this->real_time_multiplier;
}

void Gameboy::execute_pause(){
	this->pause_accepted.signal();
	do{
		this->pause_requested.wait_for(250);
	}while (this->continue_running && this->paused);
	this->pause_accepted.signal();
}

void Gameboy::stop_and_dump_vram(const char *path){
	this->continue_running = false;
	if (std::this_thread::get_id() != this->interpreter_thread->get_id()){
		join_thread(this->interpreter_thread);
	}
	auto vram = &this->display_controller.access_vram(0x8000);
	std::unique_ptr<byte_t[]> temp(new byte_t[0x8000]);
	memset(temp.get(), 0, 0x8000);
	std::ofstream dump(path, std::ios::binary);
	dump.write((const char *)temp.get(), 0x8000);
	dump.write((const char *)vram, 0x2000);
}

void Gameboy::stop(){
	if (!this->continue_running)
		return;
	this->continue_running = false;
	if (this->interpreter_thread){
		this->periodic_notification.signal();
		this->pause_requested.signal();
		join_thread(this->interpreter_thread);
	}
}

bool Gameboy::toggle_pause(int pause) NOEXCEPT{
	this->pause_accepted.reset();
	if (pause >= 0){
		bool pause2 = !!pause;
		if (pause2 == this->paused)
			return true;
		this->paused = pause2;
	}else
		this->paused = !this->paused;
	this->pause_requested.signal();
	bool ret = this->pause_accepted.wait_for(1000);
	if (!ret)
		std::cout << "Warning: Gameboy::toggle_pause() is returning without having received the pause acknowledgement from the thread.\n";
	return ret;
}

void Gameboy::save_ram(const ExternalRamBuffer &ram){
	this->ram_to_save = ram;
	this->ram_to_save.request_save(this->storage_controller.get_cart());
}
