// Fill out your copyright notice in the Description page of Project Settings.

#include "citygen.h"
#include <list>
#include <ctime>
#include <cmath>
#include <algorithm>

#define DER_EPSILON .01
#define SEED_MULT 1.5
#define EPSILON 1e-5

//inverts a vector if it is >90 degrees off from previous
Pgrd CorrectVector(const Pgrd& current, const Pgrd& previous) {
	return current.Dot(previous) < 0 ? current * -1 : current;
}

//checks if A->B->C forms a clockwise turn
bool isClockWise(const Pgrd& A, const Pgrd& B, const Pgrd& C) {
	const Pgrd N = B - A;
	const Pgrd M = C - A;
	const Pgrd N_rotated(N.Y, -N.X);

	return N_rotated.Dot(M) > 0;
}

struct NetworkNode {
	Pgrd Position;
	std::list<std::reference_wrapper<NetworkNode>> Connections;
	int Tier;
	int visited;
	NetworkNode(Pgrd P, int T) {
		Position = P;
		Tier = T;
	}
	~NetworkNode() {
	}
};

struct EdgeCollision {
	bool focus;
	std::list<std::reference_wrapper<NetworkNode>>::iterator A_iter;
	std::list<std::reference_wrapper<NetworkNode>>::iterator B_iter;
	void* A_Simple;
	void* B_Simple;
	Pgrd Position;
	EdgeCollision() {
		A_Simple = nullptr;
		B_Simple = nullptr;
	}
	const bool operator < (const EdgeCollision &r) const {
		return focus ? (A_iter < r.A_iter) : (B_iter < r.B_iter);
	}
};

struct SimpleEdge {
	using circle = std::pair<Pgrd, grd>;

	std::list<std::reference_wrapper<NetworkNode>> Contained;
	std::vector<EdgeCollision*> Collisions;
	int info;
private:
	std::list<std::reference_wrapper<NetworkNode>> BiasedSort;
	bool CollisionsSorted = false;

	circle BoundCenter;
	bool defined = false;

