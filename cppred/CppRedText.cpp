#include "CppRedText.h"

#define SEQ(x) (this->sequence_##x)
#define MEM(x) std::make_unique<MemCommand>(MemorySources::x)
#define NUM(x, y, z) std::make_unique<NumCommand>(MemorySources::x, y, z)
#define BCD(x, y) std::make_unique<BcdCommand>(MemorySources::x, y)
#define DEX std::make_unique<DexCommand>()
#define PARA std::make_unique<ParaCommand>()
#define LINE std::make_unique<LineCommand>()
#define CONT std::make_unique<ContCommand>()
#define DONE std::make_unique<DoneCommand>()
#define NEXT std::make_unique<NextCommand>()
#define PAGE std::make_unique<PageCommand>()
#define PROMPT std::make_unique<PromptCommand>()

CppRedText::CppRedText(){
#include "../CodeGeneration/output/text.inl"
}
