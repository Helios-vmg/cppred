#pragma once

#include <fstream>
#include <stdexcept>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <cassert>
#include <cstdint>
#include <limits>

inline std::deque<std::string> file_splitter(std::ifstream &file){
	std::deque<std::string> ret;
	if (!file)
		return ret;
	file.seekg(0, std::ios::end);
	if (file.tellg() > std::numeric_limits<size_t>::max())
		return ret;
	std::vector<std::uint8_t> data((size_t)file.tellg());
	file.seekg(0);
	file.read((char *)&data[0], data.size());

	std::string accum;
	bool cr_seen = false;
	for (auto c : data){
		if (cr_seen){
			ret.emplace_back(std::move(accum));
			cr_seen = false;
			if (c == 10)
				continue;
		}
		switch (c){
			case 10:
				ret.emplace_back(std::move(accum));
				break;
			case 13:
				cr_seen = true;
				break;
			default:
				accum.push_back(c);
				break;
		}
	}
	ret.emplace_back(std::move(accum));
	return ret;
}

template <typename T>
auto move_pop_front(T &x){
	auto ret = std::move(x.front());
	x.pop_front();
	return ret;
}

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
		std::ifstream file(path, std::ios::binary);
		if (!file)
			throw std::runtime_error((std::string)"CsvParser::CsvParser(): Can't open file " + path);
		
		auto lines = file_splitter(file);
		if (!lines.size())
			throw std::runtime_error((std::string)"CsvParser::CsvParser(): Invalid file: " + path);
		auto headers = split_csv_line(lines.front());
		lines.pop_front();
		for (size_t i = 0; i < headers.size(); i++)
			this->headers[headers[i]] = i;

		while (lines.size()){
			auto line = move_pop_front(lines);
			if (!line.size())
				continue;
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
