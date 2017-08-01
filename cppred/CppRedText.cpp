#include "CppRedText.h"

CppRedText::Command::~Command(){
}

CppRedText::Region &CppRedText::Region::operator<<(const char *s){
	this->commands.emplace_back(new TextCommand(s));
	return *this;
}

CppRedText::Region &CppRedText::Region::operator<<(std::unique_ptr<Command> &&p){
	this->commands.emplace_back(std::move(p));
	return *this;
}
