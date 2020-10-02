#include "Grid_Region.h"
#include <cmath>
#include <queue>
#include "debugger.h"

#define debug_suballocate
//#define debug_merge
//#define debug_clean

#if defined(debug_suballocate) || defined(debug_merge) || defined(debug_clean)
//#define using_unreal
#endif

#ifdef using_unreal
//#include "CoreMinimal.h"
#endif

//returns FaceRelationType between a face and a point
FaceRelation const getPointRelation(Face & rel, Pgrd const &test_point) {

	Edge * focus = rel.getRoot();

	bool inside = Pgrd::area(rel.getLoopPoints()) < 0;

	do {
		const Pgrd &start_vector = focus->getStart()->getPosition();
		const Pgrd &end_vector = focus->getEnd()->getPosition();

		//does it sit on
		grd y_length = end_vector.Y - start_vector.Y;
		grd y_offset = test_point.Y - start_vector.Y;

		if (y_length == 0) {
			if (y_offset == 0) {
				if ((start_vector.X <= test_point.X && test_point.X <= end_vector.X) ||
					(start_vector.X >= test_point.X && test_point.X >= end_vector.X)) {

					return FaceRelation(FaceRelationType::point_on_boundary, focus);
				}
			}
			focus = focus->getNext();
			continue;
		}
		else {
			grd ratio = y_offset / y_length;
			if (ratio <= 1 && ratio >= 0) {
				grd x_length = end_vector.X - start_vector.X;

				grd x = start_vector.X + x_length * ratio;
				grd distance = test_point.X - x;

				if (distance == 0) {
					return FaceRelation(FaceRelationType::point_on_boundary, focus);
				}
				else if (distance > 0) {
					if (ratio == 1) {
						if (y_offset > 0)
							inside = !inside;
					}
					else if (ratio == 0) {
						if (y_length < 0)
							inside = !inside;
					}
					else {
						inside = !inside;
					}
				}
			}
		}
		focus = focus->getNext();
	} while (focus != rel.getRoot());

	if (inside) {
		return FaceRelation(FaceRelationType::point_interior, nullptr);
	}
	else {
		return FaceRelation(FaceRelationType::point_exterior, nullptr);
	}
}

//returns FaceRelationType between a face (as a list) and a point
FaceRelationType const getPointRelation(std::list<Pgrd> const & rel, Pgrd const &test_point) {

	bool inside = Pgrd::area(rel) < 0;

	for (auto start = rel.begin(); start != rel.end(); ++start) {
		Pgrd start_vector = *start;
		Pgrd end_vector = *cyclic_next(start, rel);


		//does it sit on
		grd y_length = end_vector.Y - start_vector.Y;
		grd y_offset = test_point.Y - start_vector.Y;

		if (y_length == 0) {
			if (y_offset == 0) {
				if ((start_vector.X <= test_point.X && test_point.X <= end_vector.X) ||
					(start_vector.X >= test_point.X && test_point.X >= end_vector.X)) {

					return FaceRelationType::point_on_boundary;
				}
			}
			continue;
		}
		else {
			grd ratio = y_offset / y_length;
			if (ratio <= 1 && ratio >= 0) {
				grd x_length = end_vector.X - start_vector.X;

				grd x = start_vector.X + x_length * ratio;
				grd distance = test_point.X - x;

				if (distance == 0) {
					return FaceRelationType::point_on_boundary;
				}
				else if (distance > 0) {
					if (ratio == 1) {
						if (y_offset > 0)
							inside = !inside;
					}
					else if (ratio == 0) {
						if (y_length < 0)
							inside = !inside;
					}
					else {
						inside = !inside;
					}
				}
			}
		}
	}

	if (inside) {
		return FaceRelationType::point_interior;
	}
	else {
		return FaceRelationType::point_exterior;
	}
}

//returns true if "for all faces of target, test_point is strictly interior"
FaceRelation contains(Region * target, Pgrd const & test_point) {
	for (auto focus : target->getBounds()) {

		FaceRelation result = getPointRelation(*focus, test_point);
		if (result.type != FaceRelationType::point_interior) {
			return result;
		}
	}

	return FaceRelation(FaceRelationType::point_interior, nullptr);
}

