#include "tracer.h"
#include "kd_tree.h"
#include "field_model.h"

#include <vector>
#include <algorithm>
#include <map>

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

#define RK4_EPSILON .01

//contains field functions for elevation, 
class TerrainFunction {
	struct TerrainSeed {
		grd magnitude;
		grd spread;
		Pgrd location;
	};

	grd CurvedHeight(const TerrainSeed& node, grd distance) const {
		const grd steepness = node.spread;
		const grd max_height = node.magnitude;

		return max_height * pow(2.71828, (-(distance*distance) / (steepness)).n);
	}

	Pgrd DerivePoint(const Pgrd& target) const {
		Pgrd target_X(target);
		Pgrd target_Y(target);
		target_X.X += RK4_EPSILON;
		target_Y.Y += RK4_EPSILON;

		grd C = SumField(target);
		grd X = SumField(target_X);
		grd Y = SumField(target_Y);

		return Pgrd((C - X) / RK4_EPSILON, (C - Y) / RK4_EPSILON);
	}

	Pgrd CorrectVector(const Pgrd& current, const Pgrd& previous) const {
		return current.Dot(previous) < 0 ? current * -1 : current;
	}
	
public:
	void RegenerateTerrain(const Pgrd& size, int hills, int bumps) {
		seeds.clear();
		seeds.reserve(hills + bumps);

		for (int v = 0; v < hills; v++) {
			TerrainSeed temp;
			temp.location = Pgrd(pRandom(0, size.X.n), pRandom(0, size.Y.n));
			temp.magnitude = pRandom(300, 800);
			temp.spread = pRandom(1.5, 2.5) * 2000000;

			seeds.push_back(temp);
		}

		for (int v = 0; v < bumps; v++) {
			TerrainSeed temp;
			temp.location = Pgrd(pRandom(0, size.X.n), pRandom(0, size.Y.n));
			temp.magnitude = pRandom(50, 150);
			temp.spread = pRandom(1, 2) * 500000;

			seeds.push_back(temp);
		}
	}

	grd SumField(const Pgrd& target) const {
		grd total = 0;
		for (const auto& seed : seeds) {
			grd distance = (target - seed.location).Size();
			total += CurvedHeight(seed, distance);
		}
		return total;
	}

	Pgrd RK4Point(const Pgrd& position, const Pgrd& previous) const {

		Pgrd a1 = DerivePoint(position);
		Pgrd k1 = CorrectVector(a1, previous);

		Pgrd a2 = DerivePoint(position + k1 / 2);
		Pgrd k2 = CorrectVector(a2, previous);

		Pgrd a3 = DerivePoint(position + k2 / 2);
		Pgrd k3 = CorrectVector(a3, previous);

		Pgrd a4 = DerivePoint(position + k3);
		Pgrd k4 = CorrectVector(a4, previous);

		Pgrd a5 = k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6;
		return CorrectVector(a5, previous);
	}

	grd IntersectionDensity(grd nominal, Pgrd position) const {
		grd distance = 0; //10e9
		for (auto city : cities) {
			//if (Pgrd::Distance(position, CityCenters[v]) < distance) {
			//	distance = Pgrd::Distance(position, CityCenters[v]);
			//}
			distance += (position - city).Size();
		}

		distance /= cities.size() + 1;

		return nominal * (7 - 6 * pow(2.71828, (-(distance*distance) / 10e6).n));
	}

private:
	std::vector<TerrainSeed> seeds;
	std::vector<Pgrd> cities;
};



//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
// Declarations

class Network {
	class NetworkNode;
	class NetworkTier;
	class NetworkSeed;
	class NetworkStrand;
	class NetworkIntersection;

	enum OriginType {
		None = 0,
		Branch,
		Extension
	};

	// CONSTRUCTION AND BAREBONES ELEMENTS

	//represents a node
	class NetworkNode {

		int id;
		bool explored = false;
		std::vector<NetworkNode*> connected;

		Pgrd position;

	public:
		NetworkNode(const Pgrd& _position, int _id);
		NetworkNode(const NetworkNode&) = delete;
		NetworkNode(NetworkNode&&) = delete;

