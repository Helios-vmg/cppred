#include "Engine.h"
#include <SDL_main.h>
#include <stdexcept>
#include <fstream>

int main(int argc, char **argv){
	try{
		Engine engine;
		engine.run();
	}catch (std::exception &e){
		std::ofstream file("error.txt");
		file << e.what() << std::endl;
		return -1;
	}catch (...){
		std::ofstream file("error.txt");
		file << "Unknown exception.\n";
		return -1;
	}
	return 0;
}
