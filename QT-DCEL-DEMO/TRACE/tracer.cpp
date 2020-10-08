#include "tracer.h"
#include "cell.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <cmath>
#include <time.h>

#define _DEBUG_MAX_SIZE_ 4
#define _DEBUG_MIN_SIZE_ 0
#define _TENSOR_THRESH_ 0.001

double pRandom(double a, double b) {
  double random = ((double)rand()) / (double)RAND_MAX;
  double diff = b - a;
  double r = random * diff;
  return a + r;
}

Pgrd rotateVector(const Pgrd& src, grd delta) {
  const double co = cos(delta.n);
  const double si = sin(delta.n);
  return Pgrd(src.X*co - src.Y*si, src.X*si + src.Y*co);
}

Pgrd randomUnitVector() {
  return rotateVector(Pgrd(0,1),pRandom(0.0, PI * 2));
}

Pgrd randomQuarterUnitVector() {
  return rotateVector(Pgrd(0, 1), pRandom(0.0, PI / 2));
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
// Declarations

class Network::CreationWizard {
  class CreationNode;
  class CreationStrand;
  class CreationIntersection;
  class CreationBlock;

  //o holy bless'd father
  Network& net;

  struct CreationSeed {
    enum OriginType {
      None = 0,
      Branch,
      Extension
    };

    Pgrd heading;
    CreationNode * node;
    OriginType type;
    grd priority;
    int size;

    CreationSeed(const Pgrd& _heading, CreationNode& _node, OriginType _type, int _size);
  };

  //the dirty truth behind nodes
  class CreationNode final : public NetworkNode {

  public:
    Pgrd position;
    int size;
    bool explored = false;

    CreationNode(const Pgrd& _position, int _size);
    CreationNode(const CreationNode&) = delete;
    CreationNode(CreationNode&&) = delete;

    bool operator==(const CreationNode& other) = delete;
    bool operator!=(const CreationNode& other) const { return this != &other; }

    grd distance(const CreationNode * other) const {
      return (position - other->position).Size();
    }

    //list of connections, use addConnection helper function when possible
    std::vector<CreationNode*> connected;
    NodeOwner * owner = nullptr;

    //overrides
    Pgrd getPosition() const override { return position; };
    //returns a cw sorted list of node connections
    std::vector<const NetworkNode*> getConnections() const override {
      std::vector< const NetworkNode*> result(connected.size());
      std::transform(connected.begin(), connected.end(), result.begin(),
        [](CreationNode* a) {return static_cast<NetworkNode*>(a); });

      //throw std::logic_error("NoImpl: not sorted");

      return result;
    };
    int DistanceFrom(NetworkNode const& a, int max_distance) const override { return -1; };
    int getSize() const override { return size; }
    NodeOwner * getOwner() const override { return owner; }

    //adds connection to each node
    void addConnection(CreationNode& partner) { connected.push_back(&partner); partner.connected.push_back(this); }
    void sortConnections() {
      std::sort(connected.begin(), connected.end(),
        [&](CreationNode* a, CreationNode* b) {
        const Pgrd pA = a->position - position;
        const Pgrd pB = b->position - position;
        const grd rA = std::atan2(pA.Y.n, pA.X.n);
        const grd rB = std::atan2(pB.Y.n, pB.X.n);
        return rA > rB;
      });
    }


    bool WithinDistanceOf(CreationNode& a, int distance) {
      std::vector<CreationNode*> mapped;
      std::vector<CreationNode*> frontier;
      std::vector<CreationNode*> swap;

      mapped.push_back(this);
      swap.push_back(this);
      explored = true;

      bool found = false;

      for (auto i = 0; i < distance; ++i) {
        frontier.swap(swap);
        while (!frontier.empty()) {
          auto & n = frontier.back()->connected;
          for (auto & c : n) {
            if (!c->explored) {
              found |= (&a == c);
              c->explored = true;
              mapped.push_back(c);
              swap.push_back(c);
            }
            if (found) break;
          }
          if (found) break;
          frontier.pop_back();
        }
        if (found) break;
      }

      for (auto & t : mapped)
        t->explored = false;

      return found;
    }
  };

  class CreationStrand final : public NetworkStrand {
  public:
    CreationStrand();

    
    CreationIntersection* front = nullptr;
    CreationIntersection* back = nullptr;
    std::vector<CreationNode*> members;
    std::vector<Pgrd> left_border;
    std::vector<Pgrd> right_border;

    CreationBlock * left_block = nullptr;
    CreationBlock * right_block = nullptr;

    CreationIntersection* op(const CreationIntersection& other) const {
      if (front == &other)
        return back;
      if (back == &other)
        return front;
      return nullptr;
    }

    std::vector<const NetworkNode*> getMembers() const override {
      std::vector< const NetworkNode*> result(members.size());
      std::transform(members.begin(), members.end(), result.begin(),
        [](CreationNode* a) {return static_cast<NetworkNode*>(a); });
      return result;
    }
    const NetworkIntersection* getStart() const override { return front; }
    const NetworkIntersection* getEnd() const override { return back; }

    virtual const std::vector<Pgrd>& getLeftBorder() const override {
      return left_border;
    }
    virtual const std::vector<Pgrd>& getRightBorder() const override {
      return right_border;
    }
    virtual const NetworkBlock* getLeftBlock() const override {
      return left_block;
    }
    virtual const NetworkBlock* getRightBlock() const override {
      return right_block;
    }

  };

  class CreationIntersection final : public NetworkIntersection {
  public:
    CreationIntersection(CreationNode& _root);

    CreationNode& root;
    std::vector<std::pair<CreationStrand*, CreationIntersection*>> connected;
    std::vector<std::vector<Pgrd>> corner_borders;

    const NetworkNode& getRoot() const override { return root; };
    std::vector<std::pair<const NetworkStrand*, const NetworkIntersection*>> getConnections() const override {
      std::vector<std::pair<const NetworkStrand*, const NetworkIntersection*>> result(connected.size());
      std::transform(connected.begin(), connected.end(), result.begin(),
        [](const std::pair<CreationStrand*, CreationIntersection*>& a) {return a; });

      return result;
    };
    virtual const std::vector<Pgrd>& getCorner(int index) const override {
      return corner_borders[index];
    };
    void sortConnections() {
      std::sort(connected.begin(), connected.end(),
        [&](std::pair<CreationStrand*, CreationIntersection*>& _a, std::pair<CreationStrand*, CreationIntersection*>& _b) {
        CreationNode* a;
        CreationNode* b;

        if (!_a.first->members.empty()) {
          if (_a.first->front == this)
            a = _a.first->members.front();
          else
            a = _a.first->members.back();
        }
        else
          a = &_a.second->root;

        if (!_b.first->members.empty()) {
          if (_b.first->front == this)
            b = _b.first->members.front();
          else
            b = _b.first->members.back();
        }
        else
          b = &_b.second->root;

        const Pgrd pA = a->position - root.position;
        const Pgrd pB = b->position - root.position;
        const grd rA = std::atan2(pA.Y.n, pA.X.n);
        const grd rB = std::atan2(pB.Y.n, pB.X.n);
        return rA > rB;
      });
    }
  };

  class CreationBlock final : public NetworkBlock {
  public:
    std::vector<std::pair<CreationIntersection *, int>> edges;
    std::vector<std::pair<const NetworkIntersection*, int>> getEdges() const override {
      std::vector<std::pair<const NetworkIntersection*, int>> result(edges.size());
      std::transform(edges.begin(), edges.end(), result.begin(),
        [](const std::pair<CreationIntersection*, int>& a) {return a; });

      return result;
    };
    std::vector<Pgrd> getBorder() const override {
      std::vector<Pgrd> result;
      for (auto edge : edges) {
        auto & corner = edge.first->corner_borders[edge.second];

        auto * strand = edge.first->connected[edge.second].first;
        auto & side = strand->front == edge.first ? strand->right_border : strand->left_border;
        std::copy(side.begin(), side.end(), std::back_inserter(result));

        std::copy(corner.begin(), corner.end(), std::back_inserter(result));

        
      }
      return result;
    };
  };
  struct NetworkConfig {
    const int strand_length = 8000; //edges in strand
    const grd edge_length = 4;
    const grd min_edge_length = 6; //when an edge intersects an existing edge, the last node will be ignored if it would create to short an edge
    const grd intersection_spacing = 12;
    const double intersection_variance = .1;
    const grd bridge_length = 8;
    const double growth_rate = 2.7;
    const grd parrallel_thresh = 0.88;
  };

  const NetworkConfig config;

  //creation process data structures;
  //ordered list of frontier to explore first, no questions asked. Used for extensions, crossings, turnbacks, and initial roads
  std::list<CreationSeed> majors;
  //sorted list of frontier to explore. Used for branches.
  struct CompareSeeds {
    bool operator()(CreationSeed const& p1, CreationSeed const& p2)
    {
      // return "true" if "p1" is ordered  
      // before "p2", for example: 
      return p1.priority > p2.priority;
    }
  };
  std::priority_queue<CreationSeed, std::vector<CreationSeed>, CompareSeeds> frontier;

  //huff, this big boy does a lot of work to try and quicken distance searches, it shouldn't work this hard. 
  //TODO: replace with cell based lookup, built with intersection_spacing in mind
  CellSearch<CreationNode*> all_tracker;
  //like all tracker, but only tracks intersections
  CellSearch<CreationNode*> int_tracker;



  //creation methods
  void SingleStream(const CreationSeed& next);
  void create_seed(const Pgrd& _heading, CreationNode & _node, CreationSeed::OriginType _type, bool important, int _size);
  CreationNode& splitConnection(CreationNode& a, CreationNode& b, const Pgrd& pos);

public:
  CreationWizard(Network & _net);
  void create_seed(const Pgrd& _position, const Pgrd& _heading);
  void clean_nodes();
  void generate_nodes();
  void generate_structure();
  void generate_roads();
  void generate_blocks();

  static void runIntersectionTest(PolygonBuffer*);

  //result creation methods
  CreationNode& createNode(const Pgrd& _position, int _size);
  CreationStrand& createStrand();
  CreationIntersection& createIntersection(CreationNode& _root);
  CreationBlock& createBlock();

  void mergeNodes(CreationNode * from, CreationNode * into);

  void removeNode(CreationNode* target) {
    if (target != net.nodes.back()) {
      auto iter = std::find(net.nodes.begin(), net.nodes.end(), target);
      *iter = net.nodes.back();
    }
    net.nodes.pop_back();
    delete target;
  }
  void removeNode(int index) {
    auto * target = net.nodes[index];

    if (index != net.nodes.size() - 1)
      net.nodes[index] = net.nodes.back();

    net.nodes.pop_back();
    delete target;
  }
  void removeStrand(CreationStrand* target) {
    if (target != net.strands.back()) {
      auto iter = std::find(net.strands.begin(), net.strands.end(), target);
      *iter = net.strands.back();
    }
    net.strands.pop_back();
    delete target;
  }
  void removeIntersection(CreationIntersection* target) {
    if (target != net.intersections.back()) {
      auto iter = std::find(net.intersections.begin(), net.intersections.end(), target);
      *iter = net.intersections.back();
    }
    net.intersections.pop_back();
    delete target;
  }
};

//---------------------------------------------------------------------------------------------
// NetworkNode

Network::CreationWizard::CreationNode::CreationNode(const Pgrd& _position, int _size)
: position(_position)
, size(_size) {};

//---------------------------------------------------------------------------------------------
// CreationSeed

Network::CreationWizard::CreationSeed::CreationSeed(const Pgrd& _heading, CreationNode& _node, OriginType _type, int _size)
: heading(_heading)
, node(&_node)
, type(_type)
, priority(100 - (double)_size + pRandom(0.0,0.5))
, size(_size){};

//---------------------------------------------------------------------------------------------
// CreationWizard

Network::CreationWizard::CreationWizard(Network& _net)
: net(_net) 
, all_tracker(Pgrd(net.radius * 2 + config.edge_length * 2, net.radius * 2 + config.edge_length * 2), 10, 10, nullptr)
, int_tracker(Pgrd(net.radius * 2 + config.edge_length * 2, net.radius * 2 + config.edge_length * 2), 10, 10, nullptr) {};

//TODO: replace with project and skew method (with pre-emptive crossing check)
void Network::CreationWizard::SingleStream(const CreationSeed& seed) {
  //goes for a set count in layerinfo or until intersection
  
  Pgrd current = seed.node->position;
  Pgrd last = current;

  Pgrd current_heading = seed.heading;

  { //update our trajectory
    Pgrd adjusted_heading = net.terrain.SumPointContextAware(current, current_heading);
    const grd size = adjusted_heading.Size();
    //slightly larger than 0, to avoid issues where the dotproduct is 0
    if (size > _TENSOR_THRESH_) {
      const grd dot = adjusted_heading.Dot(current_heading);
      if (dot < 0)
        adjusted_heading *= -1;

      adjusted_heading.Normalize();

      current_heading = adjusted_heading;
    }
  }

  Pgrd perpendicular;

  

  std::vector<std::pair<Pgrd,Pgrd>> suggested_spots;

  CreationNode * connect = nullptr;
  bool KEEP = true;
  bool BOUNDED = true;
  bool DEGENERATE = false;
  int WATER_BOUND = 0;
  int len = 0;

  const grd intersect = config.intersection_spacing * std::pow(config.growth_rate,seed.size);
  const int check_distance = (intersect / config.edge_length).n * 2;

  //does this intersection have another connection already going this way
  for (auto * connection : seed.node->connected)
    if((connection->position - seed.node->position).NormDot(current_heading) > config.parrallel_thresh)
      return;

  //are we too close to another existing intersection
  if (seed.node->connected.size() == 2) {
    auto starting_range = int_tracker.CollectRange(current, intersect);
    for (auto check : starting_range) {
      if(check == seed.node) continue;
      if(check->WithinDistanceOf(*seed.node, check_distance))
        return;
    }
  }

  //seed 4
  /*if (net.nodes.size() == 1012) {
    len += 1;
  }
  if (net.nodes.size() > 1012) {
    return;
  }*/

  while (len < config.strand_length) {
    { //update our trajectory
      Pgrd adjusted_heading = net.terrain.SumPointContextAware(current, current_heading);
      const grd size = adjusted_heading.Size();
      //slightly larger than 0, to avoid issues where the dotproduct is 0
      if(size > _TENSOR_THRESH_){
        DEGENERATE = false;
        const grd dot = adjusted_heading.Dot(current_heading);
        if (dot < 0)
          adjusted_heading *= -1;
        else if(dot <= 0) {
          //ah shit conditional case
  
          DEGENERATE = true;
        }

        adjusted_heading.Normalize();

        current_heading = adjusted_heading;
      }
      else
        DEGENERATE = true;
    }
    
    current += current_heading * config.edge_length;

    //update our perpendicular and location
    perpendicular.X = current_heading.Y;
    perpendicular.Y = -current_heading.X;

    if (false /*water bound*/) {
      WATER_BOUND++;
      if(config.edge_length * WATER_BOUND > config.bridge_length * std::pow(config.growth_rate, seed.size)){
        KEEP = false;
        break;
      }
    }
    else {
      WATER_BOUND = 0;
    }

    {  //will this strand respect the bounds?
      if (current.Size() > net.radius) {
        BOUNDED = false;
        break;
      }
    }

    /*{  //are we close enough to another intersection to merge?
      if (auto * branch = int_tracker.FindNearest(current, config.merge_distance); branch != nullptr) {
        CreationNode& CompareNode = *branch;
        if (&CompareNode != seed.node) {
          if (len == 0) {
            for (const auto * neighbor : CompareNode.connected) {
              if (neighbor == seed.node) {
                KEEP = false;
                break;
              }
            }
          }
          if (WATER_BOUND || seed.node->WithinDistanceOf(CompareNode, check_distance))
            KEEP = false;
          else
            connect = &CompareNode;
          break;
        }
      }
    }*/

    auto nearby_nodes = all_tracker.CollectRange(current, intersect);

    { //are we running tightly parrallel with another road, end check
      Pgrd marker;
      const Pgrd offset = perpendicular * intersect * .75;
      const Pgrd C_S = current - offset;
      const Pgrd C_E = current + offset;
      const Pgrd step = current - last;
      for (const auto * node_entry : nearby_nodes) {
        const auto & node = *node_entry;
        const Pgrd A_S = node.position;

        for (const auto * edge : node.connected) { //take the closest?
          
          if (edge == seed.node) continue;
          const Pgrd A_E = edge->position;
          const grd angle = (A_S - A_E).NormDot(step);

          
          if(grd::abs(angle) > config.parrallel_thresh && Pgrd::getIntersect(A_S, A_E, C_S, C_E, marker)){
            KEEP = false;
            break;
          }
        }
        if(!KEEP) break;
      }
      if (!KEEP) break;
    }

    { //are we running tightly parrallel with another road, midpoint check
      Pgrd marker;
      const Pgrd offset = perpendicular * intersect * .75;
      const Pgrd midpoint = (current + last) / 2;
      const Pgrd C_S = midpoint - offset;
      const Pgrd C_E = midpoint + offset;
      const Pgrd step = current - last;
      for (const auto * node_entry : nearby_nodes) {
        const auto & node = *node_entry;
        const Pgrd A_S = node.position;

        for (const auto * edge : node.connected) { //take the closest?

          if (edge == seed.node) continue;
          const Pgrd A_E = edge->position;
          const grd angle = (A_S - A_E).NormDot(step);


          if (grd::abs(angle) > config.parrallel_thresh && Pgrd::getIntersect(A_S, A_E, C_S, C_E, marker)) {
            KEEP = false;
            break;
          }
        }
        if (!KEEP) break;
      }
      if (!KEEP) break;
    }


    {  //are we crossing an edge, and should we merge
      
      Pgrd marker;
      bool hit = false;
      grd best_distance = config.edge_length * 2;

      bool split = false;
      CreationNode * split_a;
      CreationNode * split_b;
      Pgrd split_marker;

      for (auto * node_entry : nearby_nodes) {
        auto & node = *node_entry;
        const Pgrd A_S = node.position;

        if (&node == seed.node) continue;

        for (auto * edge : node.connected) { //take the closest?
          if (edge == seed.node) continue;

          const Pgrd A_E = edge->position;
          if (Pgrd::getIntersect(A_S, A_E, last, current, marker)) {
            if((marker - last).Size() < best_distance){
              split = false;
              best_distance = (marker - last).Size();
              if(A_S == marker)
                connect = &node;
              else if (A_E == marker)
                connect = edge;
              else{
                
                split = true;
                split_a = &node;
                split_b = edge;
                split_marker = marker;
              }
              hit = true;
            }
          }
        }
      }

      if (hit) {
        if (split) {
          if(false /*split marker waterbound*/ || seed.node->WithinDistanceOf(*split_a, check_distance))
            KEEP = false;
          else
            connect = &splitConnection(*split_a, *split_b, split_marker);
        }
        else if (false /*connect waterbound*/ || seed.node->WithinDistanceOf(*connect, check_distance))
          KEEP = false;

        break;
      }
    }
  
    //the point has been accepted
    suggested_spots.push_back(std::make_pair(current, current_heading));

    //setup for the next loop
    last = current;
    len++;
  }

  if(WATER_BOUND) KEEP = false;

  if (KEEP) {
    int currentstep = 0;
    int futurestep = 0;
    grd current_offset = intersect * pRandom(1 - config.intersection_variance,1 + config.intersection_variance);
    grd future_offset = config.intersection_spacing * std::pow(config.growth_rate, seed.size - 1) * pRandom(1 - config.intersection_variance, 1 + config.intersection_variance);

    CreationNode * last_node = seed.node;
    for (auto point : suggested_spots) {
      CreationNode & next_node = createNode(point.first, seed.size);
      last_node->addConnection(next_node);

      const Pgrd left_tangent(point.second.Y, -point.second.X);
      const Pgrd right_tangent(-point.second.Y, point.second.X);

      if(!false /*water bound*/){

        currentstep++;
        futurestep++;
        if (config.edge_length * currentstep > current_offset) {
          create_seed(left_tangent,  next_node, CreationSeed::OriginType::Branch, false, seed.size);
          create_seed(right_tangent,  next_node, CreationSeed::OriginType::Branch, false, seed.size);
          currentstep = 0;
          current_offset = intersect * pRandom(1 - config.intersection_variance, 1 + config.intersection_variance);
        }
        if (seed.size > _DEBUG_MIN_SIZE_) {
          if (config.edge_length * futurestep > future_offset) {
            // if another road is to close to either of these seeds, dont add them
            create_seed(left_tangent, next_node, CreationSeed::OriginType::Branch, false, seed.size - 1);
            create_seed(right_tangent, next_node, CreationSeed::OriginType::Branch, false, seed.size - 1);
            futurestep = 0;
            future_offset = config.intersection_spacing * std::pow(config.growth_rate,seed.size - 1) * pRandom(1 - config.intersection_variance, 1 + config.intersection_variance);
          }
        }
      }

      last_node = &next_node;
    }

    //origin point intersection check
    if (seed.node->connected.size() == 3) {
      int_tracker.AddPoint(seed.node->position, seed.node);
    }

    if (connect) {
      if(connect->connected.size() == 2){
        int_tracker.AddPoint(connect->position, connect);
        if (BOUNDED){
        
          if(DEGENERATE){
            if(grd::abs(current_heading.NormDot(connect->position - connect->connected[0]->position)) < .3)
              create_seed(current_heading, *connect, CreationSeed::OriginType::Extension, false, seed.size);
          }
          else
            create_seed(current_heading, *connect, CreationSeed::OriginType::Extension, false, seed.size);
        }
        
      }
      connect->addConnection(*last_node);
    }
    else if(BOUNDED)
      create_seed(current_heading, *last_node, CreationSeed::OriginType::Extension, true, seed.size);
  }
}

void Network::CreationWizard::generate_nodes() {
  while (true) {
    if (!majors.empty()) {
      const CreationSeed next = majors.front();
      majors.pop_front();

      SingleStream(next);
    }
    else if (!frontier.empty()) {
      const CreationSeed next = frontier.top();
      frontier.pop();

      SingleStream(next);
    }
    else
      break;
  }
}

//guarantees no nodes are closer than min_edge_length through merges
//removes redundant nodes that fail parrallel test
void Network::CreationWizard::clean_nodes() {

  //merge nodes that are too close, prioritize size: connectivity
  auto merge_step = [&](){
    for (int i = 0; i < net.nodes.size(); i++) {
      auto * node = static_cast<CreationNode*>(net.nodes[i]);

      for (int j = 0; j < node->connected.size(); j++) {
        auto * connect = node->connected[j];
        if (connect->distance(node) < config.min_edge_length) {
          if (connect->size > node->size) {
            mergeNodes(node, connect);
            i--;
            break;
          }
          else if (connect->size < node->size) {
            mergeNodes(connect, node);
            j--;
          }
          else if (connect->connected.size() > node->connected.size()) {
            mergeNodes(node, connect);
            i--;
            break;
          }
          else {
            mergeNodes(connect, node);
            j--;
          }
        }
      }
    }
  };

  auto relax_step = [&](const grd ratio) {
    const int size = net.nodes.size();
    std::vector<Pgrd> offsets(size);
    for (int i = 0; i < size; i++) {
      auto * node = static_cast<CreationNode*>(net.nodes[i]);
      if (node->connected.size() != 2) continue;

      auto * a = node->connected[0];
      auto * b = node->connected[1];

      offsets[i] = (a->position + b->position)/2 - node->position;
    }

    for (int i = 0; i < size; i++) {
      auto * node = static_cast<CreationNode*>(net.nodes[i]);
      if (node->connected.size() != 2) continue;


      node->position += offsets[i] * ratio;
    }
  };

  for(int i = 3; i > 0; --i) {
    merge_step();
    relax_step(.2*i);
  }
  merge_step();
  
  //remove redundant parrallel nodes
  for (int i = 0; i < net.nodes.size(); i++) {
    auto * node = static_cast<CreationNode*>(net.nodes[i]);
    if (node->connected.size() != 2) continue;

    auto * a = node->connected[0];
    auto * b = node->connected[1];

    if ((a->position - node->position).NormDot(node->position - b->position) < .98) continue;

    for (auto *& back_connect : a->connected)
      if (back_connect == node)
        back_connect = b;

    for (auto *& back_connect : b->connected)
      if (back_connect == node)
        back_connect = a;

    removeNode(i);
    i--;
  }

  //sort node connetions cw
  for (auto * _node : net.nodes) {
    auto * node = static_cast<CreationNode*>(_node);
    node->sortConnections();
  }
}

void Network::CreationWizard::generate_structure() {

  //clean loose ends
  //merge intersections that aren't distinguishable
  //create intersections and strands

  auto trace_strand = [&](CreationIntersection* intersection, CreationNode* next) {
    CreationNode* last = &intersection->root;

    std::vector<CreationNode*> members;
    //does the connection exist already
    if (next->connected.size() == 2) {
      if(next->owner != nullptr) return;

      while (next->connected.size() == 2) {
        auto * focus = next;
        members.push_back(focus);

        if(focus->connected.front() == last)
          next = focus->connected.back();
        else
          next = focus->connected.front();
        last = focus;
      }
    }
    CreationIntersection * reached;
    if (reached = dynamic_cast<CreationIntersection*>(next->owner); reached != nullptr) {

      //ensure this connection hasn't already been created, or we haven't created a cycle
      if (reached == intersection)
        throw;
      
      for (auto & connection : reached->connected)
        if (connection.second == intersection) return;
    }
    else
      reached = &createIntersection(*next);

    CreationStrand & strand = createStrand();
    strand.front = intersection;
    strand.back = reached;
    for (auto node : members)
      node->owner = &strand;
    strand.members = std::move(members);

    intersection->connected.push_back(std::make_pair(&strand, reached));
    reached->connected.push_back(std::make_pair(&strand, intersection));
  };

  //find all intersections and notate their connections
  for (auto * _node : net.nodes) {
    auto * node =  static_cast<CreationNode*>(_node);
    if (node->connected.size() != 2) {
      CreationIntersection * owner = dynamic_cast<CreationIntersection*>(node->owner);
      
      if(owner == nullptr)
        owner = &createIntersection(*node);

      for (auto neighbor : node->connected){
        trace_strand(owner, neighbor);
      }
      
      if (node->connected.size() != owner->connected.size()) {
        //We have a redundant connection, probably caused by a double loop on the outer connections
        //since it is unlikely to be seen, we just choose one to preserve, and delete the other strand

        for (auto i = 0; i < node->connected.size(); ++i) {
          auto focus = node->connected[i];
          if(focus->owner != nullptr) continue;

          node->connected[i] = node->connected.back();
          node->connected.pop_back();
          
          auto * last = node;
          auto * next = focus;
          while (focus->connected.size() == 2) {
            if(focus->connected.front() == last)
              next = focus->connected.back();
            else
              next = focus->connected.front();
          
            removeNode(focus);
            last = focus;
            focus = next;
          }
          for (auto j = 0; j < focus->connected.size(); ++j)
            if (focus->connected[j] == last) {
              focus->connected[j] = focus->connected.back();
              focus->connected.pop_back();
            }
        }

        node->sortConnections();
      }
    }
  }

  for (auto * _intersection : net.intersections) {
    auto * intersection = static_cast<CreationIntersection*>(_intersection);
    intersection->sortConnections();
  }
}

void Network::CreationWizard::generate_roads() {
  const grd scale = .16;
  const grd intersection_width = 3;

  auto norm_offset_sect = [](const CreationNode* l, const CreationNode* r, const CreationNode* center, const grd offset) {
    Pgrd a = l->position - center->position;
    Pgrd c = r->position - center->position;

    if (a == c) {
      throw std::logic_error("wat");
    }

    a.Normalize();
    c.Normalize();

    if (a == c) {
      throw std::logic_error("wat");
    }

    const Pgrd at(a.Y, -a.X);

    Pgrd b = a + c;

    if (b.Dot(at) < 0) b *= -1;
    else if (b.SizeSquared() == 0) b = at;

    b.Normalize();

    if (b.Dot(at) == 0) {
      throw std::logic_error("wat");
    }

    const grd length = (offset / b.Dot(at));

    if (grd::abs(length) > 10 || std::isnan(length.n))
      throw std::logic_error("wat");

    return b * length;
  };

  auto offset_sect = [norm_offset_sect, scale](const CreationNode* left_node, const CreationNode* right_node, const CreationNode* center_node) {
    const Pgrd center = center_node->position;
    const int l_size = left_node->size;
    const int r_size = right_node->size;
    const int m_size = center_node->size;
    const int a_size = /*dynamic_cast<NetworkIntersection*>(left_node->getOwner()) ?*/ (m_size < l_size ? m_size : l_size);// : l_size;
    const int c_size = /*dynamic_cast<NetworkIntersection*>(right_node->getOwner()) ?*/ (m_size < r_size ? m_size : r_size);// : r_size;
    const grd a_offset = scale * (a_size + 1);
    const grd c_offset = scale * (c_size + 1);

    Pgrd a = left_node->position - center;
    Pgrd c = right_node->position - center;

    if (a == c) {
      throw std::logic_error("wat");
    }

    a.Normalize();
    c.Normalize();

    if (a == c) {
      throw std::logic_error("wat");
    }

    const Pgrd at(-a.Y, a.X);
    const Pgrd ct(-c.Y, c.X);
    const Pgrd d = at * a_offset + ct * c_offset;

    const grd _phi = a.NormDot(c).n;
    const grd phi = std::acos(_phi.clamp(-1, 1).n);
    if (phi < 0.05 || std::isnan(phi.n)) {
      throw std::logic_error("wat");
    }
    else if (phi > 3.12 || at.Dot(a + c) > 0) {
      //effectively oppisite each other, use average norm bisector instead
      return norm_offset_sect(left_node, right_node, center_node, (a_offset + c_offset) / 2);
    }
    const grd theta = std::acos(a.NormDot(d).n);

    const grd b = d.Size() * (std::sin(theta.n) / std::sin(phi.n)) * (at.Dot(d) > 0 ? 1 : -1);

    const Pgrd T = c * b + ct * c_offset;

    if (grd::abs(T.X) > 10 || grd::abs(T.Y) > 10 || std::isnan(T.X.n) || std::isnan(T.Y.n))
      throw std::logic_error("wat");

    return T;
  };

  for (auto * _focus : net.intersections) {
    CreationIntersection * focus = static_cast<CreationIntersection*>(_focus);

    const Pgrd center = focus->root.position;

    const auto intersect_connections = focus->connected;
    const auto node_connections = focus->root.connected;
    const int size = intersect_connections.size();
    const int center_size = focus->root.size;

    if (size == 1) {
      Pgrd ray = node_connections[0]->position;
      ray -= center;
      ray.Normalize();

      //ray *= scale * (focus->getRoot().getSize() +1);
      const Pgrd off = Pgrd(ray.Y, -ray.X) * scale * (center_size + 1);
      ray *= intersection_width;

      std::vector<Pgrd> corner;
      
      corner.push_back(center + ray + off);
      corner.push_back(center - ray + off);
      corner.push_back(center - ray - off);
      corner.push_back(center + ray - off);
      focus->corner_borders.push_back(std::move(corner));
    }
    else {
      //calculate offset bisectors
      for (auto i = 0; i < size; i++) {
        const int j = (i + 1) % size;

        std::vector<Pgrd> corner;
        {
          Pgrd ray = node_connections[i]->position - center;
          ray.Normalize();

          const int ray_size = node_connections[i]->size;
          const int a_size = /*dynamic_cast<NetworkIntersection*>(left_node->getOwner()) ?*/ (center_size < ray_size ? center_size : ray_size);// : l_size;

          const Pgrd off = Pgrd(ray.Y, -ray.X) * scale * (a_size + 1);
          ray *= intersection_width;
          corner.push_back(center + ray + off);
        }

        const Pgrd b = offset_sect(node_connections[i], node_connections[j], &focus->root);
        corner.push_back(center + b);

        {
          Pgrd ray = node_connections[j]->position - center;
          ray.Normalize();

          const int ray_size = node_connections[j]->size;
          const int a_size = /*dynamic_cast<NetworkIntersection*>(left_node->getOwner()) ?*/ (center_size < ray_size ? center_size : ray_size);// : l_size;

          const Pgrd off = Pgrd(ray.Y, -ray.X) * scale * (a_size + 1);
          ray *= intersection_width;
          corner.push_back(center + ray - off);
        }
        focus->corner_borders.push_back(std::move(corner));
      }

      /*for (auto i = 0; i < size; i++) {
        const int j = (i + 1) % size;
        const int k = (i + 2) % size;

        const Pgrd Pi = corners[i];
        const Pgrd Pj = corners[j];
        const Pgrd Pk = corners[k];

        const Pgrd T(Pi.Y - Pj.Y, Pj.X - Pi.X);
        if (T.Dot(Pk - Pj) > 0) {
          std::vector<Pgrd> issue;
          issue.push_back(Pi);
          issue.push_back(Pj);
          issue.push_back(Pk);
          errors->push_back(std::move(issue));
        }
      }*/
    }
  }

  for (auto * _focus : net.strands) {
    auto * focus = static_cast<CreationStrand*>(_focus);

    const auto members = focus->members;
    const int size = members.size();

    std::vector<Pgrd> result;
    std::vector<Pgrd> bisectors;

    for (int i = 0; i < size; i++) {
      const CreationNode *  l = (i == 0) ? &focus->front->root : members[i - 1];
      const CreationNode * r = (i == size - 1) ? &focus->back->root : members[i + 1];
      const Pgrd b = norm_offset_sect(l, r, members[i], scale * (members[i]->size + 1));

      const Pgrd center = members[i]->getPosition();
      focus->left_border.push_back(center + b);
      bisectors.push_back(center - b);
    }

    while (!bisectors.empty()) {
      focus->right_border.push_back(bisectors.back());
      bisectors.pop_back();
    }
  }
}

void Network::CreationWizard::generate_blocks() {
  auto trace_block = [&](bool strand_orientation, CreationStrand * const start) {
    CreationStrand* current_strand = start;

    CreationBlock* result = &createBlock();

    do {
      if(strand_orientation){
        if(current_strand->left_block) throw;
        current_strand->left_block = result;
      }
      else {
        if (current_strand->right_block) throw;
        current_strand->right_block = result;
      }

      CreationIntersection* next_intersect = strand_orientation ? current_strand->back : current_strand->front;

      int current_index;
      for(int i = 0; i < next_intersect->connected.size(); ++i)
        if (next_intersect->connected[i].first == current_strand) {
          current_index = i;
          break;
        }
      
      int next_index = (current_index + 1) % next_intersect->connected.size();

      result->edges.push_back({next_intersect,current_index});

      current_strand = next_intersect->connected[next_index].first;
      strand_orientation = current_strand->front == next_intersect;
    
    }while(current_strand != start);
  };

  for (auto _focus : net.strands) {
    auto * focus = static_cast<CreationStrand*>(_focus);
    if(focus->front->root.position.Size() > net.safe_radius || focus->back->root.position.Size() > net.safe_radius) continue;

    if(focus->left_block == nullptr)
      trace_block(true, focus);
    if(focus->right_block == nullptr)// trace right;
      trace_block(false, focus);
  }
};

void Network::CreationWizard::create_seed(const Pgrd& _position, const Pgrd& _heading) {
  //auto & strand = createStrand();
  const int size = _DEBUG_MAX_SIZE_;
  auto & novel = createNode(_position, size);
  majors.push_front(CreationSeed(_heading, novel, CreationSeed::OriginType::None, size));
  majors.push_front(CreationSeed(_heading * -1, novel, CreationSeed::OriginType::None, size));
}

void Network::CreationWizard::create_seed(const Pgrd& _heading, CreationNode & _node, CreationSeed::OriginType _type, bool important, int _size) {
  if(important)
    majors.push_front(CreationSeed(_heading, _node, _type, _size));
  else
    frontier.push(CreationSeed(_heading, _node, _type, _size));
}

Network::CreationWizard::CreationNode& Network::CreationWizard::splitConnection(CreationNode& a, CreationNode& b, const Pgrd& pos) {
  //if (&a.net != &b.net) throw;
  //if (&a.tier != &b.tier) throw;
  if (auto iter = std::find(a.connected.begin(), a.connected.end(), &b); iter != a.connected.end())
    a.connected.erase(iter);
  else throw std::logic_error("missing connection a -> b");
  if (auto iter = std::find(b.connected.begin(), b.connected.end(), &a); iter != b.connected.end())
    b.connected.erase(iter);
  else throw std::logic_error("missing connection b -> a");

  auto & node = createNode(pos, a.getSize() > b.getSize() ? a.getSize() : b.getSize());

  a.addConnection(node);
  b.addConnection(node);

  return node;
}

//---------------------------------------------------------------------------------------------
// NetworkStrand

Network::CreationWizard::CreationStrand::CreationStrand(){};

//---------------------------------------------------------------------------------------------
// NetworkIntersection

Network::CreationWizard::CreationIntersection::CreationIntersection(CreationNode& _root)
: root(_root) {};

//---------------------------------------------------------------------------------------------
// Network

Network::Network(const grd _radius)
: radius(_radius + 10 * 12/*config.intersection_spacing*/)
, safe_radius(_radius)
, terrain(0.0001) {
  wizard = new CreationWizard(*this);
  wizard->create_seed(Pgrd(0, 0), Pgrd(1, 0));
}

Network::~Network() {
  for (auto node : nodes) delete node;
  for (auto strand : strands) delete strand;
  for (auto intersection : intersections) delete intersection;
  for (auto block : blocks) delete block;

  delete wizard;
}

Network::CreationWizard::CreationNode& Network::CreationWizard::createNode(const Pgrd& _position, int _size) {
  auto * novel = new CreationNode(_position, _size);

  all_tracker.AddPoint(_position, novel);
  net.nodes.push_back(novel);
  return *novel;
}

Network::CreationWizard::CreationStrand& Network::CreationWizard::createStrand() {
  auto * novel = new CreationStrand();
  net.strands.push_back(novel);
  return *novel;
}
//
Network::CreationWizard::CreationIntersection& Network::CreationWizard::createIntersection(CreationNode& _root) {
  auto * novel = new CreationIntersection(_root);
  net.intersections.push_back(novel);
  _root.owner = novel;
  return *novel;
}

Network::CreationWizard::CreationBlock& Network::CreationWizard::createBlock() {
  auto * novel = new CreationBlock();
  net.blocks.push_back(novel);
  return *novel;
}

void Network::CreationWizard::mergeNodes(CreationNode * from, CreationNode * into) {
  //copy over unique connections, and repoint the neighbors

  auto & node_connects = into->connected;
  for (auto connect : from->connected) {
    if (connect == into) continue;

    //is it unique
    if (std::find(node_connects.begin(), node_connects.end(), connect) == node_connects.end()) {
      //yes
      node_connects.push_back(connect);
      //find the backwards connect and retarget
      for (auto *& back_connect : connect->connected)
        if (back_connect == from)
          back_connect = into;
    }
    else {
      //no
      //delete the backwards connect
      for (auto *& back_connect : connect->connected)
        if (back_connect == from)
          back_connect = connect->connected.back();
      connect->connected.pop_back();
    }
  }

  //delete from, from into
  for (auto *& connect : into->connected)
    if (connect == from)
      connect = into->connected.back();
  into->connected.pop_back();

  removeNode(from);
}

void Network::generate_terrain(LineBuffer* major_tobe, LineBuffer* minor_tobe) {
  const int rays = 20;
  const int length = 10;
  const grd width = 3;

  if(major_tobe == nullptr || minor_tobe == nullptr) return;

  for (int x = -rays; x <= rays; x++) {
    for(int y = -rays; y <= rays; y++) {
      const Pgrd source((radius / rays) * x, (radius / rays) * y);
      //major
      Pgrd last = source;
      Pgrd origin_heading = terrain.SumPoint(last);
      for(int i = 0; i < length; i++){
        Pgrd adjusted_heading = terrain.SumPointContextAware(last, origin_heading);
        if (adjusted_heading.Size() <= _TENSOR_THRESH_) break;
        adjusted_heading.Normalize();

        Pgrd next = last + adjusted_heading * width;

        major_tobe->push_back(std::make_pair(last, next));

        last = next;
      }
      //minor
      last = source;
      origin_heading = { origin_heading.Y, -origin_heading.X };
      for (int i = 0; i < length; i++) {
        Pgrd adjusted_heading = terrain.SumPointContextAware(last, origin_heading);
        if(adjusted_heading.Size() <= _TENSOR_THRESH_) break;
        adjusted_heading.Normalize();
        Pgrd next = last + adjusted_heading * width;

        minor_tobe->push_back(std::make_pair(last, next));

        last = next;
      }
    }
  }
}

void Network::generate_nodes(LineBuffer* tobe) {
  wizard->generate_nodes();

  if(tobe == nullptr) return;

  for (auto * focus : nodes) {
    for (const auto * neighbor : focus->getConnections()) {
      if (neighbor > focus) continue;
      tobe->push_back(std::make_pair(focus->getPosition(), neighbor->getPosition()));
    }
  }
}

void Network::clean_nodes(LineBuffer* tobe) {
  wizard->clean_nodes();

  if (tobe == nullptr) return;

  for (auto * focus : nodes) {
    for (const auto * neighbor : focus->getConnections()) {
      if (neighbor > focus) continue;
      tobe->push_back(std::make_pair(focus->getPosition(), neighbor->getPosition()));
    }
  }
}

void Network::generate_structure(LineBuffer* tobe) {
  wizard->generate_structure();

  if (tobe == nullptr) return;

  for (auto focus : intersections) {
    const Pgrd center = focus->getRoot().getPosition();
    int p = 1;
    for (const auto & connection : focus->getConnections()) {
      const auto * neighbor = connection.second;
      Pgrd offset = neighbor->getRoot().getPosition() - center;
      offset.Normalize();
      offset *= p++;
      
      if (neighbor > focus) continue;
        tobe->push_back(std::make_pair(focus->getRoot().getPosition(), neighbor->getRoot().getPosition()));
    }
  }
}

void Network::generate_roads(PolygonBuffer* tobe, PolygonBuffer* errors) {
  wizard->generate_roads();

  if(tobe == nullptr) return;

  const grd scale = .16;
  const grd intersection_width = 3;

  auto norm_offset_sect = [](const NetworkNode* l, const NetworkNode* r, const NetworkNode* center, const grd offset) {
    Pgrd a = l->getPosition() - center->getPosition();
    Pgrd c = r->getPosition() - center->getPosition();

    if (a == c) {
      throw std::logic_error("wat");
    }

    a.Normalize();
    c.Normalize();

    if (a == c) {
      throw std::logic_error("wat");
    }

    const Pgrd at(a.Y, -a.X);

    Pgrd b = a + c;

    if (b.Dot(at) < 0) b *= -1;
    else if (b.SizeSquared() == 0) b = at;

    b.Normalize();

    if (b.Dot(at) == 0) {
      throw std::logic_error("wat");
    }

    const grd length = (offset / b.Dot(at));

    if (grd::abs(length) > 10 || std::isnan(length.n))
      throw std::logic_error("wat");

    return b * length;
  };

  auto offset_sect = [norm_offset_sect, scale](const NetworkNode* left_node, const NetworkNode* right_node, const NetworkNode* center_node) {
    const Pgrd center = center_node->getPosition();
    const int l_size = left_node->getSize();
    const int r_size = right_node->getSize();
    const int m_size = center_node->getSize();
    const int a_size = /*dynamic_cast<NetworkIntersection*>(left_node->getOwner()) ?*/ (m_size < l_size ? m_size : l_size);// : l_size;
    const int c_size = /*dynamic_cast<NetworkIntersection*>(right_node->getOwner()) ?*/ (m_size < r_size ? m_size : r_size);// : r_size;
    const grd a_offset = scale * (a_size + 1);
    const grd c_offset = scale * (c_size + 1);
    
    Pgrd a = left_node->getPosition() - center;
    Pgrd c = right_node->getPosition() - center;

    if (a == c) {
      throw std::logic_error("wat");
    }

    a.Normalize();
    c.Normalize();

    if (a == c) {
      throw std::logic_error("wat");
    }

    const Pgrd at(-a.Y, a.X);
    const Pgrd ct(-c.Y, c.X);
    const Pgrd d = at * a_offset + ct * c_offset;

    const grd _phi = a.NormDot(c).n;
    const grd phi = std::acos(_phi.clamp(-1,1).n);
    if (phi < 0.05 || std::isnan(phi.n)) {
      throw std::logic_error("wat");
    }
    else if (phi > 3.12 || at.Dot(a + c) > 0) {
      //effectively oppisite each other, use average norm bisector instead
      return norm_offset_sect(left_node, right_node, center_node, (a_offset + c_offset)/2);
    }
    const grd theta = std::acos(a.NormDot(d).n);

    const grd b = d.Size() * (std::sin(theta.n) / std::sin(phi.n)) * (at.Dot(d) > 0 ? 1 : -1);

    const Pgrd T = c * b + ct * c_offset;

    if(grd::abs(T.X) > 10 || grd::abs(T.Y) > 10 || std::isnan(T.X.n) || std::isnan(T.Y.n))
      throw std::logic_error("wat");

    return T;
  };

  std::unordered_map<const NetworkStrand *, std::pair<Pgrd, Pgrd>> fronts;
  std::unordered_map<const NetworkStrand *, std::pair<Pgrd, Pgrd>> backs;

  for (auto * focus : intersections) {
    if (focus->getRoot().getPosition().Size() > safe_radius) continue;

    std::vector<Pgrd> result;
    const int size = focus->getConnections().size();
    for (int i = 0; i < size; ++i) {
      auto& corner = focus->getCorner(i);
      std::copy(corner.begin(),corner.end(), std::back_inserter(result));
    }
    tobe->push_back(std::move(result));
  }

  for (auto * focus : strands) {
    if(focus->getStart()->getRoot().getPosition().Size() > safe_radius || focus->getEnd()->getRoot().getPosition().Size() > safe_radius) continue;

    const auto front_connections = focus->getStart()->getConnections();
    const auto back_connections = focus->getEnd()->getConnections();

    int front_index;
    int back_index;
    for(int i = 0; i < front_connections.size(); i++)
      if (front_connections[i].first == focus) {
        front_index = i;
        break;
      }

    for (int i = 0; i < back_connections.size(); i++)
      if (back_connections[i].first == focus) {
        back_index = i;
        break;
      }

    

    auto& right = focus->getRightBorder();
    auto& left = focus->getLeftBorder();
    
    std::vector<Pgrd> result;
    std::copy(right.begin(),right.end(), std::back_inserter(result));
    result.push_back(focus->getStart()->getCorner(front_index).front());
    result.push_back(focus->getStart()->getCorner((front_index + front_connections.size() - 1) % front_connections.size()).back());
    std::copy(left.begin(),left.end(), std::back_inserter(result));
    result.push_back(focus->getEnd()->getCorner(back_index).front());
    result.push_back(focus->getEnd()->getCorner((back_index + back_connections.size() - 1) % back_connections.size()).back());

    tobe->push_back(std::move(result));
  }
}

grd Area(const std::vector<Pgrd>& border) {
  grd total = 0;
  for (int i = 0; i < border.size(); ++i) {
    int j = (i + 1) % border.size();
    total += (border[i].X - border[j].X) * (border[i].Y + border[j].Y) / 2;
  }
  return total;
}

void Network::generate_blocks(PolygonBuffer* tobe) {
  //how tf do we generate blocks
  wizard->generate_blocks();

  if (tobe == nullptr) return;

  for (auto block : blocks){
    auto border = block->getBorder();
    if(Area(border) > 0)
      tobe->push_back(std::move(border));
  }
};

void Network::runIntersectionTest(PolygonBuffer* buffer) {
  CreationWizard::runIntersectionTest(buffer);
};

void Network::CreationWizard::runIntersectionTest(PolygonBuffer* buffer) {

  const grd scale = .16;
  const grd off = 5;

  auto norm_offset_sect = [](const NetworkNode* l, const NetworkNode* r, const NetworkNode* center, const grd offset) {
    Pgrd a = l->getPosition() - center->getPosition();
    Pgrd c = r->getPosition() - center->getPosition();

    if (a == c) {
      throw std::logic_error("wat");
    }

    a.Normalize();
    c.Normalize();

    if (a == c) {
      throw std::logic_error("wat");
    }

    const Pgrd at(a.Y, -a.X);

    Pgrd b = a + c;

    if (b.Dot(at) < 0) b *= -1;
    else if (b.SizeSquared() == 0) b = at;

    b.Normalize();

    if (b.Dot(at) == 0) {
      throw std::logic_error("wat");
    }

    const grd length = (offset / b.Dot(at));

    if (grd::abs(length) > 10 || std::isnan(length.n))
      throw std::logic_error("wat");

    return b * length;
  };

  auto offset_sect = [norm_offset_sect, scale](const NetworkNode* left_node, const NetworkNode* right_node, const NetworkNode* center_node) {
    const Pgrd center = center_node->getPosition();
    const int l_size = left_node->getSize();
    const int r_size = right_node->getSize();
    const int m_size = center_node->getSize();
    const int a_size = /*dynamic_cast<NetworkIntersection*>(left_node->getOwner()) ?*/ (m_size < l_size ? m_size : l_size);// : l_size;
    const int c_size = /*dynamic_cast<NetworkIntersection*>(right_node->getOwner()) ?*/ (m_size < r_size ? m_size : r_size);// : r_size;
    const grd a_offset = scale * (a_size + 1);
    const grd c_offset = scale * (c_size + 1);

    Pgrd a = left_node->getPosition() - center;
    Pgrd c = right_node->getPosition() - center;

    if (a == c) {
      throw std::logic_error("wat");
    }

    a.Normalize();
    c.Normalize();

    if (a == c) {
      throw std::logic_error("wat");
    }

    const Pgrd at(-a.Y, a.X);
    const Pgrd ct(-c.Y, c.X);
    const Pgrd d = at * a_offset + ct * c_offset;

    const grd _phi = a.NormDot(c).n;
    const grd phi = std::acos(_phi.clamp(-1, 1).n);
    if (phi < 0.05 || std::isnan(phi.n)) {
      throw std::logic_error("wat");
    }
    else if (phi > 3.12 || at.Dot(a + c) > 0) {
      //effectively oppisite each other, use average norm bisector instead
      return norm_offset_sect(left_node, right_node, center_node, (a_offset + c_offset) / 2);
    }
    const grd theta = std::acos(a.NormDot(d).n);

    const grd b = d.Size() * (std::sin(theta.n) / std::sin(phi.n)) * (at.Dot(d) > 0 ? 1 : -1);;

    const Pgrd T = c * b + ct * c_offset;

    if (grd::abs(T.X) > 10 || grd::abs(T.Y) > 10 || std::isnan(T.X.n) || std::isnan(T.Y.n))
      throw std::logic_error("wat");

    return T;
  };

  const double main_road_delta = pRandom(0.0, PI * 2);
  const double crossing_road_delta = main_road_delta - (PI / 7);
  const bool crossing_crosses = pRandom(0, 1.0) > .5;

  const Pgrd main_road_offset = rotateVector(Pgrd(0, 1), main_road_delta);
  const Pgrd crossing_road_offset = rotateVector(Pgrd(0, 1), crossing_road_delta);

  const CreationNode * focus;
  const Pgrd center(pRandom(-10,10),pRandom(-10,10));

  CreationNode * center_node = new CreationNode(center,5);//(rand() % 4) + 1);
  CreationNode * a_node = new CreationNode(center - main_road_offset, center_node->size);
  CreationNode * b_node = new CreationNode(center + main_road_offset, center_node->size);
  
  CreationNode * c_node = new CreationNode(center - crossing_road_offset, 0);//(rand() % center_node->size) + 1);
  CreationNode * d_node;
    

  center_node->addConnection(*a_node);
  center_node->addConnection(*b_node);
  center_node->addConnection(*c_node);

  if(crossing_crosses){
    d_node = new CreationNode(center + crossing_road_offset, c_node->size);
    center_node->addConnection(*d_node);
  }

  center_node->sortConnections();

  focus = center_node;

  {
    const Pgrd center = focus->position;
    std::vector<Pgrd> result;

    const int size = focus->connected.size();
  
    std::vector<Pgrd> links;

    //calculate offset bisectors
    for (auto i = 0; i < size; i++) {
      const int j = (i + 1) % size;

      Pgrd a = focus->connected[i]->position - center;
      Pgrd c = focus->connected[j]->position - center;
      a.Normalize();
      c.Normalize();

      const Pgrd at(a.Y, -a.X);

      result.push_back(center + a * off - at * (focus->connected[i]->size + 1) * scale);
      result.push_back(center + a * off + at * (focus->connected[i]->size + 1) * scale);

      const Pgrd b = offset_sect(focus->connected[i], focus->connected[j], focus);
      result.push_back(center + b);

    }

    buffer->push_back(std::move(result));
  }

  delete center_node;
  delete a_node;
  delete b_node;

  delete c_node;

  if (crossing_crosses)
    delete d_node;
};