	circle SetupCircumscribed(const Pgrd& a, const Pgrd& b, const Pgrd& c) {
		const Pgrd G = b - a;
		const Pgrd H = c - a;

		const grd D = (G.X*H.Y - G.Y*H.X) * 2;

		const grd X = H.Y*(G.X*G.X + G.Y*G.Y) - G.Y*(H.X*H.X + H.Y*H.Y);
		const grd Y = G.X*(H.X*H.X + H.Y*H.Y) - H.X*(G.X*G.X + G.Y*G.Y);

		const Pgrd center(X / D, Y / D);

		return std::make_pair(center, (a - center).Size());
	}
	void GenerateTwoKnown(const Pgrd& a, const Pgrd& b, const int _step) {
		BoundCenter.first = (a + b) / 2;
		BoundCenter.second = (a - b).Size() / 2;
		circle LeftBound = BoundCenter;
		circle RightBound = BoundCenter;
		int step = _step;
		for (auto node : BiasedSort) {
			if (--step < 1) break;
			grd offset = (BoundCenter.first - node.get().Position).Size();
			if (offset > BoundCenter.second) {
				//construct a circle from the circumcircle
				const circle supposed = SetupCircumscribed(a, b, node.get().Position);

				//determine if its a leftbound or rightbound
				if (isClockWise(a, b, supposed.first)) {
					if (RightBound.second < supposed.second) {
						RightBound = supposed;
					}
				}
				else {
					if (LeftBound.second < supposed.second) {
						LeftBound = supposed;
					}
				}
			}
		}

		//which bound to use...
		if (LeftBound == BoundCenter) {
			BoundCenter = RightBound;
		}
		else if (RightBound == BoundCenter) {
			BoundCenter = LeftBound;
		}
		else {
			if (RightBound.second < LeftBound.second) {
				BoundCenter = RightBound;
			}
			else {
				BoundCenter = LeftBound;
			}
		}
	}
	void GenerateOneKnown(const Pgrd& a, const int _step) {
		BoundCenter.first = a;
		int step = _step;
		for (auto node : BiasedSort) {
			if (--step < 1) break;
			grd offset = (BoundCenter.first - node.get().Position).Size();
			if (offset > BoundCenter.second) {
				GenerateTwoKnown(a, node.get().Position, _step);
			}
		}
	}
public:
	void GenerateBound() {
		const int size = Contained.size();
		if (size < 1) {
			BoundCenter = std::make_pair(Pgrd(0, 0), grd(0));
		}
		else if (size < 2) {
			BoundCenter = std::make_pair(Contained.back().get().Position, grd(0));
		}
		else {
			const Pgrd& a = Contained.back().get().Position;
			const Pgrd& b = Contained.front().get().Position;
			BoundCenter.first = (a + b) / 2;
			BoundCenter.second = (a - b).Size() / 2;

			auto iter = Contained.begin();
			iter++;
			while (true) {
				auto focus = iter++; 
				if (iter == Contained.end()) break;

				grd offset = (BoundCenter.first - focus->get().Position).Size();
				if (offset > BoundCenter.second) {
					BoundCenter.second = offset;
				}
			}
		}
		defined = true;
	}
	circle ReturnBound() {
		if (!defined) GenerateBound();

		return BoundCenter;
	}
	void UpdateCollisions(SimpleEdge* against) {
		if (!defined) GenerateBound();

		CollisionsSorted = false;
		const circle op = against->ReturnBound();
		if ((BoundCenter.first - op.first).Size() >= BoundCenter.second + op.second) { return; }

		//they are within range to collide

		//if an endpoint is shared, dont check that end

		const bool skip_back = &Contained.back().get() == &against->Contained.front().get() || &Contained.back().get() == &against->Contained.back().get();
		const bool skip_front = &Contained.front().get() == &against->Contained.front().get() || &Contained.front().get() == &against->Contained.back().get();

		Pgrd temp;

		auto source_end = Contained.begin();

		//time to collision check!
		while (true) {
			auto source_start = source_end++;
			if (source_end == Contained.end()) break;

			auto compare_end = against->Contained.begin();
			while (true) {
				auto compare_start = compare_end++;
				if (compare_end == against->Contained.end()) break;

				if (Pgrd::getIntersect(source_start->get().Position, source_end->get().Position, compare_start->get().Position, compare_end->get().Position, temp)) {

					EdgeCollision* newCollision = new EdgeCollision();

					newCollision->A_iter = source_start;
					newCollision->B_iter = compare_start;
					newCollision->A_Simple = this;
					newCollision->B_Simple = against;
					newCollision->Position = temp;

					Collisions.push_back(newCollision);
					against->Collisions.push_back(newCollision);
				}
			}
		}
	}
	EdgeCollision* FindLink(bool direction, int position) {
		if (!CollisionsSorted) {
			//sort
			for (int v = 0; v < Collisions.size(); v++) {
				Collisions[v]->focus = ((SimpleEdge*)Collisions[v]->A_Simple == this);
			}
			//std::sort(Collisions.begin(), Collisions.end(), [](EdgeCollision * left, EdgeCollision * right) -> bool { return *left < *right; });
			CollisionsSorted = true;
		}
		if (direction) {
			for (int i = 0; i < Collisions.size() - 1; i++) {
				if (Collisions[i]->A_Simple == this) {
					if (Collisions[i]->A_index < position) {
						return Collisions[i + 1];
					}
				}
				else {
					if (Collisions[i]->B_index < position) {
						return Collisions[i + 1];
					}
				}
			}
		}
		else {
			for (int i = Collisions.size() - 1; i > 0; i--) {
				if (Collisions[i]->A_Simple == this) {
					if (Collisions[i]->A_index > position) {
						return Collisions[i - 1];
					}
				}
				else {
					if (Collisions[i]->B_index > position) {
						return Collisions[i - 1];
					}
				}
			}
		}

	}
};

struct NetworkStreamSeed {
	Pgrd Heading;
	bool Major;
	NetworkNode* Link;
	NetworkStreamSeed(NetworkNode* L, Pgrd H, bool M) {
		Link = L;
		Heading = H;
		Major = M;
	}
};

struct NetworkTierInfo {
	std::vector<NetworkStreamSeed*> Seeds;
	KD_TREE* ALL_Tracker;
	KD_TREE* INT_Tracker;

	int StrandLength; //how many segments in a strand
	int Tier;

	grd IntersectionSpacing;
	grd Varience;

	NetworkTierInfo() {
		Tier = -1;
		ALL_Tracker = new KD_TREE();
		INT_Tracker = new KD_TREE();
	}
	~NetworkTierInfo() {
		for (int v = 0; v < Seeds.size(); v++) {
			delete Seeds[v];
		}
		delete ALL_Tracker;
		delete INT_Tracker;
	}
};

