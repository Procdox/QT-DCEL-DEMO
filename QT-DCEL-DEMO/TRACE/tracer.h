#pragma once
#include "DCEL/Grid_Point.h"

using Buffer = std::list<std::pair<Pgrd, Pgrd>>;

class Tracer
{
	class Data;
	Data * d;

public:
	Tracer();
	~Tracer();
	bool run(Buffer& tobe, int step_count);
};