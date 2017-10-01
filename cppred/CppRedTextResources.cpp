#include "CppRedTextResources.h"
#include "CppRedEngine.h"
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

void TextResource::execute(CppRedEngine &cppred, TextState &state){
	for (auto &p : this->commands)
		p->execute(cppred, state);
}

TextCommand::TextCommand(const byte_t *buffer, size_t size){
	this->data.resize(size);
	memcpy(&this->data[0], buffer, size);
}

void TextStore::execute(CppRedEngine &cppred, TextResourceId id, TextState &state){
	this->resources[(int)id]->execute(cppred, state);
}

void TextCommand::execute(CppRedEngine &cppred, TextState &state){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	auto tiles = renderer.get_tilemap(state.region).tiles + state.position.x + state.position.y * Tilemap::w;
	for (auto c : this->data){
		(tiles++)->tile_no = c;
		state.position.x++;
		cppred.text_print_delay();
	}
}

void LineCommand::execute(CppRedEngine &cppred, TextState &state){
	state.position = state.start_of_line + Point{0, 2};
	state.start_of_line = state.position;
}

void TextResourceCommand::wait_for_continue(CppRedEngine &cppred, TextState &state){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	auto tilemap = renderer.get_tilemap(state.region).tiles;
	auto &arrow_location = tilemap[state.continue_location.x + state.continue_location.y * Tilemap::w].tile_no;
	for (bool b = true;; b = !b){
		arrow_location = b ? down_arrow : ' ';
		if (cppred.check_for_user_interruption(0.5))
			break;
	}
	arrow_location = ' ';
}

void ContCommand::execute(CppRedEngine &cppred, TextState &state){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	auto tilemap = renderer.get_tilemap(state.region).tiles;
	
	this->wait_for_continue(cppred, state);

	for (int i = 0; i < 2; i++){
		for (int y = 0; y < state.box_size.y - 1; y++){
			auto y0 = (state.box_corner.y + y) * Tilemap::w;
			auto y1 = y0 + Tilemap::w;
			for (int x = 0; x < state.box_size.x; x++)
				tilemap[state.box_corner.x + x + y0] = tilemap[state.box_corner.x + x + y1];
		}
		auto y0 = (state.box_corner.y + state.box_size.y - 1) * Tilemap::w;
		for (int x = 0; x < state.box_size.x; x++)
			tilemap[state.box_corner.x + x + y0].tile_no = ' ';
		engine.wait_frames(6);
	}
	state.position = state.start_of_line;
}

void ParaCommand::execute(CppRedEngine &cppred, TextState &state){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();
	auto tilemap = renderer.get_tilemap(state.region).tiles;
	
	this->wait_for_continue(cppred, state);
	
	for (int y = 0; y < state.box_size.y; y++){
		auto y0 = (state.box_corner.y + y) * Tilemap::w;
		for (int x = 0; x < state.box_size.x; x++)
			tilemap[state.box_corner.x + x + y0].tile_no = ' ';
	}
	state.start_of_line = state.position = state.first_position;
}

void PromptCommand::execute(CppRedEngine &, TextState &){}
void DoneCommand::execute(CppRedEngine &, TextState &){}
void DexCommand::execute(CppRedEngine &, TextState &){}
void AutocontCommand::execute(CppRedEngine &, TextState &){}
void MemCommand::execute(CppRedEngine &, TextState &){}
void NumCommand::execute(CppRedEngine &, TextState &){}
