#include "CodeGenerator.h"
#include "CpuDefinition.h"
#include "InterpreterCodeGenerator.h"
#include <fstream>

int main(int argc, char **argv){
	if (argc < 3)
		return -1;
	auto definition = std::make_shared<CpuDefinition>();
	InterpreterCodeGenerator icg(definition, "GameboyCpu");
	icg.generate();
	{
		std::ofstream file(argv[1]);
		icg.dump_declarations(file);
	}
	{
		std::ofstream file(argv[2]);
		icg.dump_definitions(file);
	}
}
