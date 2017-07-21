#pragma once

class CodeGenerator;

class CpuDefinition{
public:
	void generate(unsigned opcode, CodeGenerator &);
	void generate(unsigned first_opcode, unsigned second_opcode, CodeGenerator &);
};