struct BlockPolygon {
	std::vector<Pgrd>* Polygon;
	BlockPolygon() {
		Polygon = new std::vector<Pgrd>;
	}
	~BlockPolygon() {
		delete Polygon;
	}
};

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

struct FNetwork {

private:
	const CityGen& daddy;
	grd XWidth;
	grd YWidth;
	grd SegmentLength;
	grd MergeDistance;
	int MaxNodes;
	std::vector<NetworkTierInfo *> LayerInfo;
	int CurrentLayer;
	std::vector<NetworkNode*> AllNodes;
	std::vector<SimpleEdge*> AllEdges;
	std::vector<Pgrd> heightSeeds;
	std::vector<Pgrd> heightParams;
	std::vector<Pgrd> CityCenters;
	//KD_TREE* ALL_Tracker;
	//KD_TREE* INT_Tracker;


	grd IntersectionDensity(grd nominal, Pgrd position) {
		grd distance = 0; //10e9
		for (int v = 0; v < CityCenters.size(); v++) {
			//if (Pgrd::Distance(position, CityCenters[v]) < distance) {
			//	distance = Pgrd::Distance(position, CityCenters[v]);
			//}
			distance += (position - CityCenters[v]).Size();
		}

		distance /= CityCenters.size() + 1;

		return nominal * (7 - 6 * pow(2.71828, (-(distance*distance) / 10e6).n));
	}

	Pgrd DerivePoint(const Pgrd& target) {
		Pgrd target_X(target);
		Pgrd target_Y(target);
		target_X.X += DER_EPSILON;
		target_Y.Y += DER_EPSILON;

		grd C = SumField(target);
		grd X = SumField(target_X);
		grd Y = SumField(target_Y);

		return Pgrd((C - X) / DER_EPSILON, (C - Y) / DER_EPSILON);
	}
	Pgrd RK4Point(const Pgrd& position, const Pgrd& previous) {

		Pgrd a1 = DerivePoint(position);
		Pgrd k1 = CorrectVector(a1, previous);

		Pgrd a2 = DerivePoint(position) + k1 / 2;
		Pgrd k2 = CorrectVector(a2, previous);

		Pgrd a3 = DerivePoint(position) + k2 / 2;
		Pgrd k3 = CorrectVector(a3, previous);

		Pgrd a4 = DerivePoint(position) + k3;
		Pgrd k4 = CorrectVector(a4, previous);

		Pgrd a5 = k1 / 6 + k2 / 3 + k3 / 3 + k4 / 6;
		return CorrectVector(a5, previous);
	}

