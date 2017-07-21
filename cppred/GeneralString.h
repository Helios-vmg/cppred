#pragma once

#include <string>
#include <memory>

class GeneralString{
public:
	virtual ~GeneralString(){}
	typedef std::shared_ptr<GeneralString> ptr_t;
	virtual ptr_t clone() const = 0;
	virtual void operator+=(const ptr_t &) = 0;
	virtual void operator+=(const char *) = 0;
	virtual void operator+=(char) = 0;
	ptr_t operator+(const ptr_t &p) const{
		auto ret = this->clone();
		*ret += p;
		return ret;
	}
	virtual ptr_t operator+(const char *p) const{
		auto ret = this->clone();
		*ret += p;
		return ret;
	}
	virtual ptr_t operator+(char c) const{
		auto ret = this->clone();
		*ret += c;
		return ret;
	}
	virtual ptr_t get_filename() const = 0;
	virtual ptr_t get_directory() const = 0;
	virtual ptr_t append_path_part(const ptr_t &) const = 0;
	virtual ptr_t remove_extension() const = 0;
};

template <typename T, size_t N>
size_t find_last_of(const std::basic_string<T> &s, const char (&search)[N], size_t off = std::string::npos){
	T search2[N];
	std::copy(search, search + N, search2);
	return s.find_last_of(search2, off);
}

template <typename T, size_t N>
size_t find_last_not_of(const std::basic_string<T> &s, const char(&search)[N], size_t off = std::string::npos){
	T search2[N];
	std::copy(search, search + N, search2);
	return s.find_last_not_of(search2, off);
}

template <typename T>
class StdBasicString : public GeneralString{
	std::basic_string<T> str;
#define DIRECTORY_SEPARATORS "/\\"
public:
	StdBasicString() = default;
	StdBasicString(const std::basic_string<T> &str): str(str){}
	StdBasicString(const StdBasicString<T> &str): str(str.str){}
	ptr_t clone() const override{
		return ptr_t(new StdBasicString<T>(*this));
	}
	void operator+=(const ptr_t &p) override{
		if (!p)
			return;
		auto sbs = std::dynamic_pointer_cast<StdBasicString<T>>(p);
		if (sbs)
			this->str += sbs->str;
	}
	void operator+=(const char *p) override{
		for (; *p; p++)
			this->str += *p;
	}
	void operator+=(char c) override{
		this->str += c;
	}
	ptr_t get_filename() const override{
		auto slash = find_last_of(this->str, DIRECTORY_SEPARATORS);
		if (slash == this->str.npos)
			return ptr_t(new StdBasicString(this->str));
		return ptr_t(new StdBasicString(this->str.substr(slash + 1)));
	}
	ptr_t get_directory() const override{
		auto slash = find_last_of(this->str, DIRECTORY_SEPARATORS);
		if (slash == this->str.npos)
			return ptr_t(new StdBasicString());
		auto not_slash = find_last_not_of(this->str, DIRECTORY_SEPARATORS, slash);
		auto begin = this->str.begin();
		auto end = begin + slash + 1;
		if (not_slash != this->str.npos)
			end = begin + not_slash + 1;
		return ptr_t(new StdBasicString(std::basic_string<T>(begin, end)));
	}
	ptr_t append_path_part(const ptr_t &p) const override{
		std::shared_ptr<StdBasicString<T>> ret(new StdBasicString<T>(*this));
		auto &s = ret->str;
		auto sbs = std::dynamic_pointer_cast<StdBasicString<T>>(p);
		if (!s.size())
			s = sbs->str;
		else{
			while (s.size() && (s.back() == '/' || s.back() == '\\'))
				s.pop_back();
			if (sbs){
				s += '/';
				s += sbs->str;
			}
		}
		return ret;
	}
	ptr_t remove_extension() const override{
		auto dot = this->str.rfind('.');
		if (dot == this->str.npos)
			return this->clone();
		return ptr_t(new StdBasicString(this->str.substr(0, dot)));
	}

	const std::basic_string<T> get_std_basic_string() const{
		return this->str;
	}
};

typedef GeneralString::ptr_t path_t;
