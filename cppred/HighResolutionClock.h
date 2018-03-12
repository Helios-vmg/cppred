#pragma once
#include <memory>

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
	AbstractClock *parent_clock;
	double reference_time = -1;
	double current_time = 0;
public:
	SteppingClock(AbstractClock &parent): parent_clock(&parent){}
	virtual ~SteppingClock(){}
	virtual void step();
	double get() override{
		return this->current_time;
	}
	double get() const{
		return this->current_time;
	}
};

class PausableClock : public SteppingClock{
	double accumulated_time = 0;
	bool paused = true;
public:
	PausableClock(AbstractClock &parent): SteppingClock(parent){}
	void step() override;
	void pause();
	void resume();
};
