#include "Room_Boundary.h"

Room_Boundary::Room_Boundary(Face<Pgrd> * reference)
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

		Points.append(A);
		Offsets.append(inset);
		walled = true;
	}
}

//only detects local intersections
FLL<Pgrd> Room_Boundary::Inset(grd const distance) const
{
	//account for local intersections

	//project insets from roots
	//if you find an intersection
		//reverse project a new root and project to the new inset
		//replace the intersecting roots with the novel
		//replace the intersecting inset with the novel
		//rescan for intersections

	FLL<Pgrd> roots(Points);
	FLL<Pgrd> insets;
	FLL<Pgrd>::FLL_iterator_c point = roots.begin();
	for (auto source : Offsets) {
		insets.append(source * distance + *point);
		++point;
	}

	FLL<Pgrd>::FLL_iterator A_roots = roots.begin_unsafe();
	FLL<Pgrd>::FLL_iterator A_insets = insets.begin_unsafe();

	do {
		auto B_roots = A_roots.cyclic_next();
		auto C_roots = B_roots.cyclic_next();
		auto D_roots = C_roots.cyclic_next();

		auto B_insets = A_insets.cyclic_next();
		auto C_insets = B_insets.cyclic_next();
		auto D_insets = C_insets.cyclic_next();

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

			A_roots.remove_next();
			A_roots.remove_next();

			A_roots.insert_after(root);

			A_insets.remove_next();
			A_insets.remove_next();

			A_insets.insert_after(inset);
		}
		else {
			++A_roots;
			++A_insets;
		}
	} while (A_roots != roots.end_unsafe() && roots.size() > 3);

	return insets;
}