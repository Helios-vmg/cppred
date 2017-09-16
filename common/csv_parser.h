#pragma once

#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <cassert>

class CsvParser{
	std::map<std::string, size_t> headers;
	std::vector<std::string> data;

	static std::vector<std::string> split_csv_line(const std::string &line, size_t minimum_size = 0){
		std::vector<std::string> ret;
		std::string accum;
		int string_state = 0;
		for (auto c : line){
			if (c == '"'){
				if (!string_state){
					string_state = 1;
					continue;
				}
				string_state = 2;
				continue;
			}
			if (c == ','){
				if (string_state == 2)
					string_state = 0;
				if (!string_state){
					ret.push_back(accum);
					accum.clear();
					continue;
				}
			}
			accum.push_back(c);
		}
		if (accum.size())
			ret.push_back(accum);
		if (ret.size() < minimum_size)
			ret.resize(minimum_size);
		return ret;
	}
public:
	CsvParser(const char *path){
		std::ifstream file(path);
		if (!file)
			throw std::runtime_error((std::string)"CsvParser::CsvParser(): Can't open file " + path);
		
		std::string line;
		std::getline(file, line);
		if (!file)
			throw std::runtime_error((std::string)"CsvParser::CsvParser(): Invalid file: " + path);
		auto headers = split_csv_line(line);
		for (size_t i = 0; i < headers.size(); i++)
			this->headers[headers[i]] = i;

		while (true){
			std::getline(file, line);
			if (!file)
				break;
			auto processed = split_csv_line(line);
			processed.resize(this->headers.size());
			for (auto &s : processed)
				this->data.emplace_back(std::move(s));
		}
	}
	size_t row_count() const{
		assert(this->data.size() % this->headers.size() == 0);
		return this->data.size() / this->headers.size();
	}
	std::string get_cell(size_t row, size_t column) const{
		if (row > this->row_count())
			throw std::runtime_error("CsvParser::get_cell(): Invalid row.");
		if (column > this->headers.size())
			throw std::runtime_error("CsvParser::get_cell(): Invalid column.");
		size_t index = row * this->headers.size() + column;
		return this->data[index];
	}
	std::string get_cell(size_t row, const std::string &column) const{
		if (row > this->row_count())
			throw std::runtime_error("CsvParser::get_cell(): Invalid row.");
		auto it = this->headers.find(column);
		if (it == this->headers.end())
			throw std::runtime_error("CsvParser::get_cell(): Invalid column: " + column);
		return this->get_cell(row, it->second);
	}
	std::vector<std::string> get_ordered_row(size_t row, const std::vector<std::string> &desired_order){
		std::vector<std::string> ret;
		ret.reserve(desired_order.size());
		for (auto &i : desired_order)
			ret.push_back(this->get_cell(row, i));
		return ret;
	}
};