	void SingleStream(NetworkStreamSeed* seed/*, UWorld* bin*/) {
		//goes for a set count in layerinfo or until intersection
		int len = 0;
		Pgrd d;
		Pgrd r;
		Pgrd l = Pgrd(seed->Heading);
		Pgrd current = Pgrd(seed->Link->Position);
		NetworkNode* LastNode = seed->Link;
		int currentstep = 0;
		int futurestep = 0;
		std::vector<NetworkNode*> StrandNodes;
		std::vector<NetworkStreamSeed*> StrandSeeds;
		std::vector<NetworkStreamSeed*> FutureSeeds;
		bool KEEP = true;
		bool steep = false;

		while (len < LayerInfo[CurrentLayer]->StrandLength) {
			d = RK4Point(current, l);
			d.Normalize();
			//d *= SegmentLength;

			//0-derivative protection
			if (d.Size() < .9) {
				d = l;
			}

			Pgrd slope_check = current + d;
			if (grd::abs(SumField(current) - SumField(slope_check)) > daddy.MaxSlope) {
				steep = true;
				//OVER SLOPE
				//switch to sweeping road pattern
				
				const grd Theta = acos((daddy.MaxSlope / d.Size()).n);
				

				const grd delta = Theta * 180 / 3.1415926535;
				Pgrd forward_left = rotateVector(d, seed->Major ? delta : -delta);

				forward_left.Normalize();

				if (SumField(forward_left) < SumField(current)) {
					forward_left *= -1;
				}

				current += forward_left * SegmentLength;
				r.X = forward_left.Y;
				r.Y = -forward_left.X;

				//current += forward_left * SegmentLength;

			}
			else if (seed->Major) {
				steep = false;
				current += d * SegmentLength;
				r.X = d.Y;
				r.Y = -d.X;
			}
			else {
				steep = false;
				r.X = d.Y;
				r.Y = -d.X;
				current += r * SegmentLength;
				r = d;
			}

			
			
			if (current.X > XWidth || current.Y > YWidth || current.X < 0 || current.Y < 0) {
				break;
			}

			grd Intersect = IntersectionDensity(LayerInfo[CurrentLayer]->IntersectionSpacing, current);

			//are we close enough to another node to merge?
			void* TempPointer = nullptr;

			LayerInfo[CurrentLayer]->ALL_Tracker->FindNearest(current, TempPointer);
			NetworkNode* CompPointer = (NetworkNode*)TempPointer;
			if (CurrentLayer > 0) {
				LayerInfo[CurrentLayer-1]->ALL_Tracker->FindNearest(current, TempPointer);
				NetworkNode* PossiblePointer = (NetworkNode*)TempPointer;
				if (CompPointer == nullptr) {
					CompPointer = PossiblePointer;
				}
				else if ((CompPointer->Position - current).Size() > (PossiblePointer->Position - current).Size()) {
					CompPointer = PossiblePointer;
				}
			}

			if (CompPointer != nullptr && CompPointer != seed->Link && (CompPointer->Position - current).Size() < MergeDistance) {

				//merge to the nearby node
				LayerInfo[CurrentLayer]->INT_Tracker->AddPoint(CompPointer->Position, CompPointer);
				Pgrd temp = CompPointer->Position;

				LastNode->Connections.push_back(CompPointer);
				CompPointer->Connections.push_back(LastNode);

				StrandSeeds.push_back(new NetworkStreamSeed(CompPointer, d, seed->Major));

				break;
			}

			//add a point to the network, and ALL_Tracker
			NetworkNode* nextNode = new NetworkNode(Pgrd(current), LayerInfo[CurrentLayer]->Tier);
			StrandNodes.push_back(nextNode);

			LastNode->Connections.push_back(nextNode);
			nextNode->Connections.push_back(LastNode);

			if (currentstep % 2 == 1){// && !steep) {
				std::vector<void*> NearbyNodes;
				Pgrd marker;
				Pgrd LeftTemp = current + r * Intersect;
				Pgrd RightTemp = current - r * Intersect;
				
				Pgrd Direction = LeftTemp - current;
				Direction.Normalize();

				LayerInfo[CurrentLayer]->ALL_Tracker->CollectRange(current, Intersect, NearbyNodes);
				if (CurrentLayer > 0) {
					LayerInfo[CurrentLayer - 1]->ALL_Tracker->CollectRange(current, Intersect, NearbyNodes);
				}
				for (int v = 0; v < NearbyNodes.size(); v++) {
					//try and intersect all edges
					NetworkNode* CompPointer = (NetworkNode*)NearbyNodes[v];
					for (auto connect : CompPointer->Connections) {

						Pgrd partner = connect->Position;

						Pgrd offset = CompPointer->Position - partner;

						offset.Normalize();
						
						if (Pgrd::getIntersect(CompPointer->Position, partner, current, LeftTemp, marker)) {
							if (steep) {
								if (grd::abs(Direction.Dot(offset)) < daddy.HillThresh) {
									KEEP = false;
									break;
								}
							}
							else {
								if (grd::abs(Direction.Dot(offset)) < daddy.StockThresh) {
									KEEP = false;
									break;
								}
							}
						}
						else if (Pgrd::getIntersect(CompPointer->Position, partner, current, RightTemp, marker)) {
							if (grd::abs(Direction.Dot(offset))<.2) {
								/*DrawDebugLine(
									bin,
									FVector(current, 0),
									FVector(RightTemp, 0),
									FColor(0, 255, 0),
									true,
									-1,
									0,
									8
								);
								DrawDebugPoint(
									bin,
									FVector(current, 0),
									10,
									FColor(0, 0, 0),
									true,
									-1,
									0
								);*/
								KEEP = false;
								break;
							}
						}
					}
					if (!KEEP) { break; }
				}
				if (!KEEP) { break; }
			}
			//else if (currentstep % 2 == 1 && steep) {
				//the metric for spacing hill sweeping patterns

			//}

			//add seeds if spaced out
			if ((grd)(currentstep++) > Intersect * daddy.SeedCoef / SegmentLength) {

				StrandSeeds.push_back(new NetworkStreamSeed(nextNode, d, !seed->Major));
				StrandSeeds.push_back(new NetworkStreamSeed(nextNode, d * -1, !seed->Major));
				currentstep = 0;
			}
			if (CurrentLayer<LayerInfo.size()-1){
				if ((grd)(futurestep++) > IntersectionDensity(LayerInfo[CurrentLayer+1]->IntersectionSpacing, current) * daddy.SeedCoef / SegmentLength && currentstep > 0) {
					// if another road is to close to either of these seeds, dont add them
					FutureSeeds.push_back(new NetworkStreamSeed(nextNode, d, !seed->Major));
					FutureSeeds.push_back(new NetworkStreamSeed(nextNode, d * -1, !seed->Major));
					futurestep = 0;
				}
			}

			LastNode = nextNode;
			len++;
			l = d;
		}
		if (KEEP&&len<1) {
			KEEP = false;
		}
		if (KEEP) {
			if (len >= LayerInfo[CurrentLayer]->StrandLength) {
				StrandSeeds.push_back(new NetworkStreamSeed(LastNode, d, seed->Major));
			}
			for (int v = 0; v < StrandNodes.size(); v++) {
				AllNodes.push_back(StrandNodes[v]);
				LayerInfo[CurrentLayer]->ALL_Tracker->AddPoint(StrandNodes[v]->Position, StrandNodes[v]);
			}
			for (int v = 0; v < StrandSeeds.size(); v++) {
				LayerInfo[CurrentLayer]->Seeds.push_back(StrandSeeds[v]);
			}
			if (CurrentLayer<LayerInfo.size() - 1) {
				for (int v = 0; v < FutureSeeds.size(); v++) {
					LayerInfo[CurrentLayer + 1]->Seeds.push_back(FutureSeeds[v]);
				}
			}
			if (seed->Link->Connections.size() > 2) {
				LayerInfo[CurrentLayer]->INT_Tracker->AddPoint(seed->Link->Position, seed->Link);
				Pgrd temp = seed->Link->Position;
				/*DrawDebugPoint(
					bin,
					temp,
					5,
					FColor(0, 255, 255),
					true,
					1,//-1,
					0
				);*/
			}
		}
		else {
			if (StrandNodes.size() > 0) {
				seed->Link->Connections.pop_front();
			}
			for (int v = 0; v < StrandNodes.size(); v++) {
				delete StrandNodes[v];
			}
			for (int v = 0; v < StrandSeeds.size(); v++) {
				delete StrandSeeds[v];
			}
			for (int v = 0; v < FutureSeeds.size(); v++) {
				delete FutureSeeds[v];
			}
		}
	}
	void GenerateNetworkLayer(/*UWorld* bin*/) {
		// we need to alter generate streams to use a seed database instead of sorting through streams
		// we will also implement a k-d tree for nearest search

		//std::vector<NetworkStreamSeed> LSeeds;

		while (!LayerInfo[CurrentLayer]->Seeds.empty() && AllNodes.size() < MaxNodes) {
			NetworkStreamSeed* NEXT = LayerInfo[CurrentLayer]->Seeds.back();
			LayerInfo[CurrentLayer]->Seeds.pop_back();

			void* TempPointer = nullptr;

			LayerInfo[CurrentLayer]->INT_Tracker->FindNearest(NEXT->Link->Position, TempPointer);

			NetworkNode* CompPointer = (NetworkNode*)TempPointer;

			if (CurrentLayer > 0) {
				LayerInfo[CurrentLayer - 1]->INT_Tracker->FindNearest(NEXT->Link->Position, TempPointer);
				NetworkNode* PossiblePointer = (NetworkNode*)TempPointer;
				if (CompPointer == nullptr) {
					CompPointer = PossiblePointer;
				}
				else if ((CompPointer->Position - NEXT->Link->Position).Size() > (PossiblePointer->Position - NEXT->Link->Position).Size()) {
					CompPointer = PossiblePointer;
				}
			}
			if (CompPointer != nullptr && CompPointer != NEXT->Link && (CompPointer->Position - NEXT->Link->Position).Size() < IntersectionDensity(LayerInfo[CurrentLayer]->IntersectionSpacing, NEXT->Link->Position)) {
				/*DrawDebugPoint(
					bin,
					FVector(NEXT->Link->Position, 0),
					10,
					FColor(0, 0, 0),
					true,
					-1,
					0
				);*/
				//delete NEXT;
				//continue;
			}

			//generate a small stream, then cap with another seed
			SingleStream(NEXT/*, bin*/);
			delete NEXT;
		}

	};

public:
	FNetwork(int MaxSize, grd SL, grd X, grd Y, const CityGen& D)
	: daddy(D) {
		//ALL_Tracker = new KD_TREE();
		//INT_Tracker = new KD_TREE();
		SegmentLength = SL;
		MergeDistance = SL*.71;
		MaxNodes = MaxSize;
		XWidth = X;
		YWidth = Y;
	};
	~FNetwork() {
		for (int v = 0; v < AllNodes.size(); v++) {
			delete AllNodes[v];
		}
		//delete ALL_Tracker;
		//delete INT_Tracker;
	}
	