		Pgrd getPosition() const { return position; }
		//adds connection to each node, and fills in int_tracker if needed
		void addConnection(NetworkNode& partner) { connected.push_back(&partner); partner.connected.push_back(this); }
		const std::vector<NetworkNode*>& getConnections() const { return connected; }
		void cw_sort_connected() {
			std::sort(connected.begin(), connected.end(), [](NetworkNode* a, NetworkNode* b) {
				const Pgrd pA = a->getPosition();
				const Pgrd pB = b->getPosition();
				const grd rA = std::atan2(pA.Y.n, pA.X.n) + (pA.Y.n > 0) * 3.1415926;
				const grd rB = std::atan2(pB.Y.n, pB.X.n) + (pB.Y.n > 0) * 3.1415926;
				return rA > rB; 
			});
		}
		int getID() const { return id; }

		static NetworkNode& splitConnection(NetworkTier& tier, NetworkNode& a, NetworkNode& b, const Pgrd& pos) {
			//if (&a.net != &b.net) throw;
			//if (&a.tier != &b.tier) throw;
			if (auto iter = std::find(a.connected.begin(), a.connected.end(), &b); iter != a.connected.end())
				a.connected.erase(iter);
			else throw;
			if (auto iter = std::find(b.connected.begin(), b.connected.end(), &a); iter != b.connected.end())
				b.connected.erase(iter);
			else throw;

			auto * node = tier.createNode(pos);
			if (!node) throw;

			a.addConnection(*node);
			b.addConnection(*node);

			return *node;
		}

