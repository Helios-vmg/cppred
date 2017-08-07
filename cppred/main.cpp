#include "HostSystem.h"
#include "SdlProvider.h"
#include <iostream>

int main(int argc, char **argv){
	if (argc < 2)
		return 0;
	auto sdl = std::make_unique<SdlProvider>();
	auto dtp = std::make_unique<StdDateTimeProvider>();
	HostSystem system(nullptr, sdl.get(), sdl.get(), sdl.get(), sdl.get(), dtp.get());
	try{
		system.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