	int NodeCount() const {
		return AllNodes.size();
	}
	int EdgeCount() const {
		return AllEdges.size();
	}
	void GenerateTerrain(int hills, int bumps, int cities) {
		heightSeeds.clear();
		heightParams.clear();

		for (int v = 0; v < hills; v++) {
			Pgrd temp_a;
			Pgrd temp_b;

			temp_a.X = pRandom(0, XWidth.n);
			temp_a.Y = pRandom(0, YWidth.n);

			temp_b.X = pRandom(300, 800);
			temp_b.Y = pRandom(1.5, 2.5) * 2000000;

			heightSeeds.push_back(temp_a);
			heightParams.push_back(temp_b);
		}

		for (int v = 0; v < bumps; v++) {
			Pgrd temp_a;
			Pgrd temp_b;
			temp_a.X = pRandom(0, XWidth.n);
			temp_a.Y = pRandom(0, YWidth.n);

			temp_b.X = pRandom(50, 150);
			temp_b.Y = pRandom(1, 2) * 500000;

			heightSeeds.push_back(temp_a);
			heightParams.push_back(temp_b);
		}

		for (int v = 0; v < cities; v++) {
			Pgrd temp_a;
			temp_a.X = pRandom(0, XWidth.n);
			temp_a.Y = pRandom(0, YWidth.n);

			CityCenters.push_back(temp_a);
		}
	}
	
