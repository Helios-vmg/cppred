#include "timer.h"
#if (defined _WIN32 || defined _WIN64)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::uint64_t get_timer_resolution(){
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}

std::uint64_t get_timer_count(){
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return count.QuadPart;
}
#else
#include <chrono>

std::uint64_t get_timer_resolution(){
	typedef std::chrono::high_resolution_clock::period T;
	return T::den / T::num;
}

std::uint64_t get_timer_count(){
	auto now = std::chrono::high_resolution_clock::now();
	return now.time_since_epoch().count();
}
#endif

