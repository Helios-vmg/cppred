#include "CppRedText.h"
#include "CppRed.h"

#define SEQ(x) (this->##x)
#define MEM(x) std::make_unique<MemCommand>([this](){ return this->parent->wram.x.to_string(); })
#define NUM(x, y, z) std::make_unique<NumCommand>(NumSource::x, y, z)
#define BCD(x, y) std::make_unique<BcdCommand>(BcdSource::x, y)
#define DEX std::make_unique<DexCommand>()
#define PARA std::make_unique<ParaCommand>()
#define LINE std::make_unique<LineCommand>()
#define CONT std::make_unique<ContCommand>()
#define DONE std::make_unique<DoneCommand>()
#define NEXT std::make_unique<NextCommand>()
#define PAGE std::make_unique<PageCommand>()
#define PROMPT std::make_unique<PromptCommand>()
#define AUTOCONT std::make_unique<AutocontCommand>()

CppRedText::CppRedText(CppRed &parent): parent(&parent){
#include "../CodeGeneration/output/text.inl"
	this->initialize_maps();
}