	void PushLayerInfo(NetworkTierInfo& TopLayer) {
		LayerInfo.push_back(&TopLayer);
	}
	
	void GenerateNetwork(/*UWorld* bin*/) {

		NetworkNode* nextNode = new NetworkNode(Pgrd(XWidth / 2, YWidth / 2), LayerInfo[0]->Tier);
		AllNodes.push_back(nextNode);

		LayerInfo[0]->Seeds.push_back(new NetworkStreamSeed(nextNode, Pgrd(1, 0), true));
		LayerInfo[0]->Seeds.push_back(new NetworkStreamSeed(nextNode, Pgrd(-1, 0), true));

		LayerInfo[0]->ALL_Tracker->AddPoint(nextNode->Position, nextNode);

		for(CurrentLayer=0; CurrentLayer < LayerInfo.size(); CurrentLayer++){
			GenerateNetworkLayer(/*bin*/);
		}

	}

	void GenerateSimpleDiagram() {
		for (int v = 0; v < AllNodes.size(); v++) {
			AllNodes[v]->visited = 0;
		}

		std::list<NetworkNode*> Frontier;
		Frontier.push_back(AllNodes[0]);
		while (Frontier.size() > 0) {

			NetworkNode* target = Frontier.back();
			Frontier.pop_back();

			target->visited = 2;
			for (auto connect : target->Connections) {

				NetworkNode* Last = target;
				NetworkNode* Current = connect;

				SimpleEdge* Strand = new SimpleEdge;
				//Strand->Start = target;

				Strand->Contained.push_back(target);

				while (Current->Connections.size() == 2) {

					Strand->Contained.push_back(Current);

					if (Current->Connections.front() == Last) {
						Last = Current;
						Current = Current->Connections.back();
					}
					else {
						Last = Current;
						Current = Current->Connections.front();
					}
				}

				if (Current->visited < 2) {
					Strand->Contained.push_back(Current);
					//Strand->End = Current;
					Current->visited = 1;
					Frontier.push_back(Current);
					AllEdges.push_back(Strand);
					Strand->GenerateBound();
				}
			}
		}
		for (int x = 0; x < AllEdges.size()-1; x++) {
			for (int y = x+1; y < AllEdges.size(); y++) {
				AllEdges[x]->UpdateCollisions(AllEdges[y]);
			}
		}
	}
	void GenerateBlockPolygons(std::vector<BlockPolygon>* buffer) {
		//problem: the graph isn't entirely planar :/
		for (int i = 0; i < AllEdges.size(); i++) {
			AllEdges[i]->info = 0;
		}
		int index = 0;
		SimpleEdge* Initial;
		SimpleEdge* Current;
		int Position;
		bool Direction;

		//pick an edge and direction that hasn't been covered
		while (index < AllEdges.size()) {
			//log where we started (if we end up tracing)
			Initial = AllEdges[index];

			if (Initial->info < 3) {
				Current = Initial;
				if (Initial->info % 2 == 0) {
					Direction = false;
					Position = Initial->Contained.size() - 1;
				}
				else {
					Direction = true;
					Position = 0;
				}

				do {
					if (Direction) {
						Current->info++;
					}
					else {
						Current->info += 2;
					}

					//find next
					EdgeCollision* temp = Current->FindLink(Direction, Position);

					//find direction


				} while (Current != Initial);



			}
			else {
				index++;
			}
		}
	}