void cleanFace(Face * target) {
	auto edge = target->getRoot();
	auto next = edge->getNext();
	auto B = next->getEnd()->getPosition() - next->getStart()->getPosition();
	auto A = edge->getEnd()->getPosition() - edge->getStart()->getPosition();
	A.Normalize();
	B.Normalize();

	do {
		if(edge->getInv()->getLast()->getInv() == next)
			if (A.Dot(B) == 1)
				edge->contract();

		edge = next;
		next = edge->getNext();

		A = B;
		B = next->getEnd()->getPosition() - next->getStart()->getPosition();
		B.Normalize();
	} while (edge != target->getRoot());
}

//TODO: self cleaning merge
bool merge(Region * a, Region * b) {
	//since both areas are continuous, its trivial that only one or no boundary pair can touch

	//regions are either strictly internal, strictly external, or weakly external to boundaries

	//if a boundary contains any part of a region, it can't touch that region (


	if (a != b) {


		Face * local_face = nullptr;
		Face * target_face = nullptr;
		for (auto focus_local : a->getBounds()) {

			auto neighbors = focus_local->getNeighbors();

			for (auto focus_target : b->getBounds()) {

				if (list_contains(neighbors, focus_target)) {
					local_face = focus_local;
					target_face = focus_target;
					break;
				}
			}

			if (local_face != nullptr) {
				break;
			}
		}

		if (local_face == nullptr) return false;

#ifdef debug_merge
		debug_proxy(("merging"));
		debug_proxy(("a"));
		for (auto bound : a->getBounds()) {
			debug_proxy(("face >k-"));
			for (auto point : bound->getLoopPoints()) {
				debug_proxy(("(%f,%f)"), point.X.n, point.Y.n);
			}
		}
		debug_proxy(("b"));
		for (auto bound : b->getBounds()) {
			debug_proxy(("face >b-"));
			for (auto point : bound->getLoopPoints()) {
				debug_proxy(("(%f,%f)"), point.X.n, point.Y.n);
			}
		}
#endif

		//we have found a boundary pair that neighbors one another, merge them

		a->remove(local_face);
		b->remove(target_face);

		auto tba = local_face->mergeWithFace(target_face);

		auto t(b->getBounds());
		for (auto face : t)
			a->add_border(face);

		for (auto face : tba) {
			cleanFace(face);
			a->add_border(face);
		}

#ifdef debug_merge
		debug_proxy(("result"));
		for (auto bound : a->getBounds()) {
			debug_proxy(("face >r:"));
			for (auto point : bound->getLoopPoints()) {
				debug_proxy(("(%f,%f)"), point.X.n, point.Y.n);
			}
		}
#endif
		b->getUni()->removeRegion(b);
		return true;
	}
	else {
#ifdef debug_merge
		debug_proxy(("merging"));
		for (auto bound : a->getBounds()) {
			debug_proxy(("face >k:"));
			for (auto point : bound->getLoopPoints()) {
				debug_proxy(("(%f,%f)"), point.X.n, point.Y.n);
			}
		}
#endif

		auto bounds = a->getBounds();
		for (auto focus_local = bounds.begin(); focus_local != bounds.end();) {

			for (auto focus_compare = focus_local; focus_compare != bounds.end();) {

				if ((*focus_local)->neighbors(*focus_compare)) {
					auto tba = (*focus_local)->mergeWithFace(*focus_compare);

					for (auto face : tba)
						if (face != *focus_local)
							a->add_border(face);

					focus_compare = focus_local;
				}

				++focus_compare;
			}
			++focus_local;
		}
#ifdef debug_merge
		debug_proxy(("result"));
		for (auto bound : a->getBounds()) {
			debug_proxy(("face >r:"));
			for (auto point : bound->getLoopPoints()) {
				debug_proxy(("(%f,%f)"), point.X.n, point.Y.n);
			}
		}
#endif

		return false;
	}

}

struct intersect {

	Pgrd location;
	Edge* mark;
	grd distance;
};

