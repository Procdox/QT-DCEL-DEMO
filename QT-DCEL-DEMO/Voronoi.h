#pragma once
#ifndef VORONOI_H
#define VORONOI_H

#include "./DCEL/Grid_Tools.h"
#include "./DCEL/Room_Boundary.h"

class config_gen {
  class config_gen_data;
  config_gen_data * data;

public:
  config_gen();
  ~config_gen();

  enum pointtypeenum {
    ptPoisson,
    ptBiasedPoisson,
    ptGrid,
    ptRandomOffsetGrid,
    ptSimplexOffsetGrid,
    ptComplexGrid,
    ptRoadOverride
  };
  enum boundarytypeenum {
    btRadius,
    btBox,
    btFill
  };

  pointtypeenum PointType = ptPoisson;
  int MaxPoints = 300;
  grd PointSpacing = 15;
  int PointBiasStrength = 3;
  
  boundarytypeenum BoundaryType = btRadius;
  grd GlobalRadius = 70;
  grd BoundaryRadius = 30;

  grd HallWidth = 2;

  int throttle = 1000;

  void run();
  std::list<Region *> const & halls();
  std::list<Region *> const & blocks();
  std::list<Region *> const & exteriors();
};

#endif