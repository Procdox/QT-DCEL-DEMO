#pragma once
#include "Grid_Point.h"
#include <vector>

class CityGen
{
public:
	CityGen();
	std::vector<std::pair<Pgrd, Pgrd>> Fill();

	//Generation Settings
	grd Area_Size = 10000;
	int SegMax = 200000;
	grd SegLength = 8;

	//Terrain
	int MntCount = 4;
	int HillCount = 10;
	int CityCount = 3;

	//Spacing Densities
	int HighDensity = 60;
	int MidDensity = 30;
	int LowDensity = 6;

	//Spacing Lengths
	int HighLength = 500;
	int MidLength = 200;
	int LowLength = 200;

	//Intersection Attempt Spacing
	grd SeedCoef = 1.5;

	grd MaxSlope = .4;

	//Stock Parrallel Threshold
	grd StockThresh = .2;

	//Hill Parrallel Threshold
	grd HillThresh = .3;
};