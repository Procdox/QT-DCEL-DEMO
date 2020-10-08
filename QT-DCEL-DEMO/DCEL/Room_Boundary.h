#pragma once

#include "Grid_Tools.h"

struct Room_Boundary
{
  Room_Boundary(Face * reference);

  bool walled;
  std::list<Pgrd> Points;
  std::list<Pgrd> Offsets;

  std::list<Pgrd> Inset(grd const distance) const;
};