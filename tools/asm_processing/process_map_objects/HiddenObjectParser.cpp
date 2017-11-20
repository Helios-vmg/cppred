#include "HiddenObjectParser.h"
#include "utility.h"
#include "data.h"
#include <sstream>

HiddenObjectParser::HiddenObjectParser(const std::string &path){
	auto queue = normalize_file_contents(load_file(path));
	this->expect_file(queue);
}

HiddenObjectParser::Object::~Object(){}
HiddenObjectParser::Expression::~Expression(){}
HiddenObjectParser::Number::~Number(){}

std::string HiddenObjectParser::IntegerLiteral::to_string() const{
	std::stringstream stream;
	stream << this->value;
	return stream.str();
}

unsigned HiddenObjectParser::Identifier::get_value(){
	throw std::runtime_error("Attempt to get value of identifier " + this->id);
	return 0;
}

void HiddenObjectParser::expect_file(input_t &input){
	if (!input.size())
		return;
	while (true){
		skip_whitespace_and_empty_lines(input);
		if (!input.size())
			break;
		std::string label;
		std::deque<std::shared_ptr<Expression>> expressions;
		if (accept(label, input, expect_label))
			this->objects.emplace_back(new Label(label));
		else if (accept(expressions, input, expect_numbers) || accept(expressions, input, expect_bdw)){
			for (auto &p : expressions)
				this->objects.emplace_back(std::move(p));
		}else
			assume(false);
	}
}

std::deque<std::shared_ptr<HiddenObjectParser::Expression>> HiddenObjectParser::expect_numbers(input_t &input){
	auto id = expect_identifier(input);
	assume(id == "db" || id == "dw");
	skip_whitespace(input);
	return expect_non_empty_number_list(input);
}

std::deque<std::shared_ptr<HiddenObjectParser::Expression>> HiddenObjectParser::expect_bdw(input_t &input){
	assume(expect_identifier(input) == "dbw");
	skip_whitespace(input);
	return expect_non_empty_number_list(input);
}

std::shared_ptr<HiddenObjectParser::Expression> HiddenObjectParser::expect_expression(input_t &input){
	auto temp = std::make_shared<LinearExpression>();
	while (true){
		skip_whitespace(input);
		if (!input.size() || PEEK == ',' || PEEK == ';' || is_newline(PEEK))
			break;
		if (PEEK == '(' || PEEK == ')' || PEEK == '+' || PEEK == '-' || PEEK == '/'){
			temp->add(std::make_shared<ExpressionOperator>(POP));
			continue;
		}
		auto number = expect_single_number(input);
		if (!number)
			continue;
		temp->add(number);
	}
	if (!temp->size())
		return nullptr;
	return temp->compile();
}

std::shared_ptr<HiddenObjectParser::Number> HiddenObjectParser::expect_single_number(input_t &input){
	assume(input.size());
	if (isdigit(PEEK))
		return std::make_unique<IntegerLiteral>(expect_dec_number(input));
	if (PEEK == '$'){
		input.pop_front();
		return std::make_unique<IntegerLiteral>(expect_hex_number(input));
	}
	if (is_identifier_first(PEEK)){
		int i;
		if (accept(i, input, expect_bank_macro))
			return std::make_unique<IntegerLiteral>(0);
		return std::make_unique<Identifier>(expect_identifier(input));
	}

	assume(false);
	return nullptr;
}

int HiddenObjectParser::expect_bank_macro(input_t &input){
	auto id = expect_identifier(input);
	assume(id == "BANK" || id == "Bank");
	skip_whitespace(input);
	expect_character(input, '(');
	expect_identifier(input);
	skip_whitespace(input);
	expect_character(input, ')');
	return 0;
}

void HiddenObjectParser::expect_number_list(std::deque<std::shared_ptr<Expression>> &dst, input_t &input){
	while (true){
		end_line(input);
		if (!input.size() || is_newline(PEEK))
			return;
		expect_character(input, ',');
		skip_whitespace(input);
		auto expr = expect_expression(input);
		if (!expr)
			continue;
		dst.push_back(expr);
	}
}

std::deque<std::shared_ptr<HiddenObjectParser::Expression>> HiddenObjectParser::expect_non_empty_number_list(input_t &input){
	std::deque<std::shared_ptr<Expression>> ret;
	auto expr = expect_expression(input);
	if (expr)
		ret.push_back(expr);
	expect_number_list(ret, input);
	return ret;
}

