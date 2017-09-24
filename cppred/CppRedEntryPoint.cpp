#include "CppRedEntryPoint.h"
#include "Engine.h"
#include "Renderer.h"
#include "CppRedIntro.h"
#include "CppRedEngine.h"

namespace CppRed{

void entry_point(Engine &engine){
	CppRedEngine cppred(engine);
	engine.get_renderer().set_enable_bg(true);
	intro(cppred);
}

}
