#pragma once
#include "FirstLevelParser.h"
#include "../../../common/json.hpp"

class HiddenObjectParser{

	class Object{
	public:
		virtual ~Object() = 0;
	};

	class Label : public Object{
		std::string label;
	public:
		Label(const std::string &s): label(s){}
		const std::string &get_label() const{
			return this->label;
		}
	};

	class CompiledExpression;

	class ExpressionToken{
	public:
		virtual ~ExpressionToken(){}
		virtual std::string to_string() const = 0;
		virtual unsigned eval(CompiledExpression *left, CompiledExpression *right) = 0;
		virtual unsigned eval(CompiledExpression *left, CompiledExpression *right, const std::map<std::string, unsigned> &) = 0;
	};

	class Expression : public Object{
	public:
		virtual ~Expression() = 0;
	};

	class CompiledExpression : public Expression{
		std::shared_ptr<ExpressionToken> op;
		std::shared_ptr<CompiledExpression> left, right;
	public:
		CompiledExpression() = default;
		CompiledExpression(const std::shared_ptr<ExpressionToken> &op, const std::shared_ptr<CompiledExpression> &left, const std::shared_ptr<CompiledExpression> &right):
			op(op),
			left(left),
			right(right){}
		virtual unsigned eval();
		virtual unsigned eval(const std::map<std::string, unsigned> &);
	};

	class LinearExpression : public Expression{
		std::vector<std::shared_ptr<ExpressionToken>> tokens;
	public:
		virtual ~LinearExpression(){}
		void add(const std::shared_ptr<ExpressionToken> &token){
			this->tokens.push_back(token);
		}
		size_t size() const{
			return this->tokens.size();
		}
		std::shared_ptr<CompiledExpression> compile();
	};

	class ExpressionOperator : public ExpressionToken{
		char c;
	public:
		ExpressionOperator(char c): c(c){}
		std::string to_string() const{
			return std::string(1, this->c);
		}
		char get_character() const{
			return this->c;
		}
		unsigned eval(CompiledExpression *left, CompiledExpression *right) override;
		unsigned eval(CompiledExpression *left, CompiledExpression *right, const std::map<std::string, unsigned> &) override;
	};

	class Number : public CompiledExpression, public ExpressionToken{
	public:
		virtual ~Number() = 0;
		unsigned eval(CompiledExpression *left, CompiledExpression *right) override{
			return this->eval();
		}
		unsigned eval(CompiledExpression *left, CompiledExpression *right, const std::map<std::string, unsigned> &constants){
			return CompiledExpression::eval(constants);
		}
		unsigned eval() override;
		virtual unsigned get_value() = 0;
	};

	class IntegerLiteral : public Number{
		unsigned value;
	public:
		IntegerLiteral(unsigned value): value(value){}
		unsigned get_value() override{
			return this->value;
		}
		std::string to_string() const override;
		unsigned eval(const std::map<std::string, unsigned> &) override{
			return this->value;
		}
	};

	class Identifier : public Number{
		std::string id;
	public:
		Identifier(const std::string &id): id(id){}
		unsigned get_value() override;
		std::string to_string() const{
			return this->id;
		}
		unsigned eval(const std::map<std::string, unsigned> &) override;
	};

	std::vector<std::shared_ptr<Object>> objects;

	void expect_file(input_t &input);
	static std::deque<std::shared_ptr<Expression>> expect_numbers(input_t &input);
	static std::deque<std::shared_ptr<Expression>> expect_bdw(input_t &input);
	static std::shared_ptr<Expression> expect_expression(input_t &input);
	static std::shared_ptr<Number> expect_single_number(input_t &input);
	static int expect_bank_macro(input_t &input);
	static void expect_number_list(std::deque<std::shared_ptr<Expression>> &dst, input_t &input);
	static std::deque<std::shared_ptr<Expression>> expect_non_empty_number_list(input_t &input);
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
	HiddenObjectParser(const std::string &path);
	std::map<std::string, nlohmann::json> serialize();
};
