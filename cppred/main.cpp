#include "Engine.h"
#include <SDL_main.h>
#include <stdexcept>
#include <iostream>

int main(int argc, char **argv){
	try{
		Engine engine;
		engine.run();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
		return -1;
	}catch (...){
		std::cerr << "Unknown exception.\n";
		return -1;
	}
	return 0;
}
