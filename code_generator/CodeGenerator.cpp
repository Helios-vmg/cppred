#include "CodeGenerator.h"
#include "CpuDefinition.h"

CodeGenerator::~CodeGenerator(){}

void CodeGenerator::generate(){
	for (int i = 0; i < 0x100; i++){
		this->begin_opcode_definition(i);
		this->opcode_begins();
		this->definition->generate(i, *this);
		this->end_opcode_definition(i);
	}
}

void CodeGenerator::double_opcode(unsigned first_opcode){
	this->opcode_cb_branching();
	for (int i = 0; i < 0x100; i++){
		this->begin_double_opcode_definition(first_opcode, i);
		this->definition->generate(first_opcode, i, *this);
		this->end_double_opcode_definition(first_opcode, i);
	}
}
