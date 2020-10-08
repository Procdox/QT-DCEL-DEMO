#pragma once
#include "DCEL.h"

/*

Contains a definition for a specific DCEL representation that
allows for continuous regional allocations in the discrete 2d plane

continuous implies that any boundary is "inside" every other boundary
therefor there can only be one clockwise boundary and any number of discrete counter
clockwise boundaries contained within the clockwise boundary.

*/

//the relational types between a query point and a face
enum FaceRelationType { point_exterior, point_on_boundary, point_interior };

struct interact {
  Pgrd location;
  FaceRelationType type;

  FaceRelationType mid_type;
  Pgrd mid_location;

  Edge * mark;

  interact() {
    mark = nullptr;
  }
};

//represents the relation a query point has to a face
//returned by getPointRelation
struct FaceRelation {
  //this can only be created by queris / copying 
  FaceRelation(FaceRelationType t, Edge * e) {
    type = t;
    relevant = e;
  }
  FaceRelationType type;
  //if type is point_on_boundary, this is the edge that contains the point
  //root inclusive, end exclusive
  Edge * relevant;
};

FaceRelation const getPointRelation(Face &rel, Pgrd const &test_point);

FaceRelationType const getPointRelation(std::list<Pgrd> const & rel, Pgrd const &test_point);

bool merge(Region * a, Region * b);

bool angledBetween(Pgrd const &A, Pgrd const &B, Pgrd const &test);

void determineInteriors(Region *, std::list<interact *> &, std::list<Face *> &,
  std::list<Face *> &);

bool markRegion(Region *, std::list<Pgrd> const &, std::list<interact *> &);

//type dependent
void subAllocate(Region * target, std::list<Pgrd> const & boundary,
  std::list<Region *> &exteriors, std::list<Region *> & interiors);

FaceRelation contains(Region * target, Pgrd const & test_point);

std::list<Region *> getNeighbors(Region * target);

void cleanRegion(Region * target);

//adds an edge between two edges within this region
  //fails if edges do not belong to this region
  //returns novel region if this splits the region
Region * RegionAdd(Region * target, Edge * A, Edge * B);