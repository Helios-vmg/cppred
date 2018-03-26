#pragma once
#ifndef HAVE_PCH
#include <vector>
#include <algorithm>
#include <initializer_list>
#endif

template <typename T>
class StaticSet{
	std::vector<T> data;
public:
	StaticSet() = delete;
	StaticSet(const StaticSet &) = delete;
	StaticSet(StaticSet &&) = delete;
	void operator=(const StaticSet &) = delete;
	void operator=(const StaticSet &&) = delete;
	StaticSet(std::initializer_list<T> &&data): data(std::move(data)){
		std::sort(this->data.begin(), this->data.end());
	}
	bool contains(const T &x) const{
		return std::binary_search(this->data.begin(), this->data.end(), x);
	}
};