		bool WithinDistanceOf(NetworkNode& a, int distance) {
			std::vector<NetworkNode*> mapped;
			std::vector<NetworkNode*> frontier;
			std::vector<NetworkNode*> swap;

			mapped.push_back(this);
			swap.push_back(this);
			explored = true;

			bool found = false;

			for (auto i = 0; i < distance; ++i) {
				frontier.swap(swap);
				while (!frontier.empty()) {
					auto & n = frontier.back()->connected;
					for (auto c : n) {
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

			for (auto t : mapped)
				t->explored = false;

			return found;
			
		}
	};

	//represents a hierarchical tier in the network, responsible for expansion of a given tier and seeding of the next
	class NetworkTier {
		Network& net;

		//represents a route of expansion, the frontier
		struct NetworkSeed {
			Pgrd heading;
			bool major;
			NetworkNode& node;
			OriginType type;

			NetworkSeed(const Pgrd& _heading, bool _major, NetworkNode& _node, OriginType _type);
		};

		NetworkTier* next; //if this tier has a lower hierarchy tier to generate seeds for
		std::list<NetworkSeed> frontier;

		int strand_length = 8; //edges in strand
		grd edge_length = 2;
		grd intersection_spacing = 12;
		grd varience = 0;
		grd grid_threshold = 1;
		grd max_slope = 2;
		grd merge_distance = 1.5;
		grd seed_coef = 1;
		grd stock_thresh = .2;

		void SingleStream(const NetworkSeed next);

	public:
		NetworkTier(Network& _net, NetworkTier * _next = nullptr);
		NetworkTier(const NetworkTier&) = delete;

		NetworkNode* createNode(const Pgrd& _position);

		bool Generate(int count = 100);

		//create a novel seed and node
		void create_seed(const Pgrd& _position, const Pgrd& _heading, bool _major, OriginType _type);
	private:
		//creates a seed from an upper networks node
		void create_seed(const Pgrd& _heading, bool _major, NetworkNode & _node, OriginType _type, bool important = false);
	};

	// GROUPING AND CONNECTEDNESS ELEMENTS

	//represents a path in the network such that if one contracted the path to a single edge [ members.front <-> members.back ] it would not change the network topology
	// or: represents a section of road with no internal intersections (the ends may be intersections or dead ends)
	class NetworkStrand {
		Network& net;

		NetworkTier& tier;
		std::vector<NetworkNode*> members;

	public:
		NetworkStrand(Network& _net, NetworkTier& _tier);
		NetworkStrand(const NetworkStrand&) = delete;
	};
	//represents an end of strands (shared by point), or road intersection
	class NetworkIntersection {
		Network& net;

		struct relation {
			int strand;
			int intersection;
		};

		NetworkNode& root;
		std::vector<relation> connected;

	public:
		NetworkIntersection(Network& _net, NetworkNode& _root);
		NetworkIntersection(const NetworkStrand&) = delete;
	};

	const Tensor_Field& terrain;
	const Pgrd size;
	bool finished = false;
	int last_wrote = 0;

	KD_TREE<NetworkNode*> all_tracker; //tracks all nodes
	KD_TREE<NetworkNode*> int_tracker; //tracks intersection nodes

	std::vector<NetworkNode*> nodes;
	std::vector<NetworkTier*> tiers;
	std::vector<NetworkStrand*> strands;
	std::vector<NetworkIntersection*> intersections;

	NetworkTier* createTier(NetworkTier * _next = nullptr);
	NetworkStrand* createStrand(NetworkTier& _tier);
	NetworkIntersection* createIntersection(NetworkNode& _root);

public:
	Network(const Tensor_Field& _terrain, const Pgrd);
	~Network();

	bool run(Buffer& tobe, int count = 100);
	bool isFinished() const { return finished; }
};

//---------------------------------------------------------------------------------------------
// NetworkNode

Network::NetworkNode::NetworkNode(const Pgrd& _position, int _id)
: position(_position)
, id(_id) {};

//---------------------------------------------------------------------------------------------
// NetworkSeed

Network::NetworkTier::NetworkSeed::NetworkSeed(const Pgrd& _heading, bool _major, NetworkNode& _node, OriginType _type)
: heading(_heading)
, major(_major)
, node(_node)
, type(_type){};

//---------------------------------------------------------------------------------------------
// NetworkTier

Network::NetworkTier::NetworkTier(Network& _net, NetworkTier * _next)
: net(_net)
, next(_next) {};

//TODO: replace with project and skew method (with pre-emptive crossing check)
void Network::NetworkTier::SingleStream(const NetworkSeed seed) {
	//goes for a set count in layerinfo or until intersection
	

	Pgrd current_heading = seed.heading;
	Pgrd adjusted_heading, perpendicular;

	Pgrd current = seed.node.getPosition();
	Pgrd last = current;

	std::vector<std::pair<Pgrd,Pgrd>> suggested_spots;

	NetworkNode * connect = nullptr;
	bool KEEP = true;
	bool BOUNDED = true;
	int len = 0;

	//are we too close to another existing intersection
	if (seed.node.getConnections().size() < 3) {
		const grd starting_intersect = intersection_spacing;// net.terrain.IntersectionDensity(intersection_spacing, current);
		auto starting_range = net.int_tracker.CollectRange(current, starting_intersect);
		for (auto branch : starting_range) {
			auto & test = *(branch->Data());
			if (seed.node.WithinDistanceOf(test, 12))
				return;
		}
	}

	while (len < strand_length) {
		adjusted_heading = net.terrain.SumPoint(current);
		if (adjusted_heading.Dot(current_heading) < 0)
			adjusted_heading *= -1;

		//adjusted_heading = net.terrain.RK4Point(current, current_heading);
		//grd slope = grd::abs(adjusted_heading.Size());

		//grd bias; //.08 to .1
		//if (slope < .08) {
		//	adjusted_heading = current_heading;
		//}
		//else if (slope < .1) {
		//	bias = (slope - .08) * 50;
		//	adjusted_heading.Normalize();
		//	adjusted_heading = adjusted_heading * bias + current_heading * (grd(1) - bias);
		//}

		adjusted_heading.Normalize();

		perpendicular.X = adjusted_heading.Y;
		perpendicular.Y = -adjusted_heading.X;

		if (seed.major) {
			current += adjusted_heading * edge_length;
		}
		else {
			current += perpendicular * edge_length;
			perpendicular = adjusted_heading;
		}

		{	//will this strand respect the bounds?
			if (grd::abs(current.X) > net.size.X || grd::abs(current.Y) > net.size.Y) {
				BOUNDED = false;
				break;
			}
		}

		{	//are we close enough to another intersection to merge?
			if (auto* branch = net.int_tracker.FindNearest(current); branch != nullptr) {
				NetworkNode* CompPointer = branch->Data();
				if (CompPointer != nullptr && CompPointer != &seed.node && (CompPointer->getPosition() - current).Size() < merge_distance) {
					if (len == 0) {
						for (const auto connect : CompPointer->getConnections()) {
							if (connect == &seed.node) {
								KEEP = false;
								break;
							}
						}
					}

					connect = CompPointer;
					break;
				}
			}
		}

		//if(len == 0){
		//	auto nearby_nodes = net.all_tracker.CollectRange(current, merge_distance);
		//	for (auto node_entry : nearby_nodes) {
		//		auto & node = *(node_entry->Data());

		//		if (&node == &seed.node) continue;

		//		if (seed.node.WithinDistanceOf(node, 2)) {
		//			KEEP = false;
		//			break;
		//		}
		//	}
		//	if (!KEEP) break;
		//}

		//get local points
		

		{	//are we crossing an edge, and should we merge
			const grd intersect = intersection_spacing;// net.terrain.IntersectionDensity(intersection_spacing, current);
			auto nearby_nodes = net.all_tracker.CollectRange(current, intersect);

			Pgrd marker;
			bool hit = false;

			for (auto node_entry : nearby_nodes) {
				auto & node = *(node_entry->Data());
				const Pgrd A_S = node.getPosition();

				if (&node == &seed.node) continue;

				for (auto edge : node.getConnections()) { //take the closest?
					auto & partner = *edge;
					if (&partner == &seed.node) continue;

					const Pgrd A_E = partner.getPosition();
					if (Pgrd::getIntersect(A_S, A_E, last, current, marker)) {
						if (marker == A_S) {
							connect = &node;
						}
						else if (marker == A_E) {
							connect = &partner;
						}
						else {
							connect = &NetworkNode::splitConnection(*this, node, partner, marker);
						}
						hit = true;
						break;
					}
				}
				if (hit) break;
			}

			if (hit) break;
		}

		{	//are we close enough to another point to merge?
			if (auto* branch = net.all_tracker.FindNearest(last); branch != nullptr) {
				NetworkNode* CompPointer = branch->Data();
				if (CompPointer != nullptr && CompPointer != &seed.node && (CompPointer->getPosition() - last).Size() < merge_distance) {

					connect = CompPointer;
					break;
				}
			}
		}

		//the point has been accepted
		suggested_spots.push_back(std::make_pair(current, adjusted_heading));

		//setup for the next loop
		last = current;
		len++;
		current_heading = adjusted_heading;
	}

	if (connect && seed.node.WithinDistanceOf(*connect, 12)) 
		KEEP = false;

	if (KEEP) {
		//synthesize path, and add seeds
		//add seeds if spaced out
		int currentstep = 0;
		int futurestep = 0;

		NetworkNode* last_node = &seed.node;
		for (auto point : suggested_spots) {
			NetworkNode *next_node = createNode(point.first);
			last_node->addConnection(*next_node);

			const grd intersect = intersection_spacing;// net.terrain.IntersectionDensity(intersection_spacing, point.first);

			if ((grd)(currentstep++) > (intersect * seed_coef) / edge_length) {
				create_seed(point.second, !seed.major, *next_node, OriginType::Branch);
				create_seed(point.second * -1, !seed.major, *next_node, OriginType::Branch);
				currentstep = 0;
			}
			else if (next) {
				if ((intersect * seed_coef) / edge_length < futurestep++) {

					// if another road is to close to either of these seeds, dont add them
					next->create_seed(point.second, !seed.major, *next_node, OriginType::Branch);
					next->create_seed(point.second * -1, !seed.major, *next_node, OriginType::Branch);
					futurestep = 0;
				}
			}

			currentstep++;
			futurestep++;
			last_node = next_node;
		}

		if (connect) {
			connect->addConnection(*last_node);

			net.int_tracker.AddPoint(connect->getPosition(), connect);

			last_node = connect;
		}

		if(BOUNDED)
			create_seed(current_heading, seed.major, *last_node, OriginType::Extension, true);


		//origin point intersection check
		if (seed.node.getConnections().size() > 2) {
			net.int_tracker.AddPoint(seed.node.getPosition(), &seed.node);
		}

		


	}
}

bool Network::NetworkTier::Generate(int count) {
	while (!frontier.empty() && count > 0) {
		NetworkSeed next = frontier.front();
		frontier.pop_front();
		
		SingleStream(next);
		--count;
	}

	return count != 0;
}

void Network::NetworkTier::create_seed(const Pgrd& _position, const Pgrd& _heading, bool _major, OriginType _type) {
	auto* novel = createNode(_position);
	frontier.push_back(NetworkSeed(_heading, _major, *novel, _type));
}

void Network::NetworkTier::create_seed(const Pgrd& _heading, bool _major, NetworkNode & _node, OriginType _type, bool important) {
	if(important)
		frontier.push_front(NetworkSeed(_heading, _major, _node, _type));
	else
		frontier.push_back(NetworkSeed(_heading, _major, _node, _type));
}

//---------------------------------------------------------------------------------------------
// NetworkStrand

Network::NetworkStrand::NetworkStrand(Network& _net, NetworkTier& _tier)
: net(_net)
, tier(_tier) {};

//---------------------------------------------------------------------------------------------
// NetworkIntersection

Network::NetworkIntersection::NetworkIntersection(Network& _net, NetworkNode& _root) 
: net(_net)
, root(_root) {};

//---------------------------------------------------------------------------------------------
// Network

Network::Network(const Tensor_Field& _terrain, const Pgrd _size)
: terrain(_terrain), size(_size) {
	auto * tier = createTier(nullptr);
	tier->create_seed(Pgrd(0, 0), Pgrd(1, 0), true, OriginType::None);
	tier->create_seed(Pgrd(0, 0), Pgrd(-1, 0), true, OriginType::None);
}

Network::~Network() {
	for (auto node : nodes) delete node;
	for (auto tier : tiers) delete tier;
	for (auto strand : strands) delete strand;
	for (auto intersection : intersections) delete intersection;
}

Network::NetworkNode* Network::NetworkTier::createNode(const Pgrd& _position) {
	auto* novel = new NetworkNode(_position, net.nodes.size());
	net.all_tracker.AddPoint(_position, novel);
	net.nodes.push_back(novel);
	return novel;
}

Network::NetworkTier* Network::createTier(NetworkTier * _next) {
	auto novel = new NetworkTier(*this, _next);
	tiers.push_back(novel);
	return novel;
}

Network::NetworkStrand* Network::createStrand(NetworkTier& _tier) {
	auto novel = new NetworkStrand(*this, _tier);
	strands.push_back(novel);
	return novel;
}

Network::NetworkIntersection* Network::createIntersection(NetworkNode& _root) {
	auto novel = new NetworkIntersection(*this, _root);
	intersections.push_back(novel);
	return novel;
}

bool Network::run(Buffer& tobe, int count) {
	finished |= tiers[0]->Generate(count);

	const int made = nodes.size();
	for (int c = 0; c < made; ++c) {
		const auto * focus = nodes[c];
		for (const auto * connect : focus->getConnections()) {
			if (connect->getID() > focus->getID()) continue;

			tobe.push_back(std::make_pair(focus->getPosition(), connect->getPosition()));
		}
	}

	last_wrote = made;
	return finished;
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

struct Tracer::Data {
	Tensor_Field terrain;
	Network network;

	Data();
};

Tracer::Data::Data()
	: terrain(0.001)
	, network(terrain, Pgrd(300,300)) {

	terrain.AddEffect(new Radial_Effect(Pgrd(10,10),500));
	//terrain.RegenerateTerrain(Pgrd(300, 300), 0, 2);
}

Tracer::Tracer() 
: d(new Data()) {

}

Tracer::~Tracer() {
	delete d;
}
bool Tracer::run(Buffer& tobe, int step_count) {
	return d->network.run(tobe, step_count);
}