	std::vector<std::pair<Pgrd, Pgrd>> Display() {
		std::vector<std::pair<Pgrd, Pgrd>> buffer;
		for(auto focus : AllNodes)
			for (auto partner : focus->Connections)
				if (partner > focus) {
					buffer.push_back(std::make_pair(focus->Position, partner->Position));
				}
		return buffer;
	}
	/*void DisplaySimple(FVector Offset, UWorld* bin) {
		for (int v = 0; v < AllEdges.Num(); v++) {
			SimpleEdge* temp = AllEdges[v];

			NetworkNode* Start = (*temp->Contained)[0];
			NetworkNode* End = temp->Contained->Last();

			FVector start(Start->Position.X + Offset.X, Start->Position.Y + Offset.Y, SumField(&Start->Position) + Offset.Z);
			temp->Contained->Last();
			FVector end(End->Position.X + Offset.X, End->Position.Y + Offset.Y, SumField(&End->Position) + Offset.Z);
			if (Start->Tier > 1 && End->Tier > 1) {
				DrawDebugLine(
					bin,
					start,
					end,
					FColor(255, 0, 0),
					true,
					-1,
					0,
					8
				);
			}
			else if (Start->Tier > 0 && End->Tier > 0) {
				DrawDebugLine(
					bin,
					start,
					end,
					FColor(255, 0, 255),
					true,
					-1,
					0,
					8
				);
			}
			else {
				DrawDebugLine(
					bin,
					start,
					end,
					FColor(0, 0, 255),
					true,
					-1,
					0,
					5
				);
			}
			FVector bound = temp->ReturnBound();
			float radius = bound.Z;
			if (radius < 1000 || temp->Contained->Num()>100) {
				bound.Z = (start.Z + end.Z) / 2;
				DrawDebugSphere(
					bin,
					bound,
					radius,
					8,
					FColor(255, 0, 0),
					true,
					-1
				);
			}
			else {
				radius++;
			}
		}
	}*/
};

// Called when the game starts
CityGen::CityGen()
{
	

	//MyCustomNet.DisplaySimple(FVector(0,0,0), GetWorld());
}

std::vector<std::pair<Pgrd, Pgrd>> CityGen::Fill() {
	FNetwork MyCustomNet(SegMax, SegLength, Area_Size, Area_Size, *this);

	NetworkTierInfo Alpha, Beta, Charlie;
	Alpha.StrandLength = HighLength;
	Alpha.IntersectionSpacing = SegLength * HighDensity;
	Alpha.Tier = 2;

	Beta.StrandLength = MidLength;
	Beta.IntersectionSpacing = SegLength * MidDensity;
	Beta.Tier = 1;

	Charlie.StrandLength = LowLength;
	Charlie.IntersectionSpacing = SegLength * LowDensity;
	Charlie.Tier = 0;

	MyCustomNet.GenerateTerrain(MntCount, HillCount, CityCount);
	MyCustomNet.PushLayerInfo(Alpha);
	MyCustomNet.PushLayerInfo(Beta);
	MyCustomNet.PushLayerInfo(Charlie);
	MyCustomNet.GenerateNetwork(/*GetWorld()*/);


	MyCustomNet.GenerateSimpleDiagram();

	return MyCustomNet.Display();
}