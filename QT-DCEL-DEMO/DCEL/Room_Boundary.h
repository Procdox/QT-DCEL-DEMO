#pragma once

#include "Grid_Tools.h"

struct Room_Boundary
{
	Room_Boundary(Face<Pgrd> * reference);

	bool walled;
	FLL<Pgrd> Points;
	FLL<Pgrd> Offsets;

	FLL<Pgrd> Inset(grd const distance) const;
};