struct intersectSort{
	bool operator()(intersect const *a, intersect const *b) const {
		return a->distance > b->distance;
	}
};

typedef std::priority_queue<intersect *, std::vector<intersect *>, intersectSort> sorted_intersects;

//returns a list of intersects sorted by distance
sorted_intersects findIntersects(Pgrd const & start, Pgrd const & stop,
	std::list<Edge *> const & canidates) {

	//detect intersect
	sorted_intersects product;

	for (auto target : canidates) {

		Pgrd intersect_location;

		Pgrd test_start = target->getStart()->getPosition();
		Pgrd test_stop = target->getEnd()->getPosition();

		bool valid = Pgrd::getIntersect(start, stop, test_start, test_stop, intersect_location);

		if (valid) {
			intersect * output = DBG_NEW intersect{ intersect_location, target, 
        (intersect_location - start).SizeSquared() };

			product.push(output);
		}
		else {
			//parrallel test
			Pgrd a = stop - start;
			Pgrd b = test_stop - test_start;

			grd x, y;
			if (a.Y != 0 && b.Y != 0) {
				x = a.X / a.Y;
				y = b.X / b.Y;
			}
			else if (a.Y == 0 && b.Y == 0) {
				x = a.X / a.X;
				y = b.X / b.X;
			}

			if (x == y || x == -y) {

				//create an interesect for the ends of each segment, that lie on the other segment
				if (Pgrd::isOnSegment(start, test_start, test_stop)) {
					intersect * output = DBG_NEW intersect();

					output->location = start;
					output->mark = target;
					output->distance = 0;

					product.push(output);
				}

				if (Pgrd::isOnSegment(stop, test_start, test_stop)) {
					intersect * output = DBG_NEW intersect();

					output->location = stop;
					output->mark = target;
					output->distance = (stop - start).SizeSquared();

					product.push(output);
				}

				if (Pgrd::isOnSegment(test_start, start, stop) && test_start != start && test_start != stop) {
					intersect * output = DBG_NEW intersect();

					output->location = test_start;
					output->mark = target;
					output->distance = (test_start - start).SizeSquared();

					product.push(output);
				}

				if (Pgrd::isOnSegment(test_stop, start, stop) && test_stop != start && test_stop != stop) {
					intersect * output = DBG_NEW intersect();

					output->location = test_stop;
					output->mark = target;
					output->distance = (test_stop - start).SizeSquared();

					product.push(output);
				}
			}
		}
	}


	return product;
}

