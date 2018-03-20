#include "TextResources.h"
#include "Game.h"
#include "utility.h"
#include "../PokemonInfo.h"
#include "../CodeGeneration/output/audio.h"
#include "../CodeGeneration/output/pokemon_declarations.h"
#include <sstream>

namespace CppRed{

TextStore::TextStore(){
	std::vector<std::pair<std::string, SpeciesId>> species;
	for (auto &p : pokemon_by_pokedex_id)
		species.emplace_back(p->internal_name, p->species_id);
	std::sort(species.begin(), species.end(), [](const auto &a, const auto &b){ return a.first < b.first; });
	BufferReader buffer(packed_text_data, packed_text_data_size);
	while (!buffer.empty()){
		auto resource = this->parse_resource(species, buffer);
		if ((size_t)resource->id >= this->resources.size())
			this->resources.resize((size_t)resource->id);
		this->resources.emplace_back(std::move(resource));
	}
}

std::unique_ptr<TextResource> TextStore::parse_resource(const std::vector<std::pair<std::string, SpeciesId>> &species, BufferReader &buffer){
	if (buffer.remaining_bytes() < 4)
		throw std::runtime_error("TextStore::parse_command(): Parse error.");
	auto id = (TextResourceId)buffer.read_u32();
	auto ret = std::make_unique<TextResource>();
	ret->id = id;
	bool stop;
	do{
		auto command = this->parse_command(species, buffer, stop);
		if (command)
			ret->commands.emplace_back(std::move(command));
	}while (!stop);
	return ret;
}

std::unique_ptr<TextResourceCommand> TextStore::parse_command(const std::vector<std::pair<std::string, SpeciesId>> &species, BufferReader &buffer, bool &stop){
	auto command = (TextResourceCommandType)buffer.read_byte();
	stop = false;
	std::unique_ptr<TextResourceCommand> ret;
	switch (command){
		case TextResourceCommandType::End:
			stop = true;
			break;
		case TextResourceCommandType::Text:
			ret.reset(new TextCommand(buffer.read_string_as_vector()));
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
			ret.reset(new MemCommand((StringVariableId)buffer.read_varint()));
			break;
		case TextResourceCommandType::Num:
			{
				auto variable = (IntegerVariableId)buffer.read_varint();
				auto digits = buffer.read_u32();
				ret.reset(new NumCommand(variable, digits));
			}
			break;
		case TextResourceCommandType::Cry:
			ret.reset(new CryCommand((SpeciesId)buffer.read_varint()));
			break;
		default:
			throw std::runtime_error("TextStore::parse_command(): Invalid switch.");
	}
	return ret;
}

void TextResource::execute(Game &game, TextState &state){
	for (auto &p : this->commands)
		p->execute(game, state);
}

void TextStore::execute(Game &game, TextResourceId id, TextState &state){
	this->resources[(int)id]->execute(game, state);
}

template <typename T>
void progressively_write_text(const T &data, Game &game, TextState &state){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();

	auto tiles = renderer.get_tilemap(TileRegion::Window).tiles + state.position.x + state.position.y * Tilemap::w;
	for (auto c : data){
		tiles->tile_no = (typename std::make_unsigned<decltype(c)>::type)c;
		tiles++;
		state.position.x++;
		game.text_print_delay();
	}
}

void TextCommand::execute(Game &game, TextState &state){
	progressively_write_text(this->data, game, state);
}

void LineCommand::execute(Game &game, TextState &state){
	auto temp = state.start_of_line + Point{ 0, 2 };
	if (temp.y < state.box_corner.y + state.box_size.y)
		state.start_of_line = temp;
	state.position = state.start_of_line;
}

void TextResourceCommand::wait_for_continue(Game &game, TextState &state, bool display_arrow){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	auto tilemap = renderer.get_tilemap(TileRegion::Window).tiles;
	auto &tile = tilemap[state.continue_location.x + state.continue_location.y * Tilemap::w];
	auto &arrow_location = tile.tile_no;
	for (bool b = true;; b = !b){
		if (display_arrow)
			arrow_location = b ? down_arrow : ' ';
		if (game.check_for_user_interruption_no_auto_repeat(0.5))
			break;
	}
	if (display_arrow){
		arrow_location = ' ';
		game.get_audio_interface().play_sound(AudioResourceId::SFX_Press_AB);
	}
}

void ContCommand::execute(Game &game, TextState &state){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	auto tilemap = renderer.get_tilemap(TileRegion::Window).tiles;
	
	this->wait_for_continue(game, state);

	for (int i = 0; i < 2; i++){
		for (int y = 0; y < state.box_size.y - 1; y++){
			auto y0 = (state.box_corner.y + y) * Tilemap::w;
			auto y1 = y0 + Tilemap::w;
			for (int x = 0; x < state.box_size.x; x++)
				tilemap[state.box_corner.x + x + y0] = tilemap[state.box_corner.x + x + y1];
		}
		auto y0 = (state.box_corner.y + state.box_size.y - 1) * Tilemap::w;
		for (int x = 0; x < state.box_size.x; x++){
			auto &tile = tilemap[state.box_corner.x + x + y0];
			tile.tile_no = ' ';
		}
		Coroutine::get_current_coroutine().wait_frames(6);
	}
	state.position = state.start_of_line;
}

void ParaCommand::execute(Game &game, TextState &state){
	auto &engine = game.get_engine();
	auto &renderer = engine.get_renderer();
	auto tilemap = renderer.get_tilemap(TileRegion::Window).tiles;
	
	this->wait_for_continue(game, state);
	
	for (int y = 0; y < state.box_size.y; y++){
		auto y0 = (state.box_corner.y + y) * Tilemap::w;
		for (int x = 0; x < state.box_size.x; x++){
			auto &tile = tilemap[state.box_corner.x + x + y0];
			tile.tile_no = ' ';
		}
	}
	state.start_of_line = state.position = state.first_position;
}

void PromptCommand::execute(Game &game, TextState &state){
	this->wait_for_continue(game, state);
	DoneCommand::execute(game, state);
}

void DoneCommand::execute(Game &game, TextState &){
	game.delayed_reset_dialogue();
}

void DexCommand::execute(Game &game, TextState &state){
	char temp[] = {'.'};
	progressively_write_text(temp, game, state);
}

void AutocontCommand::execute(Game &, TextState &){}

void MemCommand::execute(Game &game, TextState &state){
	auto value = game.get_variable_store().get(this->variable);
	progressively_write_text(value, game, state);
}

void NumCommand::execute(Game &game, TextState &state){
	std::stringstream stream;
	stream << game.get_variable_store().get(this->variable);
	progressively_write_text(stream.str(), game, state);
}

void CryCommand::execute(Game &game, TextState &state){
	throw std::runtime_error("Not implemented.");
}

}
