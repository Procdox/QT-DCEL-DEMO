#pragma once

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#define grid_epsilon .000001

class grd {
	

public:
	double n;

	grd();
	grd(double num);

	grd(grd &&target);
	grd(grd const &target);

	grd & operator=(double const & target);
	grd & operator=(grd const & target);
	
	grd operator-() const;
	grd operator+(double factor) const;
	grd operator+(const grd &add) const;
	grd operator-(double factor) const;
	grd operator-(const grd &sub) const;
	grd operator*(double factor) const;
	grd operator*(const grd &target) const;
	grd operator/(double factor) const;
	grd operator/(const grd &target) const;

	grd& operator+=(double factor);
	grd& operator+=(const grd &target);
	grd& operator-=(double factor);
	grd& operator-=(const grd &target);
	grd& operator*=(double factor);
	grd& operator*=(const grd &target);
	grd& operator/=(double factor);
	grd& operator/=(const grd &target);

	bool operator==(const double &test) const;
	bool operator==(const grd &test) const;
	bool operator!=(const double &test) const;
	bool operator!=(const grd &test) const;
	bool operator>(const double &test) const;
	bool operator>(const grd &test) const;
	bool operator<(const double &test) const;
	bool operator<(const grd &test) const;
	bool operator>=(const double &test) const;
	bool operator>=(const grd &test) const;
	bool operator<=(const double &test) const;
	bool operator<=(const grd &test) const;

	grd sqrt() const;

  grd clamp(grd min, grd max) const;

	static grd abs(const grd &target);
};