//finds interact features for a suballocation, and subidivides region edges where needed
//returns true if boundary is entirely strictly external
bool markRegion(Region * target, std::list<Pgrd> const & boundary, std::list<interact *>  & details) {

	bool exterior = true;

  std::list<Edge *> canidates;

  for (auto canidate_focus : target->getBounds()) {
    auto tba = canidate_focus->getLoopEdges(); //TODO: rvalues
    canidates.splice(canidates.end(), tba);
  }

	{
		

		auto last = boundary.back();
		for (auto next : boundary) {
			//find and perform on all intersects

			auto intersects = findIntersects(last, next, canidates);

			bool end_collision = false;

			while(!intersects.empty()) {
				auto * intersect_focus = intersects.top();
        intersects.pop();

				auto mark = intersect_focus->mark;

				//ignore hits at the start of either segment
				if (intersect_focus->location != last && intersect_focus->location != mark->getStart()->getPosition()) {


					interact* feature = DBG_NEW interact();

					feature->location = intersect_focus->location;
					feature->type = FaceRelationType::point_on_boundary;

					if (intersect_focus->location == mark->getEnd()->getPosition()) {
						//no need to subdivide

						feature->mark = mark;
					}
					else {
						//subdivide mark

						mark->getInv()->subdivide(intersect_focus->location);

						feature->mark = mark->getLast();

						canidates.push_front(feature->mark);
					}

					if (intersect_focus->location == next) {
						//prevents duplicate features for ends of segments
						end_collision = true;
					}

					exterior = false;
					details.push_back(feature);
				}

				delete intersect_focus;
			}

      auto last_state = contains(target, last);
      auto state = contains(target, next);
      if(details.empty()){
        if ((last_state.type == FaceRelationType::point_exterior && state.type == FaceRelationType::point_interior)
          || (last_state.type == FaceRelationType::point_interior && state.type == FaceRelationType::point_exterior)) {

          auto debug_intersects = findIntersects(last, next, canidates);

          throw std::logic_error("no intersects found, despite change in face relation");
        }
      }
      else {
        if ((details.back()->type == FaceRelationType::point_exterior && state.type == FaceRelationType::point_interior)
          || (details.back()->type == FaceRelationType::point_interior && state.type == FaceRelationType::point_exterior)) {

          auto debug_intersects = findIntersects(last, next, canidates);

          throw std::logic_error("no intersects found, despite change in face relation");
        }
      }

			if (!end_collision) {

				interact* feature = DBG_NEW interact();

				feature->location = next;
				feature->type = state.type;
				feature->mark = state.relevant;

				exterior = exterior && (state.type == FaceRelationType::point_exterior);
				details.push_back(feature);
			}

			last = next;
		}
  }


	//calculate mid inclusion
  {
		auto last = details.back();
		for (auto next : details) {
			last->mid_location = (last->location + next->location) / 2;

			auto result = contains(target, last->mid_location);

			last->mid_type = result.type;

#if 1 // do inclusion checks
      if ((last->type == FaceRelationType::point_exterior && next->type == FaceRelationType::point_interior)
      || (last->type == FaceRelationType::point_interior && next->type == FaceRelationType::point_exterior)) {

        auto debug_intersects = findIntersects(last->location, next->location, canidates);

        throw std::logic_error("Missing PoB transition, last to next");
      }
  
      if ((last->type == FaceRelationType::point_exterior && last->mid_type == FaceRelationType::point_interior)
        || (last->type == FaceRelationType::point_interior && last->mid_type == FaceRelationType::point_exterior)) {

        auto debug_intersects = findIntersects(last->location, next->location, canidates);

        throw std::logic_error("Missing PoB transition, last to mid");
      }

      if ((last->mid_type == FaceRelationType::point_exterior && next->type == FaceRelationType::point_interior)
        || (last->mid_type == FaceRelationType::point_interior && next->type == FaceRelationType::point_exterior)) {

        auto debug_intersects = findIntersects(last->location, next->location, canidates);

        throw std::logic_error("Missing PoB transition, mid to next");
      }

      if (last->mid_type == FaceRelationType::point_on_boundary && (last->type != FaceRelationType::point_on_boundary || next->type != FaceRelationType::point_on_boundary)) {
        
        auto debug_intersects = findIntersects(last->location, next->location, canidates);

        debug_proxy("detail:");
        debug_proxy("", last->location.X.n, last->location.Y.n);
        debug_proxy("", last->mid_location.X.n, last->mid_location.Y.n);
        debug_proxy("", next->location.X.n, next->location.Y.n);
        debug_proxy("intersects:");
        while(!debug_intersects.empty()){
          auto * intersect = debug_intersects.top();
          debug_intersects.pop();
          debug_proxy("", intersect->location.X.n, intersect->location.Y.n);
          delete intersect;
         }

        throw std::logic_error("mid cannot be on boundary for a PoB transition edge");
      }
#endif

			last = next;
		}
	}

	return exterior;
}

//returns if test is between A and B clockwise (right about the origin from A, left about from B)
bool angledBetween(Pgrd const &A, Pgrd const &B, Pgrd const &test) {

	Pgrd A_inward(A.Y, -A.X);
	Pgrd B_inward(-B.Y, B.X);

	grd bounds_relation = A_inward.Dot(B);

	if (bounds_relation > 0) {
		//the angle between bounds is in (0,180)
		return A_inward.Dot(test) >= 0 && B_inward.Dot(test) >= 0;
	}
	else if (bounds_relation == 0) {
		//the angle between bounds is 180 or 0, or one bound is length 0
		//any case other than 180 is due to an error as used for determine interiors

		return A_inward.Dot(test) >= 0;
	}
	else {
		//the angle between bounds is in (180,360)
		return A_inward.Dot(test) >= 0 || B_inward.Dot(test) >= 0;
	}
}

