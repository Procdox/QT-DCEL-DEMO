#pragma once
#include "Grid_Point.h"

#define list_contains(a,b) (std::find(a.begin(),a.end(),b) != a.end())
#define cyclic_next(a,b) (std::next(a) == b.end() ? b.begin() : std::next(a))

/*

Contains definition for a spacial representation agnostic template of a DCEL structure
_P is not directly interacted with, therefor any data type you like can be push_backed
The structure specifically details how regions who have intersecting boundaries are related.

*/

class Point;
class Edge;
class Face;
class Region;
class DCEL;

// Represents a point in space, the ends of edges, and corners of faces
class Point {
  //friend Face;
  friend DCEL;
  friend Edge;

  // The associated DCEL system
  DCEL * universe;

  // An edge leaving this, nullptr if no edges reference this point
  Edge * root;

  // The position of this point of course!
  Pgrd position;

  // This can only be created through DCEL system functions
  Point(DCEL * uni);
  Point(Point &&) = delete;
  Point(Point const &) = delete;

  ~Point();
public:
  int mark;

  void setPosition(Pgrd p);
  Pgrd getPosition() const;

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Traversal Methods

  Edge * getRoot();

  Edge const * getRoot() const;
};

enum EdgeModType { face_destroyed, faces_preserved, face_created};

class EdgeModResult {
  friend Edge;
  //this can only be created by modifications
  EdgeModResult(EdgeModType t, Face * f) {
    type = t;
    relevant = f;
  }
public:

  EdgeModType type;
  Face * relevant;
};

// Represents a one way connection between two points
class Edge {
  //friend Face;
  friend DCEL;
  friend Face;

  // The associated DCEL system
  DCEL * universe;

  // The point this edge originates from
  Point * root;

  // The next edge on the boundary this edge forms
  Edge * next;
  // The previous edge on the boundary this edge forms
  Edge * last;
  // The inverse edge of this one.
  Edge * inv;

  // The face this edge forms a boundary for
  Face * loop;

  //this can only be created through DCEL system functions
  Edge(DCEL * uni);
  Edge(Edge &&) = delete;
  Edge(Edge const &) = delete;

  ~Edge();
public:
  int mark;

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Traversal Methods

  Point * getStart();
  Point * getEnd();

  Edge * getNext();
  Edge * getLast();
  Edge * getInv();

  Face * getFace();

  //get the next edge cw around the root point
  Edge * getCW();
  //get the next edge ccw around the root point
  Edge * getCCW();

  Point const * getStart() const;
  Point const * getEnd() const;

  Edge const * getNext() const;
  Edge const * getLast() const;
  Edge const * getInv() const;

  Face const * getFace() const;

  //get the next edge cw around the root
  Edge const * getCW() const;
  //get the next edge ccw around the root
  Edge const * getCCW() const;

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Modification

  //subdivides this point, maintains this edges root
  void subdivide(Pgrd mid_point);

  //detach from the current root point in favor of a novel point
  //just moves root if it is isolated
  //returns the face left at og root if it exists
  EdgeModResult moveRoot(Pgrd p);

  //detaches from the current root and inserts to the end of target
  //deletes og root if thereafter isolated
  //returns the face left at og root if it exists
  EdgeModResult insertAfter(Edge* target);

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Removal

  //remove this edge, its inverse, and either end points that are thereafter isolated
  //returns novel face if created
  EdgeModResult remove();

  //contracts this edge, the resulting point position is this edges root position
  void contract();

};

//represents a loop formed by a series of edges
class Face {
  friend DCEL;
  friend Region;
  friend Edge;

  DCEL * universe;

  Edge * root;

  Region * group;

  //this can only be created through DCEL system functions
  Face(DCEL * uni);
  Face(DCEL * uni, Region * grp);
  Face(Face &&) = delete;
  Face(Face const &) = delete;

  ~Face();

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Modifiers
  //sets all edges of this loop to reference this
  void reFace();

public:
  int mark;

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Traversal Methods

  Edge * getRoot();

  Edge const * getRoot() const;

  Region * getGroup();

  Region const * getGroup() const;

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Queries

  //get the count of edges in the boundary
  int getLoopSize() const;

