#pragma once
#include <memory>

class HighResolutionClock{
	double resolution;
public:
	HighResolutionClock();
	double get();
};
