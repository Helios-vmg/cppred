#pragma once
#include <mutex>
#include <memory>

class FreeImageInitializer{
	static std::mutex mutex;
	static std::unique_ptr<void, void(*)(void *)> ptr;
public:
	FreeImageInitializer();
	FreeImageInitializer(const FreeImageInitializer &) = delete;
	FreeImageInitializer(FreeImageInitializer &&) = delete;
	void operator=(const FreeImageInitializer &) = delete;
	void operator=(FreeImageInitializer &&) = delete;
};