unsigned HiddenObjectParser::expect_dec_number(input_t &input){
	assume(input.size() && isdigit(PEEK));
	unsigned ret = 0;
	do{
		ret *= 10;
		ret += POP - '0';
	} while (input.size() && isdigit(PEEK));
	return ret;
}

unsigned HiddenObjectParser::expect_hex_number(input_t &input){
	assume(input.size() && is_hex_digit(PEEK));
	unsigned ret = 0;
	do{
		ret *= 16;
		ret += hex_digit_to_value(POP);
	}while (input.size() && is_hex_digit(PEEK));
	return ret;
}

std::string HiddenObjectParser::expect_label(input_t &input){
	skip_whitespace(input);
	auto ret = expect_identifier(input);
	skip_whitespace(input);
	expect_character(input, ':');
	skip_whitespace(input);
	expect_character_class(input, is_newline);
	return ret;
}

std::string HiddenObjectParser::expect_identifier(input_t &input){
	std::string ret;
	ret += expect_character_class(input, is_identifier_first);
	char c;
	while (maybe_character_class(c, input, is_identifier_nth))
		ret += c;
	return ret;
}

void HiddenObjectParser::expect_character(input_t &input, char c){
	assume(input.size() && POP == c);
}

void HiddenObjectParser::end_line(input_t &input){
	skip_whitespace(input);
	if (!input.size())
		return;
	if (PEEK == ';')
		skip_character_class(input, [](auto c){ return !is_newline(c); });
}