  //return a list of the points in the loop
  std::list<Pgrd> getLoopPoints() const;

  //return a list of the edges in the loop
  std::list<Edge *> getLoopEdges();

  //returns a list of the edges in the loop
  std::list<Edge const *> getLoopEdges() const;

  //write to a list, the edges in the loop
  void getLoopEdges(std::list<Edge *> &target);

  //write to a list, the edges in the loop
  void getLoopEdges(std::list<Edge const *> &target) const;

  //return a list of the faces that share a boundary in the loop
  std::list<Face *> getNeighbors();

  //return a list of the faces that share a boundary in the loop
  std::list<Face const *> getNeighbors() const;

  bool neighbors(Face const * target) const;
  /*struct interact_point {
    interaction_state state;
    int region;
    _P point;
    interact_point* next;
    int mid_state;
    interact_point(const _P p) {
      region = -1;
      state = unknown_region;
      point = p;
      next = nullptr;
      mid_state = -1;
    }
    interact_point(const _P p, interact_point* n) {
      region = -1;
      state = unknown_region;
      point = p;
      next = n;
      mid_state = -1;
    }
  };

  struct strand {
    Edge* interior_edge;
    Edge* exterior_edge;
    bool unique_interior;
    bool unique_exterior;
    strand() {
      unique_interior = true;
      unique_exterior = true;
    }
  };

  int getPointState(const _P &test_point) const;

  int getFirstIntersect(const _P &start, const _P &end, _P &intersect) const;*/

  grd area() const;

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //         Regioning

  //takes in a simple region boundary and divides this face into faces interior to, and faces exterior to that boundary
  //void subAllocateFace(std::list<Pgrd> const &boundary, FLL<Face*> &interiors, FLL<Face*> &exteriors);

  std::list<Face *> mergeWithFace(Face* target);
};

class Region {
  friend DCEL;

  DCEL * universe;

  Region(DCEL * uni);
  ~Region();
  std::list<Face *> Boundaries;

  Region(Region &&) = delete;
  Region(Region const &) = delete;

public:
  int mark;

  std::list<Face *> const & getBounds();
  //Face * operator[](int a);

  int size() const;
  grd area() const;

  void add_border(Face * border);
  void remove(Face * border);
  void clear();
  DCEL * getUni();

  std::list<Region *> getNeighbors();

};



class DCEL {
  std::list<Point *> points;
  std::list<Edge *> edges;
  std::list<Face *> faces;
  std::list<Region *> regions;

  friend Edge;

  //creates a point
  //no parameters are initialized
  Point * createPoint();
  //creates an edge and its inverse
  //no parameters are initialized
  Edge * createEdge();
  //creates a face
  //no parameters are initialized
  Face * createFace();

  //removes a point
  //does NOT check to see if referenced elsewhere
  void removePoint(Point * target);
  //removes an edge and its inverse
  //does NOT check to see if referenced elsewhere
  void removeEdge(Edge * target);
  //removes a face
  //does NOT check to see if referenced elsewhere
  void removeFace(Face * target);

public:
  ~DCEL();

  int pointCount() const;
  int edgeCount() const;
  int faceCount() const;
  int regionCount() const;

  //creates an edge and its inverse connecting two novel points
  Edge * addEdge(Pgrd a, Pgrd b);
  //creates an edge and its inverse connecting after an edge to a novel point
  Edge * addEdge(Edge * a, Pgrd b);
  //creates an edge and its inverse connecting after an two edges
  //if this links two faces, b's is removed
  //if this splits a face, the novel has b as it's root
  Edge * addEdge(Edge * a, Edge * b);

  Region * region();

  Region * region(Face * face);
  Region * region(std::list<Pgrd> const &boundary);

  //creates a circular chain of edges forming a loop with the given boundary
  //returns a pointer to the clock-wise oriented interior of the boundary
  Face * draw(std::list<Pgrd> const &boundary);

  //removes a region
  //does NOT check to see if referenced elsewhere
  void removeRegion(Region * target);

  void resetPointMarks();

  void resetEdgeMarks();

  void resetFaceMarks();

  void resetRegionMarks();
};
