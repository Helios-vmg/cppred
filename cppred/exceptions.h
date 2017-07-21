#pragma once
#include <exception>
#include <stdexcept>

#if defined _MSC_VER
#if _MSC_VER >= 1900
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif
#else
#define NOEXCEPT noexcept
#endif

class GameBoyException : public std::runtime_error{
public:
	GameBoyException(const std::string &s): std::runtime_error(s){}
	virtual ~GameBoyException(){}
	virtual GameBoyException *clone() = 0;
};

class GenericException : public GameBoyException{
public:
	GenericException(const std::string &s): GameBoyException(s){}
	virtual ~GenericException(){}
	virtual GameBoyException *clone() override{
		return new GenericException(this->what());
	}
};

class NotImplementedException : public GenericException{
public:
	NotImplementedException(): GenericException("Not implemented."){}
	GameBoyException *clone() override{
		return new NotImplementedException();
	}
};
