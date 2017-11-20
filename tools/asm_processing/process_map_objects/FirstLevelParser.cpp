#include "FirstLevelParser.h"
#include "utility.h"
#include <fstream>
#include <sstream>

namespace FirstLevelParser{

Object::~Object(){}

Number::~Number(){}

std::string Parser::expect_label(input_t &input){
	skip_whitespace(input);
	auto ret = expect_identifier(input);
	skip_whitespace(input);
	expect_character(input, ':');
	skip_whitespace(input);
	expect_character_class(input, is_newline);
	return ret;
}

std::string Parser::expect_identifier(input_t &input){
	std::string ret;
	ret += expect_character_class(input, is_identifier_first);
	char c;
	while (maybe_character_class(c, input, is_identifier_nth))
		ret += c;
	return ret;
}

void Parser::expect_character(input_t &input, char c){
	assume(input.size() && POP == c);
}

void Parser::end_line(input_t &input){
	skip_whitespace(input);
	if (!input.size())
		return;
	if (PEEK == ';')
		skip_character_class(input, [](auto c){ return !is_newline(c); });
}

void Parser::skip_whitespace_and_empty_lines(input_t &input){
	while (true){
		skip_whitespace(input);
		if (!input.size())
			return;
		if (PEEK != ';' && !is_newline(PEEK))
			break;
		skip_character_class(input, [](auto c){ return !is_newline(c); });
		expect_character_class(input, is_newline);
	}
}

std::deque<std::shared_ptr<Number>> Parser::expect_bytes(input_t &input){
	assume(expect_identifier(input) == "db");
	skip_whitespace(input);
	return expect_non_empty_number_list(input);
}

std::deque<std::shared_ptr<Number>> Parser::expect_non_empty_number_list(input_t &input){
	std::deque<std::shared_ptr<Number>> ret;
	ret.push_back(expect_single_number(input));
	expect_number_list(ret, input);
	return ret;
}

std::shared_ptr<Number> Parser::expect_single_number(input_t &input){
	assume(input.size());
	if (isdigit(PEEK))
		return std::make_unique<IntegerLiteral>(expect_dec_number(input));
	if (PEEK == '$'){
		input.pop_front();
		return std::make_unique<IntegerLiteral>(expect_hex_number(input));
	}
	if (is_identifier_first(PEEK))
		return std::make_unique<Identifier>(expect_identifier(input));

	assume(false);
	return nullptr;
}

void Parser::expect_number_list(std::deque<std::shared_ptr<Number>> &dst, input_t &input){
	while (true){
		end_line(input);
		if (!input.size() || is_newline(PEEK))
			return;
		expect_character(input, ',');
		skip_whitespace(input);
		dst.push_back(expect_single_number(input));
	}
}

unsigned Parser::expect_dec_number(input_t &input){
	assume(input.size() && isdigit(PEEK));
	unsigned ret = 0;
	do{
		ret *= 10;
		ret += POP - '0';
	} while (input.size() && isdigit(PEEK));
	return ret;
}

unsigned Parser::expect_hex_number(input_t &input){
	assume(input.size() && is_hex_digit(PEEK));
	unsigned ret = 0;
	do{
		ret *= 16;
		ret += hex_digit_to_value(POP);
	} while (input.size() && is_hex_digit(PEEK));
	return ret;
}

std::shared_ptr<MapObject> Parser::expect_map_object(input_t &input){
	assume(expect_identifier(input) == "object");
	skip_whitespace(input);
	auto ret = std::make_unique<MapObject>();
	for (auto &p : expect_non_empty_number_list(input))
		ret->add_element(std::move(p));
	return ret;
}

std::shared_ptr<EventDisp> Parser::expect_event_disp(input_t &input){
	assume(expect_identifier(input) == "EVENT_DISP");
	skip_whitespace(input);
	auto ret = std::make_unique<EventDisp>();
	for (auto &p : expect_non_empty_number_list(input))
		ret->add_element(std::move(p));
	return ret;
}

void Parser::expect_file(input_t &input){
	if (!input.size())
		return;
	this->label = expect_label(input);
	while (true){
		skip_whitespace_and_empty_lines(input);
		if (!input.size())
			break;
		std::deque<std::shared_ptr<Number>> bytes;
		std::shared_ptr<MapObject> map_object;
		std::shared_ptr<EventDisp> event_disp;
		if (accept(bytes, input, expect_bytes)){
			for (auto &p : bytes)
				this->objects.emplace_back(std::move(p));
		}else if (accept(map_object, input, expect_map_object)){
			this->objects.emplace_back(std::move(map_object));
		}else if (accept(event_disp, input, expect_event_disp)){
			this->objects.emplace_back(std::move(event_disp));
		}else
			assume(false);
	}
}

Parser::Parser(const std::string &path){
	auto queue = normalize_file_contents(load_file(path));
	this->expect_file(queue);
}

std::string IntegerLiteral::to_string() const{
	std::stringstream stream;
	stream << this->value;
	return stream.str();
}

unsigned Identifier::get_value(){
	throw std::runtime_error("Attempt to get value of identifier " + this->id);
	return 0;
}

}
