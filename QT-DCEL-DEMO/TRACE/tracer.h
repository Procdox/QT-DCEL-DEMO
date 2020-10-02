#pragma once
#include "DCEL/Grid_Point.h"
#include "field_model.h"
#include <vector>

#define PI 3.1415926535

using LineBuffer = std::vector<std::pair<Pgrd, Pgrd>>;
using PolygonBuffer = std::vector<std::vector<Pgrd>>;

class Network {
  class CreationWizard;
public:
  class NodeOwner {
  public:
    virtual ~NodeOwner() {};
  };
  class NetworkNode;
  class NetworkStrand;
  class NetworkIntersection;
  class NetworkBlock;

  //  Interfaces

  //represents a point in the network, superset of NetworkIntersections where edges have cooresponding NetworkStrand members inserted
  class NetworkNode {
    //returns the node position
  public:
    virtual ~NetworkNode() {};

    virtual Pgrd getPosition() const = 0;
    //returns a cw sorted list of node connections
    virtual std::vector<const NetworkNode*> getConnections() const = 0;

    virtual int DistanceFrom(NetworkNode const& a, int max_distance) const = 0;
    //DEBUG ONLY - for testing mid generation
    virtual int getSize() const = 0;
    virtual NodeOwner * getOwner() const = 0;
  };

  //represents a path in the network such that if one contracted the path to a single edge [ members.front <-> members.back ] it would not change the network topology
  // or: represents a section of road with no internal intersections (the ends may be intersections or dead ends)
  class NetworkStrand : public NodeOwner {
  public:
    virtual ~NetworkStrand() {};

    virtual std::vector<const NetworkNode*> getMembers() const = 0;
    virtual const NetworkIntersection* getStart() const = 0;
    virtual const NetworkIntersection* getEnd() const = 0;
    virtual const std::vector<Pgrd>& getLeftBorder() const = 0;
    virtual const std::vector<Pgrd>& getRightBorder() const = 0;
    virtual const NetworkBlock* getLeftBlock() const = 0;
    virtual const NetworkBlock* getRightBlock() const = 0;
  };
  class NetworkIntersection : public NodeOwner {
  public:
    virtual ~NetworkIntersection() {};

    virtual const NetworkNode& getRoot() const = 0;
    //returns a cw sorted list of intersection connections (cw by strand, not destination)
    virtual std::vector<std::pair<const NetworkStrand*, const NetworkIntersection*>> getConnections() const = 0;
    virtual const std::vector<Pgrd>& getCorner(int index) const = 0;
  };

  class NetworkBlock {
  public:
    virtual ~NetworkBlock() {};
    virtual std::vector<std::pair<const NetworkIntersection*,int>> getEdges() const = 0;
    virtual std::vector<Pgrd> getBorder() const = 0;
  };

private:
  const grd radius;
  const grd safe_radius;
  CreationWizard * wizard = nullptr;

  //results
  std::vector<NetworkNode*> nodes;
  std::vector<NetworkStrand*> strands;
  std::vector<NetworkIntersection*> intersections;
  std::vector<NetworkBlock*> blocks;

public:
  Network(const grd);
  ~Network();

  static void runIntersectionTest(PolygonBuffer*);

  void generate_terrain(LineBuffer* major_tobe, LineBuffer* minor_tobe);
  void generate_nodes(LineBuffer* tobe);
  void clean_nodes(LineBuffer* tobe);
  void generate_structure(LineBuffer* tobe);
  void generate_roads(PolygonBuffer* tobe, PolygonBuffer* errors);
  void generate_blocks(PolygonBuffer* tobe);
  bool isFinished() const { return wizard == nullptr; }

  const std::vector<NetworkBlock const *> getBlocks() const {
    return {blocks.begin(), blocks.end()};
  }

  Tensor_Field terrain;
};

double pRandom(double a, double b);

Pgrd rotateVector(const Pgrd& src, grd delta);

Pgrd randomUnitVector();

Pgrd randomQuarterUnitVector();

grd Area(const std::vector<Pgrd>& border);