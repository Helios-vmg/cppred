#include "CppRedCommonFunctions.h"
#include "Engine.h"
#include "Renderer.h"

namespace CppRedScripts{

void clear_screen(Engine &engine){
	engine.get_renderer().clear_screen();
	engine.wait_exactly_one_frame();
}

}
