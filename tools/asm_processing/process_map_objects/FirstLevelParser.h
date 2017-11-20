#pragma once
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <exception>

typedef std::deque<char> input_t;

#define POP pop(input)
#define PEEK input.front()

template <typename T>
void assume(T condition){
	if (!condition)
		throw std::exception();
}

template <typename T>
T pop(std::deque<T> &q){
	auto ret = std::move(q.front());
	q.pop_front();
	return ret;
}

namespace FirstLevelParser{

class Object{
public:
	virtual ~Object() = 0;
};

class Number : public Object{
public:
	virtual ~Number() = 0;
	virtual unsigned get_value() = 0;
	virtual std::string to_string() const = 0;
};

class IntegerLiteral : public Number{
	unsigned value;
public:
	IntegerLiteral(unsigned value): value(value){}
	unsigned get_value() override{
		return this->value;
	}
	std::string to_string() const override;
};

class Identifier : public Number{
	std::string id;
public:
	Identifier(const std::string &id): id(id){}
	unsigned get_value() override;
	const std::string &get_identifier() const{
		return this->id;
	}
	std::string to_string() const{
		return this->id;
	}
};

class MapObject : public Object{
	std::vector<std::shared_ptr<Number>> elements;
public:
	MapObject() = default;
	MapObject(const MapObject &) = delete;
	MapObject(MapObject &&other){
		this->elements = std::move(other.elements);
	}
	void operator=(const MapObject &) = delete;
	void operator=(MapObject &&) = delete;

	void add_element(std::shared_ptr<Number> &&element){
		this->elements.emplace_back(std::move(element));
	}
	const decltype(elements) &get_elements() const{
		return this->elements;
	}
};

class EventDisp : public Object{
	std::vector<std::shared_ptr<Number>> elements;
public:
	EventDisp() = default;
	EventDisp(const EventDisp &) = delete;
	EventDisp(EventDisp &&other){
		this->elements = std::move(other.elements);
	}
	void operator=(const EventDisp &) = delete;
	void operator=(EventDisp &&) = delete;

	void add_element(std::shared_ptr<Number> &&element){
		this->elements.emplace_back(std::move(element));
	}
	const decltype(elements) &get_elements() const{
		return this->elements;
	}
};

class Parser{
	std::string label;
	std::vector<std::shared_ptr<Object>> objects;

	void expect_file(input_t &input);
	static std::deque<std::shared_ptr<Number>> expect_bytes(input_t &input);
	static std::shared_ptr<MapObject> expect_map_object(input_t &input);
	static std::shared_ptr<EventDisp> expect_event_disp(input_t &input);
	static std::shared_ptr<Number> expect_single_number(input_t &input);
	static void expect_number_list(std::deque<std::shared_ptr<Number>> &dst, input_t &input);
	static std::deque<std::shared_ptr<Number>> expect_non_empty_number_list(input_t &input);
	static unsigned expect_dec_number(input_t &input);
	static unsigned expect_hex_number(input_t &input);
	static std::string expect_label(input_t &input);
	static std::string expect_identifier(input_t &input);
	static void expect_character(input_t &input, char c);
	static void end_line(input_t &input);
	template <typename T>
	static char expect_character_class(input_t &input, const T &f){
		assume(input.size());
		auto ret = POP;
		assume(f(ret));
		return ret;
	}
	template <typename T>
	static bool maybe_character_class(char &dst, input_t &input, const T &f){
		if (!input.size() || !f(PEEK))
			return false;
		dst = POP;
		return true;
	}
	template <typename T>
	static void skip_character_class(input_t &input, const T &f){
		char c;
		while (maybe_character_class(c, input, f));
	}
	static void skip_whitespace(input_t &input){
		skip_character_class(input, is_whitespace);
	}
	static void skip_whitespace_and_empty_lines(input_t &input);
	static bool is_newline(char c){
		return c == '\n';
	}
	static bool is_whitespace(char c){
		return c == ' ';
	}
	static bool is_whitespace_or_newline(char c){
		return is_whitespace(c) || is_newline(c);
	}
	static bool is_identifier_first(char c){
		return isalpha(c) || c == '_';
	}
	static bool is_identifier_nth(char c){
		return is_identifier_first(c) || isdigit(c);
	}
	static bool is_hex_digit(char c){
		if (isdigit(c))
			return true;
		if (!isalpha(c))
			return false;
		c = tolower(c);
		return c >= 'a' && c <= 'f';
	}
	static unsigned hex_digit_to_value(char c){
		assume(is_hex_digit(c));
		if (isdigit(c))
			return c - '0';
		c = tolower(c);
		return c - 'a' + 10;
	}
	template <typename F, typename T>
	static bool accept(T &dst, input_t &input, const F &f){
		auto copy = input;
		std::deque<std::shared_ptr<Number>> temp;
		try{
			dst = std::move(f(copy));
		}catch (std::exception &){
			return false;
		}
		input = std::move(copy);
		return true;
	}
public:
	Parser(const std::string &path);
	const std::string &get_label() const{
		return this->label;
	}
	const decltype(objects) &get_objects() const{
		return this->objects;
	}
};

}