//insert strands into target, and determine face inclusions
void determineInteriors(Region * target, std::list<Pgrd> const &root_list, std::list<interact *> & details,
	std::list<Face *> & exteriors, std::list<Face *> & interiors) {

	exteriors = target->getBounds();

	auto last = details.begin();
	auto next = cyclic_next(last, details);


	//consider first segment, if entirely internal, we need to create an edge from scratch
	if ((*last)->type == FaceRelationType::point_interior) {

		auto into = *next;
		auto from = *last;

		if (into->type == FaceRelationType::point_interior) {

			into->mark = target->getUni()->addEdge(from->location, into->location);

			from->mark = into->mark->getInv();
		}
		else if (into->type == FaceRelationType::point_on_boundary) {

			from->mark = target->getUni()->addEdge(into->mark, from->location);

			into->mark = from->mark->getInv();
		}

		from->type = FaceRelationType::point_on_boundary;

		++last;
	}

	while (last != details.end()) {
		next = cyclic_next(last, details);

		auto into = *next;
		auto from = *last;

		if (from->type != FaceRelationType::point_exterior) {
			if (into->type == FaceRelationType::point_interior) {
        if(from->mark == nullptr) throw;

				into->mark = target->getUni()->addEdge(from->mark, into->location);

			}
			else if (into->type == FaceRelationType::point_on_boundary) {
				if (from->mid_type == FaceRelationType::point_interior) {
					//if (from->mark->getNext() != into->mark) {
					if (into->mark->getFace() == from->mark->getFace()) {

						exteriors.remove(into->mark->getFace());

						auto created = target->getUni()->addEdge(from->mark, into->mark);

						//dot(mid-created_end, (next_end-created_end).cw(90) ) > 0 and dot(mid-created_end, (created_start-created_end).ccw(90) )
						Pgrd next_vector = created->getNext()->getEnd()->getPosition() - into->location;
						Pgrd created_vector = created->getStart()->getPosition() - into->location;
						Pgrd orientation = into->mid_location - into->location;

						if (angledBetween(next_vector, created_vector, orientation))
							into->mark = created;

						if (!list_contains(interiors,created->getFace()))
							interiors.push_front(created->getFace());

						exteriors.push_front(created->getInv()->getFace());
					}
					else if (next == details.begin()) {
						auto relevant = into->mark->getNext()->getInv();
						exteriors.remove(relevant->getFace());

						auto created = target->getUni()->addEdge(from->mark, relevant);

						if (!list_contains(interiors, created->getFace()))
							interiors.push_front(created->getFace());

						exteriors.push_front(created->getInv()->getFace());
					}
					else {
						exteriors.remove(into->mark->getFace());

						into->mark = target->getUni()->addEdge(from->mark, into->mark);
					}
					//}
				}
			}
      else if(from->type == FaceRelationType::point_interior && into->type == FaceRelationType::point_exterior){
        throw;
      }
		}
    else if(into->type == FaceRelationType::point_interior) {
      throw;
    }

		++last;
	}

	//does the entire boundary lie on a loop

	//get face interacted with (there can only be one due to continuity of regions constraint)
	if (interiors.empty()) {
		Face * interacted = nullptr;

		for (auto detail : details)
			if (detail->type == FaceRelationType::point_on_boundary) {
				interacted = detail->mark->getFace();
				break;
			}
		if (interacted == nullptr) {
			//should never happen
			throw std::logic_error("No Boundary alligned detail found, despite earlier edge case");
		}

		bool all_alligned = true;
		for (auto point : interacted->getLoopPoints())
			if (getPointRelation(root_list, point) != FaceRelationType::point_on_boundary) {
				all_alligned = false;
				break;
			}

		//debug_proxy("A", interacted->area().n);
		//debug_proxy("A*", Pgrd::area(interacted->getLoopPoints()).n);
		//debug_proxy("B", Pgrd::area(root_list).n);



		bool root_wrap = Pgrd::area(root_list) > 0;
		bool target_wrap = interacted->area() > 0;

		if ((all_alligned && (root_wrap == target_wrap)) || (!all_alligned && !root_wrap)) {
			//debug_proxy("CONSUME");
			interiors.splice(interiors.end(), exteriors);
		}
	}

	/*if (interiors.empty()) {
		auto from = *details.begin();


		if (from->type == FaceRelationType::point_on_boundary) {
			auto face = from->mark->getFace();
			debug_proxy("CONSUME");

			exteriors.remove(face);

			interiors.push_front(face);
		}
	}*/
}

