#pragma once
#include "threads.h"
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <SDL.h>

class Engine;
class AudioRenderer;
enum class AudioResourceId;
namespace CppRed{
class AudioProgram;
}

class AudioScheduler{
	Engine *engine;
	std::unique_ptr<AudioRenderer> renderer;
	std::unique_ptr<CppRed::AudioProgram> program;
	std::unique_ptr<std::thread> thread;
	std::atomic<bool> continue_running;
	SDL_TimerID timer_id = 0;
	Event timer_event;

	static Uint32 SDLCALL timer_callback(Uint32 interval, void *param);
	void processor();
	void stop();
public:
	AudioScheduler(Engine &engine, std::unique_ptr<AudioRenderer> &&renderer, std::unique_ptr<CppRed::AudioProgram> &&program);
	~AudioScheduler();
	void start();
};
