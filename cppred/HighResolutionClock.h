#pragma once
#ifndef HAVE_PCH
#include <memory>
#include <string>
#endif

class AbstractClock{
public:
	virtual ~AbstractClock(){}
	virtual double get() = 0;
};

class HighResolutionClock : public AbstractClock{
	std::uint64_t reference_time;
	double resolution;
public:
	HighResolutionClock();
	double get() override;
};

class SteppingClock : public AbstractClock{
protected:
	std::string name;
	AbstractClock *parent_clock;
	double reference_time = -1;
	double current_time = 0;
public:
	SteppingClock(AbstractClock &parent, const std::string &name = std::string()): name(name), parent_clock(&parent){}
	virtual ~SteppingClock(){}
	virtual void step();
	double get() override{
		return this->current_time;
	}
	double get() const{
		return this->current_time;
	}
};

class FixedClock : public SteppingClock{
	std::uint64_t clock = 0;
public:
	FixedClock(const std::string &name = std::string()): SteppingClock(*this, name){}
	void step() override;
};

class PausableClock : public SteppingClock{
	double accumulated_time = 0;
	bool paused = true;
public:
	PausableClock(AbstractClock &parent, const std::string &name = std::string()): SteppingClock(parent, name){}
	void step() override;
	void pause();
	void resume();
};
