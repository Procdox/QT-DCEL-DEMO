#include "DCEL.h"

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//	Point

Point::Point(DCEL * uni) {
	universe = uni;

	mark = 0;
};
Point::~Point() {

}
void Point::setPosition(Pgrd p) {
	position = p;
};
Pgrd Point::getPosition() const {
	return position;
};

//=========================================
//         Traversal Methods

Edge * Point::getRoot() {
	return root;
}
Edge const * Point::getRoot() const {
	return root;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//	Edge

Edge::Edge(DCEL * uni) {
	universe = uni;

	mark = 0;
}
Edge::~Edge() {

}

//=========================================
//         Traversal Methods

Point * Edge::getStart() {
	return root;
}
Point * Edge::getEnd() {
	return inv->root;
}
Edge * Edge::getNext() {
	return next;
}
Edge * Edge::getLast() {
	return last;
}
Edge * Edge::getInv() {
	return inv;
}
Face * Edge::getFace() {
	return loop;
}
Edge * Edge::getCW() {
	return last->inv;
}
Edge * Edge::getCCW() {
	return inv->next;
}
Point const * Edge::getStart() const {
	return root;
}
Point const * Edge::getEnd() const {
	return inv->root;
}
Edge const * Edge::getNext() const {
	return next;
}
Edge const * Edge::getLast() const {
	return last;
}
Edge const * Edge::getInv() const {
	return inv;
}
Face const * Edge::getFace() const {
	return loop;
}
Edge const * Edge::getCW() const {
	return inv->next;
}
Edge const * Edge::getCCW() const {
	return last->inv;
}

//=========================================
//         Modification

void Edge::subdivide(Pgrd mid_point) {
	Edge* adjoint = universe->createEdge();
	Point* mid = universe->createPoint();

	//we need to connect mid - root, position

	//20 variables internal to our 4 edges
	//4 of these are inverses, can be ignored
	//5: root, last, inv->next, loop, inv->loop don't change
	//- adjoint->(next, last, loop, root)
	//- adoint->inv->(next, last, loop, root)
	//- next
	//- inv->(last, root)

	//11 internal changes

	//6 possible external references
	//root->root doesnt change
	//last->next doesnt change
	//inv->next->last doesnt change
	//- inv->root->root
	//- last->next
	//- inv->next->last

	//3 external changes

	mid->position = mid_point;
	mid->root = adjoint;

	if (next != inv) {
		inv->root->root = next;

		adjoint->next = next;
		adjoint->last = this;
		adjoint->loop = loop;
		adjoint->root = mid;

		adjoint->inv->next = inv;
		adjoint->inv->last = inv->last;
		adjoint->inv->loop = inv->loop;
		adjoint->inv->root = inv->root;

		next->last = adjoint;
		inv->last->next = adjoint->inv;

		next = adjoint;
		inv->last = adjoint->inv;
		inv->root = mid;
	}
	else {
		inv->root->root = adjoint->inv;

		adjoint->next = adjoint->inv;
		adjoint->last = this;
		adjoint->loop = loop;
		adjoint->root = mid;

		adjoint->inv->next = inv;
		adjoint->inv->last = adjoint;
		adjoint->inv->loop = loop;
		adjoint->inv->root = inv->root;

		next = adjoint;
		inv->last = adjoint->inv;
		inv->root = mid;
	}
}
EdgeModResult Edge::moveRoot(Pgrd p) {
	Point* og = root;
	if (last == inv) {
		//root is isolated

		root->setPosition(p);
		return EdgeModResult(EdgeModType::faces_preserved, nullptr);
	}
	else {
		Point* end = universe->createPoint();
		Edge* old = inv->next;
		end->root = this;
		end->position = p;

		root->root = old;

		last->next = old;
		old->last = last;

		last = inv;
		inv->next = this;
		root = end;

		//reface this and old
		if (loop == inv->loop) {
			//we have disconnected a loop
			loop->root = this;

			Face* novel = universe->createFace();
			novel->root = old;
			novel->reFace();

			return EdgeModResult(EdgeModType::face_created, novel);
		}
		else {
			//we have merged two loops
			universe->removeFace(inv->loop);
			loop->reFace();

			return EdgeModResult(EdgeModType::face_destroyed, inv->loop);
		}
	}
}
EdgeModResult Edge::insertAfter(Edge* target) {
	//remove from og

	Face* novel_a = nullptr;
	Face* novel_b = nullptr;

	if (last == inv) {
		universe->removePoint(root);
	}
	else {
		Edge* old = inv->next;

		root->root = old;

		last->next = old;
		old->last = last;

		//reface this and old
		if (loop == inv->loop) {
			//we have disconnected a loop
			loop->root = this;

			novel_a = universe->createFace();
			novel_a->root = old;
			novel_a->reFace();
		}
		else {
			//we have merged two loops
			universe->removeFace(inv->loop);
			loop->reFace();
		}
	}

	//insert elsewhere
	root = target->inv->root;

	inv->next = target->next;
	target->next->last = inv;

	target->next = this;
	last = target;

	if (target->loop == loop) {
		//we have split a loop
		loop->root = this;

		novel_b = universe->createFace();
		novel_b->root = inv;
		novel_b->reFace();

		return EdgeModResult(EdgeModType::face_created, novel_b);
	}
	else {
		//we have joined two loops

		//is it the loop we just disconnected?
		if (target->loop == novel_a) {
			novel_a = nullptr;
		}

		universe->removeFace(target->loop);
		loop->reFace();

		return EdgeModResult(EdgeModType::face_destroyed, target->loop);
	}
}

//=========================================
//         Removal

EdgeModResult Edge::remove() {

	//if either point is isolated, we dont need to reface
	bool loose_strand = false;
	bool isolated = true;
	Face* novel = nullptr;
	EdgeModResult product(EdgeModType::faces_preserved, nullptr);

	if (next == inv) {
		loose_strand = true;

		universe->removePoint(inv->root);
	}
	else {
		isolated = false;

		inv->root->root = next;
		next->last = inv->last;
		inv->last->next = next;
		loop->root = next;
	}

	if (last == inv) {
		loose_strand = true;

		universe->removePoint(root);
	}
	else {
		isolated = false;

		root->root = inv->next;
		last->next = inv->next;
		inv->next->last = last;
		loop->root = last;
	}

	//we may be connecting or disconnecting two loops
	if (!loose_strand) {
		if (loop == inv->loop) {
			//we have disconnected a loop
			loop->root = next;
			loop->reFace();

			novel = universe->createFace();
			novel->root = last;
			novel->reFace();

			product.type = EdgeModType::face_created;
			product.relevant = novel;

		}
		else {
			//we have merged two loops
			loop->root = next;
			universe->removeFace(inv->loop);
			loop->reFace();

			product.type = EdgeModType::face_destroyed;
			product.relevant = inv->loop;
		}
	}
	else if (isolated) {
		product.type = EdgeModType::face_destroyed;
		product.relevant = loop;
	}

	universe->removeEdge(this);
	return product;
}
void Edge::contract() {

	Edge* focus = next;

	do {
		focus->root = root;
		focus = focus->inv->next;
	} while (focus != inv);

	root->root = next;

	last->next = next;
	next->last = last;

	inv->next->last = inv->last;
	inv->last->next = inv->next;

	if (loop->root == this) {
		loop->root = next;
	}
	if (inv->loop->root == inv) {
		inv->loop->root = inv->last;
	}

	universe->removePoint(inv->root);
	universe->removeEdge(this);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//	Face

Face::Face(DCEL * uni) {
	universe = uni;
	group = nullptr;

	mark = 0;
}
Face::Face(DCEL * uni, Region * grp) {
	universe = uni;

	mark = 0;
}
Face::~Face() {

}

//=========================================
//         Modifiers

void Face::reFace() {
	Edge * focus = root;
	do {
		focus->loop = this;
		focus = focus->next;
	} while (focus != root);
}

//=========================================
//         Traversal Methods

Edge * Face::getRoot() {
	return root;
}
Edge const * Face::getRoot() const {
	return root;
}
Region * Face::getGroup() {
	return group;
}
Region const * Face::getGroup() const {
	return group;
}

//=========================================
//         Queries

int Face::getLoopSize() const {
	Edge const * focus = root;
	int count = 0;

	do {
		count++;

		focus = focus->next;
	} while (focus != root);

	return count;
}
std::list<Pgrd> Face::getLoopPoints() const {
	std::list<Pgrd> target;
	Edge const * focus = root;

	do {
		Pgrd p = focus->root->getPosition();
		target.push_back(p);

		focus = focus->next;
	} while (focus != root);

	return target;
}
std::list<Edge *> Face::getLoopEdges() {
	Edge * focus = root;
	std::list<Edge *> target;

	do {
		target.push_back(focus);

		focus = focus->next;
	} while (focus != root);

	return target;
}
std::list<Edge const *> Face::getLoopEdges() const {
	Edge * focus = root;
	std::list<Edge const *> target;

	do {
		target.push_back(focus);

		focus = focus->next;
	} while (focus != root);

	return target;
}
void Face::getLoopEdges(std::list<Edge *> &target) {
	Edge * focus = root;
	do {
		target.push_back(focus);

		focus = focus->next;
	} while (focus != root);
}
void Face::getLoopEdges(std::list<Edge const *> &target) const {
	Edge * focus = root;
	do {
		target.push_back(focus);

		focus = focus->next;
	} while (focus != root);
}
std::list<Face *> Face::getNeighbors() {
	std::list<Face *> target;
	Edge * focus = root;

	do {
		Face * canidate = focus->inv->loop;
		if (!list_contains(target, canidate)) {
			target.push_back(canidate);
		}
		focus = focus->next;
	} while (focus != root);

	return target;
}
std::list<Face const *> Face::getNeighbors() const {
	std::list<Face const *> target;
	Edge const * focus = root;

	do {
		Face const * canidate = focus->inv->loop;
		if (!list_contains(target, canidate)) {
			target.push_back(canidate);
		}
		focus = focus->next;
	} while (focus != root);

	return target;
}
bool Face::neighbors(Face const * target) const {
	Edge const * focus = root;

	do {
		if (focus->inv->loop == target) {
			return true;
		}

		focus = focus->next;
	} while (focus != root);

	return false;
}
grd Face::area() const {
	grd total = 0;
	Edge * focus = root;

	do {
		Pgrd const & start = focus->getStart()->getPosition();
		Pgrd const & end = focus->getEnd()->getPosition();
		total += ((start.Y + end.Y) / 2) * (end.X - start.X);

		focus = focus->next;
	} while (focus != root);

	return total;
}
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

//=========================================
//         Regioning

std::list<Face *> Face::mergeWithFace(Face* target) {
	std::list<Edge *> markToRemove;
	std::list<Face *> product;

	Edge * focus = root;
	do {

		if (focus->inv->loop == target)
			if (!list_contains(markToRemove, focus->inv))
				markToRemove.push_back(focus);

		focus = focus->next;
	} while (focus != root);

	product.push_back(this);

	while (!markToRemove.empty()) {
		EdgeModResult result = markToRemove.front()->remove();
		markToRemove.pop_front();
		if (result.type == EdgeModType::face_created) {
			product.push_back(result.relevant);
		}
		else if (result.type == EdgeModType::face_destroyed) {
			product.remove(result.relevant);
		}
	}
	return product;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//	Region

Region::Region(DCEL * uni) {
	universe = uni;

	mark = 0;
}
Region::~Region() {

}


std::list<Face *> const & Region::getBounds() {
	return Boundaries;
}
//Face * operator[](int a) {
//	return Boundaries[a];
//}
int Region::size() const {
	return Boundaries.size();
}

grd Region::area() const {
	grd total = 0;
	for (auto f : Boundaries)
		total += f->area();
	return total;
}
void Region::add_border(Face * border) {
	if (border->group != this) {
		if (border->group != nullptr) {
			border->group->remove(border);
		}

		border->group = this;
		Boundaries.push_front(border);
	}
}
void Region::remove(Face * border) {
	if (border->group == this) {
		border->group = nullptr;
		Boundaries.remove(border);
	}
}
void Region::clear() {
	for (auto border : Boundaries) {
		border->group = nullptr;
	}

	Boundaries.clear();
}
DCEL * Region::getUni() {
	return universe;
}
std::list<Region *> Region::getNeighbors() {
	std::list<Region *> product;

	for (auto border : Boundaries) {
		auto canidates = border->getNeighbors();
		for (auto suggest : canidates) {
			if (!list_contains(product, suggest->group)) {
				product.push_front(suggest->group);
			}
		}
	}

	return product;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//	DCEL

Point * DCEL::createPoint() {
	Point * result = DBG_NEW Point(this);
	points.push_front(result);
	return result;
}
Edge * DCEL::createEdge() {
	Edge * result = DBG_NEW Edge(this);
	Edge * inverse = DBG_NEW Edge(this);

	edges.push_front(result);
	edges.push_front(inverse);

	result->inv = inverse;
	inverse->inv = result;

	return result;
}
Face * DCEL::createFace() {
	Face * result = DBG_NEW Face(this);
	faces.push_front(result);
	return result;
}

void DCEL::removePoint(Point * target) {
	points.remove(target);

	delete target;
}
void DCEL::removeEdge(Edge * target) {
	edges.remove(target);
	edges.remove(target->inv);

	delete target->inv;
	delete target;
}
void DCEL::removeFace(Face * target) {
	faces.remove(target);

	delete target;
}

DCEL::~DCEL() {
	for (auto focus_point : points) {
		delete focus_point;
	}

	for (auto focus_edge : edges) {
		delete focus_edge;
	}

	for (auto focus_face : faces) {
		delete focus_face;
	}

	for (auto focus_region : regions) {
		delete focus_region;
	}
}

int DCEL::pointCount() const {
	return points.size();
}
int DCEL::edgeCount() const {
	return edges.size();
}
int DCEL::faceCount() const {
	return faces.size();
}
int DCEL::regionCount() const {
	return regions.size();
}

Edge * DCEL::addEdge(Pgrd a, Pgrd b) {
	Edge * result = createEdge();
	Point * A = createPoint();
	Point * B = createPoint();
	Face * loop = createFace();

	result->next = result->inv;
	result->last = result->inv;
	result->inv->next = result;
	result->inv->last = result;

	result->root = A;
	A->root = result;
	A->position = a;

	result->inv->root = B;
	B->root = result->inv;
	B->position = b;

	result->loop = loop;
	result->inv->loop = loop;

	loop->root = result;

	return result;
}
Edge * DCEL::addEdge(Edge * a, Pgrd b) {
	Edge * result = createEdge();
	Point * B = createPoint();

	result->next = result->inv;
	result->inv->last = result;

	result->root = a->inv->root;

	result->inv->root = B;
	B->root = result->inv;
	B->position = b;

	a->next->last = result->inv;
	result->inv->next = a->next;

	a->next = result;
	result->last = a;

	result->loop = a->loop;
	result->inv->loop = a->loop;

	return result;
}
Edge * DCEL::addEdge(Edge * a, Edge * b) {
	Edge * result = createEdge();
	Face * novel = nullptr;

	a->next->last = result->inv;
	result->inv->next = a->next;

	a->next = result;
	result->last = a;

	b->next->last = result;
	result->next = b->next;

	b->next = result->inv;
	result->inv->last = b;

	result->root = a->inv->root;
	result->inv->root = b->inv->root;

	if (a->loop == b->loop) {
		//we have split a loop

		a->loop->root = a;
		result->loop = a->loop;

		novel = createFace();
		novel->root = b;
		novel->reFace();
	}
	else {
		//we have joined two loops
		removeFace(b->loop);
		a->loop->reFace();
	}

	return result;
}

Region * DCEL::region() {
	Region * product = DBG_NEW Region(this);
	regions.push_back(product);
	return product;
}
Region * DCEL::region(Face * face) {
	Region * product = DBG_NEW Region(this);
	product->add_border(face);
	regions.push_back(product);
	return product;
}
Region * DCEL::region(std::list<Pgrd> const &boundary) {
	Region * product = DBG_NEW Region(this);
	product->add_border(draw(boundary));
	regions.push_back(product);
	return product;
}

Face * DCEL::draw(std::list<Pgrd> const &boundary) {
	auto track = boundary.begin();

	Pgrd a = *track;
	++track;
	Pgrd b = *track;
	++track;

	Edge * start = addEdge(a, b);
	Edge * strand = start;

	while (track != boundary.end()) {
		b = *track;
		++track;

		strand = addEdge(strand, b);
	}

	addEdge(strand, start->inv);

	start->loop->root = start;

	return start->loop;
}

void DCEL::removeRegion(Region * target) {
	regions.remove(target);

	delete target;
}

void DCEL::resetPointMarks() {
	for (auto point : points)
		point->mark = 0;
}
void DCEL::resetEdgeMarks() {
	for (auto edge : edges)
		edge->mark = 0;
}
void DCEL::resetFaceMarks() {
	for (auto face : faces)
		face->mark = 0;
}
void DCEL::resetRegionMarks() {
	for (auto region : regions)
		region->mark = 0;
}
