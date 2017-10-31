#include "FreeImageInitializer.h"
#include <FreeImage.h>
#include <iostream>

static void deinit(void *p){
	if (p){
		std::cout << "Deinitializing FreeImage.\n";
		FreeImage_DeInitialise();
	}
}

std::mutex FreeImageInitializer::mutex;
std::unique_ptr<void, void(*)(void *)> FreeImageInitializer::ptr(nullptr, deinit);

FreeImageInitializer::FreeImageInitializer(){
	std::lock_guard<std::mutex> lg(this->mutex);
	if (this->ptr)
		return;
	std::cout << "Initializing FreeImage.\n";
	FreeImage_Initialise();
	this->ptr.reset((void *)1);
}
