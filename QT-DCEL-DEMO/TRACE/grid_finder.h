#pragma once
#include "DCEL/Grid_Point.h"
#include <map>
#include <vector>

// allows for fast lookup of nearby positional elements <t>
// IS NOT responsible for data lifetime
template<typename t>
class GRID_FINDER {
	struct GRID_ELEM {
		Pgrd location;
		t data;
	};

	std::multimap<std::pair<int, int>, GRID_ELEM> range;
	Pgrd cell_size;

	std::pair<int, int> getBox(const Pgrd& target) const {
		const auto r = target / cell_size;
		return std::make_pair(std::floor(r.X.n), std::floor(r.Y.n));
	}
public:
	~GRID_FINDER() {
	}
	GRID_FINDER(Pgrd _cell_size) 
	: cell_size(_cell_size) {
	}

	void AddPoint(const Pgrd insert, const t _data) {
		range.emplace(getBox(insert), )

	}
	GRID_ELEM const * FindNearest(const Pgrd& target) {

		return result;
	}
	std::list<GRID_ELEM const *> CollectRange(const Pgrd target, const grd range) {
		std::list<KD_Branch const *> result;

		return result;
	}
};