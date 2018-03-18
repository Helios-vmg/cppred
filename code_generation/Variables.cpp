#include "Variables.h"
#include "../common/csv_parser.h"

extern const char * const variables_file;

Variable Variables::get(const std::string &name){
	this->init();
	auto it = this->variables.find(name);
	if (it == this->variables.end())
		throw std::runtime_error("Variable " + name + " not defined.");
	
	return it->second;
}

static std::map<std::string, bool> load_variables_file(){
	std::map<std::string, bool> ret;
	CsvParser csv(variables_file);
	static const std::vector<std::string> order = {"name", "type"};

	for (auto i = csv.row_count(); i--;){
		auto columns = csv.get_ordered_row(i, order);
		bool is_string;
		if (columns[1] == "int")
			is_string = false;
		else if (columns[1] == "string")
			is_string = true;
		else
			throw std::runtime_error("Error: variable " + columns[0] + " has invalid type " + columns[1] + ". Must be int or string.");
		ret[columns[0]] = is_string;
	}
	return ret;
}

void Variables::init(){
	if (this->initialized)
		return;
	this->initialized = true;

	unsigned iid = 0,
		sid = 0;
	for (auto &kv : load_variables_file()){
		auto id = !kv.second ? iid++ : sid++;
		this->variables[kv.first] = {id, kv.second};
	}
}

const std::map<std::string, Variable> &Variables::get_map(){
	this->init();
	return this->variables;
}
