#include "Room_Boundary.h"

Room_Boundary::Room_Boundary(Face * reference)
{
	for (auto edge : reference->getLoopEdges()) {
		Pgrd const previous = edge->getLast()->getStart()->getPosition();
		Pgrd const A = edge->getStart()->getPosition();
		Pgrd const B = edge->getEnd()->getPosition();

		Pgrd A_last = previous - A;
		Pgrd A_next = B - A;

		A_last.Normalize();
		A_next.Normalize();

		Pgrd inset = A_last + A_next;

		if (inset == Pgrd(0, 0)) {
			inset.X = A_next.Y;
			inset.Y = -A_next.X;
		}
		else {
			inset.Normalize();
			Pgrd rot(A_next.Y, -A_next.X);

			inset /= inset.Dot(rot);
		}

		inset;

		Points.push_back(A);
		Offsets.push_back(inset);
		walled = true;
	}
}

//only detects local intersections
std::list<Pgrd> Room_Boundary::Inset(grd const distance) const
{
	//account for local intersections

	//project insets from roots
	//if you find an intersection
		//reverse project a new root and project to the new inset
		//replace the intersecting roots with the novel
		//replace the intersecting inset with the novel
		//rescan for intersections

	std::list<Pgrd> roots(Points);
	std::list<Pgrd> insets;
	auto point = roots.begin();
	for (auto source : Offsets) {
		insets.push_back(source * distance + *point);
		++point;
	}

	auto A_roots = roots.begin();
	auto A_insets = insets.begin();

	do {
		auto B_roots = cyclic_next(A_roots, roots);
		auto C_roots = cyclic_next(B_roots, roots);
		auto D_roots = cyclic_next(C_roots, roots);

		auto B_insets = cyclic_next(A_insets, insets);
		auto C_insets = cyclic_next(B_insets, insets);
		auto D_insets = cyclic_next(C_insets, insets);

		Pgrd Br = *B_roots;
		Pgrd Bi = *B_insets;

		Pgrd Cr = *C_roots;
		Pgrd Ci = *C_insets;

		Pgrd I;

		if (Pgrd::getIntersect(Br, Bi, Cr, Ci, I)) {

			Pgrd Ar = *A_roots;
			Pgrd Dr = *D_roots;

			grd r = (Br - I).Size() / (Br - Bi).Size();
			Pgrd before = Ar - Br;
			Pgrd after = Dr - Cr;
			before.Normalize();
			after.Normalize();

			Pgrd dir = before + after;

			if (dir == Pgrd(0, 0)) {
				dir.X = after.Y;
				dir.Y = -after.X;
			}
			else {
				dir.Normalize();
				Pgrd rot(after.Y, -after.X);

				dir /= dir.Dot(rot);
			}

			Pgrd root = I - (dir * r * distance);
			Pgrd inset = I + (dir * (grd(1) - r) * distance);

			roots.erase(cyclic_next(A_roots, roots));
			roots.erase(cyclic_next(A_roots, roots));

			roots.insert(cyclic_next(A_roots, roots), root);

			insets.erase(cyclic_next(A_insets, insets));
			insets.erase(cyclic_next(A_insets, insets));

			insets.insert(cyclic_next(A_insets, insets), inset);
		}
		else {
			++A_roots;
			++A_insets;
		}
	} while (A_roots != roots.end() && roots.size() > 3);

	return insets;
}