void subAllocate(Region * target, std::list<Pgrd> const & boundary,
	std::list<Region *> & exteriors, std::list<Region *> & interiors) {

#ifdef debug_suballocate
	debug_proxy("SA");
	debug_proxy("Boundary >g:");
	for (auto point : boundary) {
		debug_proxy("", point.X.n, point.Y.n);
	}

	for (auto face : target->getBounds()) {
		debug_proxy("Face >k-");
		for (auto point : face->getLoopPoints()) {
			debug_proxy("",point.X.n, point.Y.n);
		}
	}
#endif
	//subdivide all edges based on intersects
	//this means all boundary edges are either
	//exterior
	//on point (with previous edge noted)
	//interior
	//subdivides are performed on inverse to preserve marks

	std::list<interact *> details;

	bool exterior = markRegion(target, boundary, details);

	std::list<Face *> exterior_faces;
	std::list<Face *> interior_faces;

#ifdef debug_suballocate
	for (auto detail : details) {
		if (detail->type == FaceRelationType::point_exterior) {
			debug_proxy("exterior ", detail->location.X.n, detail->location.Y.n);
		}
		else if (detail->type == FaceRelationType::point_interior) {
			debug_proxy("interior ", detail->location.X.n, detail->location.Y.n);
		}
		else {
			debug_proxy("bound ", detail->location.X.n, detail->location.Y.n);
		}

	}
#endif

	if (exterior) {
		//bool interior = true;
		//for (auto face : target->getBounds()) {
		//	for (auto point : face->getLoopPoints()) {
		//		interior = (getPointRelation(boundary, point) != FaceRelationType::point_exterior);
		//		if (!interior) break;
		//	}
		//	if (!interior) break;
		//}
		//if (interior) {

		auto test = (*target->getBounds().begin())->getRoot()->getStart()->getPosition();
		if(getPointRelation(boundary, test) == FaceRelationType::point_interior) {
			interiors.push_front(target);
#ifdef debug_suballocate
			debug_proxy("EXTERIOR EDGE CASE: contains target");
#endif
		}
		else {
			exteriors.push_front(target);
#ifdef debug_suballocate
			debug_proxy("EXTERIOR EDGE CASE: no overlap");
#endif
		}
	}
	else {
		determineInteriors(target, boundary, details, exterior_faces, interior_faces);

		//find regions, place holes

		//determine clockwise faces
		//determine clockwise containment tree
		//insert counterclockwise containment at deepest symmetric level

		//determine interior face + exterior face sets with universal containment, create regions out of these
		//determine exterior face sets with universal containment, create regions out of these\

		//for each interior face, create a region, add any symmetricly contained exterior faces to that region
		for (auto interior_face : interior_faces) {
			Region * novel = target->getUni()->region();

			auto interior_root = interior_face->getRoot()->getStart()->getPosition();

			getPointRelation(*interior_face, interior_root);

			for (auto exterior_focus = exterior_faces.begin(); exterior_focus != exterior_faces.end();) {

				auto exterior_face = *exterior_focus;
				auto exterior_root = exterior_face->getRoot()->getStart()->getPosition();

				auto ex_contains_in = getPointRelation(*exterior_face, interior_root);
				auto in_contains_ex = getPointRelation(*interior_face, exterior_root);

				++exterior_focus;

				if (ex_contains_in.type == FaceRelationType::point_interior && in_contains_ex.type == FaceRelationType::point_interior) {
					novel->add_border(exterior_face);

					exterior_faces.remove(exterior_face);
				}
			}

			novel->add_border(interior_face);

			interiors.push_front(novel);
#ifdef debug_suballocate
			debug_proxy("added Interior Face");
#endif
		}

		//for each exterior face, see which faces are symmetric with it and create regions

		for (auto exterior_focus = exterior_faces.begin(); exterior_focus != exterior_faces.end(); ++exterior_focus) {
			Region * novel = target->getUni()->region();

			auto base_face = *exterior_focus;
			auto base_root = base_face->getRoot()->getStart()->getPosition();

			for (auto compare = std::next(exterior_focus); compare != exterior_faces.end();) {

				auto comp_face = *compare;
				auto comp_root = comp_face->getRoot()->getStart()->getPosition();

				auto comp_contains_base = getPointRelation(*comp_face, base_root);
				auto base_contains_comp = getPointRelation(*base_face, comp_root);

				++compare;

				if (comp_contains_base.type == FaceRelationType::point_interior && base_contains_comp.type == FaceRelationType::point_interior) {
					novel->add_border(comp_face);

					exterior_faces.remove(comp_face);
				}
			}

			novel->add_border(base_face);

			exteriors.push_front(novel);
#ifdef debug_suballocate
			debug_proxy("added Exterior Face");
#endif
		}

		target->getUni()->removeRegion(target);
	}

	for (auto detail : details)
		delete detail;
}

