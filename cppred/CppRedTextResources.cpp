#include "CppRedTextResources.h"
#include "utility.h"

TextStore::TextStore(){
	auto buffer = packed_text_data;
	size_t size = packed_text_data_size;
	while (size){
		auto resource = this->parse_resource(buffer, size);
		if ((size_t)resource->id >= this->resources.size())
			this->resources.resize((size_t)resource->id);
		this->resources.emplace_back(std::move(resource));
	}
}

std::unique_ptr<TextResource> TextStore::parse_resource(const byte_t *&buffer, size_t &size){
	if (size < 4)
		throw std::runtime_error("TextStore::parse_command(): Parse error.");
	auto id = (TextResourceId)read_u32(buffer);
	
	buffer += 4;
	size -= 4;
	auto ret = std::make_unique<TextResource>();
	ret->id = id;
	bool stop;
	do{
		auto command = this->parse_command(buffer, size, stop);
		if (command)
			ret->commands.emplace_back(std::move(command));
	}while (!stop);
	return ret;
}

std::unique_ptr<TextResourceCommand> TextStore::parse_command(const byte_t *&buffer, size_t &size, bool &stop){
	if (size < 1)
		throw std::runtime_error("TextStore::parse_command(): Parse error.");
	auto command = (TextResourceCommandType)*(buffer++);
	size--;
	stop = false;
	std::unique_ptr<TextResourceCommand> ret;
	switch (command){
		case TextResourceCommandType::End:
			stop = true;
			break;
		case TextResourceCommandType::Text:
			{
				size_t n = 0;
				while (true){
					if (!size)
						throw std::runtime_error("TextStore::parse_command(): Parse error.");
					if (!buffer[n])
						break;
					n++;
					size--;
				}
				ret.reset(new TextCommand(buffer, n));
				buffer += n + 1;
				if (!size)
					throw std::runtime_error("TextStore::parse_command(): Parse error.");
				size--;
			}
			break;
		case TextResourceCommandType::Line:
			ret.reset(new LineCommand);
			break;
		case TextResourceCommandType::Next:
			ret.reset(new NextCommand);
			break;
		case TextResourceCommandType::Cont:
			ret.reset(new ContCommand);
			break;
		case TextResourceCommandType::Para:
			ret.reset(new ParaCommand);
			break;
		case TextResourceCommandType::Page:
			ret.reset(new PageCommand);
			break;
		case TextResourceCommandType::Prompt:
			ret.reset(new PromptCommand);
			break;
		case TextResourceCommandType::Done:
			ret.reset(new DoneCommand);
			stop = true;
			break;
		case TextResourceCommandType::Dex:
			ret.reset(new DexCommand);
			stop = true;
			break;
		case TextResourceCommandType::Autocont:
			ret.reset(new AutocontCommand);
			break;
		case TextResourceCommandType::Mem:
			{
				std::string variable;
				while (true){
					if (!size)
						throw std::runtime_error("TextStore::parse_command(): Parse error.");
					if (!*buffer)
						break;
					variable.push_back(*buffer);
					buffer++;
					size--;
				}
				buffer++;
				if (!size)
					throw std::runtime_error("TextStore::parse_command(): Parse error.");
				size--;
				ret.reset(new MemCommand(std::move(variable)));
			}
			break;
		case TextResourceCommandType::Num:
			{
				std::string variable;
				while (true){
					if (!size)
						throw std::runtime_error("TextStore::parse_command(): Parse error.");
					if (!*buffer)
						break;
					variable.push_back(*buffer);
					buffer++;
					size--;
				}
				buffer++;
				if (size < 5)
					throw std::runtime_error("TextStore::parse_command(): Parse error.");
				auto digits = read_u32(buffer);
				buffer += 4;
				size -= 5;
				ret.reset(new NumCommand(std::move(variable), digits));
			}
			break;
		default:
			throw std::runtime_error("TextStore::parse_command(): Invalid switch.");
			break;
	}
	return ret;
}

void TextResource::execute(CppRedEngine &cppred, TextStore &store){
	for (auto &p : this->commands)
		p->execute(cppred, store);
}

TextCommand::TextCommand(const byte_t *buffer, size_t size){
	this->data.resize(size);
	memcpy(&this->data[0], buffer, size);
}

void TextCommand::execute(CppRedEngine &, TextStore &){}
void LineCommand::execute(CppRedEngine &, TextStore &){}
void NextCommand::execute(CppRedEngine &, TextStore &){}
void ContCommand::execute(CppRedEngine &, TextStore &){}
void ParaCommand::execute(CppRedEngine &, TextStore &){}
void PageCommand::execute(CppRedEngine &, TextStore &){}
void PromptCommand::execute(CppRedEngine &, TextStore &){}
void DoneCommand::execute(CppRedEngine &, TextStore &){}
void DexCommand::execute(CppRedEngine &, TextStore &){}
void AutocontCommand::execute(CppRedEngine &, TextStore &){}
void MemCommand::execute(CppRedEngine &, TextStore &){}
void NumCommand::execute(CppRedEngine &, TextStore &){}