void HiddenObjectParser::skip_whitespace_and_empty_lines(input_t &input){
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

unsigned HiddenObjectParser::ExpressionOperator::eval(CompiledExpression *left, CompiledExpression *right){
	switch (this->c){
		case '(':
			assume(left);
			return left->eval();
		case '+':
			assume(left && right);
			return left->eval() + right->eval();
		case '-':
			assume(left && right);
			return left->eval() - right->eval();
		case '*':
			assume(left && right);
			return left->eval() * right->eval();
		case '/':
			assume(left && right);
			return left->eval() / right->eval();
	}
	assume(false);
	return 0;
}

unsigned HiddenObjectParser::ExpressionOperator::eval(CompiledExpression *left, CompiledExpression *right, const std::map<std::string, unsigned> &constants){
	switch (this->c){
		case '(':
			assume(left);
			return left->eval(constants);
		case '+':
			assume(left && right);
			return left->eval(constants) + right->eval(constants);
		case '-':
			assume(left && right);
			return left->eval(constants) - right->eval(constants);
		case '*':
			assume(left && right);
			return left->eval(constants) * right->eval(constants);
		case '/':
			assume(left && right);
			return left->eval(constants) / right->eval(constants);
	}
	assume(false);
	return 0;
}

unsigned HiddenObjectParser::CompiledExpression::eval(){
	return this->op->eval(this->left.get(), this->right.get());
}

unsigned HiddenObjectParser::CompiledExpression::eval(const std::map<std::string, unsigned> &constants){
	return this->op->eval(this->left.get(), this->right.get(), constants);
}

unsigned HiddenObjectParser::Number::eval(){
	return this->get_value();
}

unsigned HiddenObjectParser::Identifier::eval(const std::map<std::string, unsigned> &constants){
	auto it = constants.find(this->id);
	assume(it != constants.end());
	return it->second;
}

static int precedence(char c){
	switch (c){
		case '(':
			return 0;
		case ')':
			return 1;
		case '+':
		case '-':
			return 2;
		case '*':
		case '/':
			return 3;
	}
	assume(false);
	return -1;
}

std::shared_ptr<HiddenObjectParser::CompiledExpression> HiddenObjectParser::LinearExpression::compile(){
	std::vector<std::shared_ptr<ExpressionOperator>> operator_stack;
	std::vector<std::shared_ptr<ExpressionToken>> output;

	for (auto &token : this->tokens){
		auto number = std::dynamic_pointer_cast<Number>(token);
		if (number){
			output.push_back(number);
			continue;
		}
		auto op = std::dynamic_pointer_cast<ExpressionOperator>(token);
		assume(op);

		if (op->get_character() == '('){
			operator_stack.push_back(op);
			continue;
		}
		if (op->get_character() == ')'){
			while (true){
				assume(operator_stack.size());
				if (operator_stack.back()->get_character() == '('){
					operator_stack.pop_back();
					break;
				}
				output.push_back(operator_stack.back());
				operator_stack.pop_back();
			}
			continue;
		}

		auto p = precedence(op->get_character());
		while (operator_stack.size()){
			if (precedence(operator_stack.back()->get_character()) < p)
				break;
			output.push_back(operator_stack.back());
			operator_stack.pop_back();
		}
		operator_stack.push_back(op);
	}

	while (operator_stack.size()){
		output.push_back(operator_stack.back());
		operator_stack.pop_back();
	}

	std::vector<std::shared_ptr<CompiledExpression>> final_stack;
	for (auto &o : output){
		auto number = std::dynamic_pointer_cast<Number>(o);
		if (number){
			final_stack.push_back(number);
			continue;
		}
		auto op = std::dynamic_pointer_cast<ExpressionOperator>(o);
		assume(op && final_stack.size() >= 2);
		auto right = final_stack.back();
		final_stack.pop_back();
		auto left = final_stack.back();
		final_stack.pop_back();
		final_stack.emplace_back(new CompiledExpression(op, left, right));
	}
	assume(final_stack.size() == 1);
	return final_stack.back();
}

std::map<std::string, nlohmann::json> HiddenObjectParser::serialize(){
	std::vector<std::string> keys, values;
	{
		bool reading = false;
		for (auto &o : this->objects){
			if (!reading){
				auto label = std::dynamic_pointer_cast<Label>(o);
				if (label && label->get_label() != "HiddenObjectMaps")
					continue;
				reading = true;
				continue;
			}
			auto i = std::dynamic_pointer_cast<IntegerLiteral>(o);
			if (i && i->get_value() == 0xFF)
				break;
			auto id = std::dynamic_pointer_cast<Identifier>(o);
			if (!id){
				keys.push_back("");
				continue;
			}
			auto it = map_map.find(id->to_string());
			assume(it != map_map.end());
			keys.push_back(it->second);
		}
	}
	{
		bool reading = false;
		for (auto &o : this->objects){
			if (values.size() == keys.size())
				break;
			if (!reading){
				auto label = std::dynamic_pointer_cast<Label>(o);
				if (!label || label->get_label() != "HiddenObjectPointers")
					continue;
				reading = true;
				continue;
			}
			auto id = std::dynamic_pointer_cast<Identifier>(o);
			assume(id);
			values.push_back(id->to_string());
		}
	}
	assume(keys.size() == values.size());

	std::map<std::string, std::string> map;
	for (size_t i = 0; i < values.size(); i++){
		if (!keys[i].size())
			continue;
		auto it = map_objects_map.find(keys[i]);
		assume(it != map_objects_map.end());
		map[values[i]] = it->second;
	}

	typedef nlohmann::json J;
	typedef nlohmann::json::array_t A;
	std::map<std::string, J> ret;
	J *current = nullptr;
	bool skipping = false;
	auto constants = ::constants;
	std::map<unsigned, std::string> remapped_constants;
	const unsigned remap_base = 1 << 20;
	unsigned i = remap_base;
	for (auto &kv : item_constants){
		remapped_constants[i] = kv.second;
		constants[kv.first] = i;
		i++;
	}

	for (size_t i = 0; i < this->objects.size(); i++){
		auto label = std::dynamic_pointer_cast<Label>(this->objects[i]);
		if (!label && skipping)
			continue;
		if (label){
			auto it = map.find(label->get_label());
			if (it == map.end()){
				skipping = true;
				continue;
			}
			current = &(ret[it->second] = A());
			skipping = false;
			continue;
		}

		auto y =      std::dynamic_pointer_cast<CompiledExpression>(this->objects[i + 0]);
		if (y->eval() == 0xFF)
			continue;
		auto x =      std::dynamic_pointer_cast<CompiledExpression>(this->objects[i + 1]);
		auto param =  std::dynamic_pointer_cast<CompiledExpression>(this->objects[i + 2]);
		auto script = std::dynamic_pointer_cast<Identifier>(this->objects[i + 4]);
		i += 4;

		J j;
		j["type"] = "hidden";
		j["x"] = x->eval();
		j["y"] = y->eval();
		auto constant = param->eval(constants);
		if (constant < remap_base)
			j["param"] = constant;
		else
			j["param"] = remapped_constants[constant];
		j["script"] = script->to_string();
		current->push_back(j);
	}

	return ret;
}