void cleanRegion(Region * target) {

#ifdef debug_clean
	debug_proxy(("Clean Region"));
#endif
	for (auto border : target->getBounds()) {
		auto og_root = border->getRoot();
		auto focus = og_root;

#ifdef debug_clean
		debug_proxy(("Face >k:"));
		for (auto point : border->getLoopPoints()) {
			debug_proxy(("(%f,%f)"), point.X.n, point.Y.n);
		}
#endif

		while (true) {
			auto next = focus->getNext();

			// mid point degree is two test
			if (focus->getInv()->getLast() == next->getInv()) {

				bool parallel = false;

				// parallel test

				Pgrd const start = focus->getStart()->getPosition();
				Pgrd const mid = focus->getEnd()->getPosition();
				Pgrd const end = next->getEnd()->getPosition();

				Pgrd const a = mid - start;
				Pgrd const b = end - mid;

				if (a.Y != 0 && b.Y != 0) {
					grd x = a.X / a.Y;
					grd y = b.X / b.Y;
					parallel = x == y;
				}
				else if (a.Y == 0 && b.Y == 0) {
					parallel = true;
				}

				if (parallel) {
#ifdef debug_clean
					debug_proxy(("contract >r:"));
					debug_proxy(("(%f,%f)"), start.X.n, start.Y.n);
					debug_proxy(("(%f,%f)"), mid.X.n, mid.Y.n);
					debug_proxy(("contract >g:"));
					debug_proxy(("(%f,%f)"), mid.X.n, mid.Y.n);
					debug_proxy(("(%f,%f)"), end.X.n, end.Y.n);
#endif
					next->getInv()->contract();
				}
			}

			if (next == og_root) {
				break;
			}

			focus = focus->getNext();
		}
	}
}

Region * RegionAdd(Region * target, Edge * A, Edge * B) {
	auto A_face = A->getFace();
	auto B_face = B->getFace();


	Region * result = nullptr;

	if (A_face->getGroup() != target || B_face->getGroup() != target)
		return nullptr;

	if (B_face == A_face) {
		//we will be splitting the region

		target->getUni()->addEdge(A, B);

		B_face = B->getFace();

		result = target->getUni()->region();

		result->add_border(B_face);

		std::list<Face *> transfers;

		for (auto edge : target->getBounds())
			if (edge == A_face)
				continue;
			else if (getPointRelation(*edge, B_face->getRoot()->getStart()->getPosition()).type == FaceRelationType::point_interior
				&& getPointRelation(*B_face, edge->getRoot()->getStart()->getPosition()).type == FaceRelationType::point_interior)
				transfers.push_back(edge);

		for (auto edge : transfers)
			result->add_border(edge);
	}
	else {
		//we will be connecting two boundaries
		target->remove(B_face);

		target->getUni()->addEdge(A, B);
	}

	return result;
}
