#pragma once
#include "DCEL/Grid_Point.h"
#include <stack>
#include <vector>

// allows for fast lookup of nearby positional elements <t>
// IS NOT responsible for data lifetime
template<typename t>
class KD_TREE {
public:
	class KD_Branch {
		friend KD_TREE;
		bool x_oriented;
		Pgrd position;
		t data;
		KD_Branch* left = nullptr;
		KD_Branch* right = nullptr;
    
    KD_Branch(t _data)
    : data(_data) {}

	public:
		t Data() const { return data; }
		Pgrd Position() const { return position; }

		~KD_Branch() {
			delete left;
			delete right;
		}
	};
private:
	KD_Branch* root = nullptr;

public:
	~KD_TREE() {
		if (root) delete root;
	}

	void AddPoint(const Pgrd& insert, t _data) {
		KD_Branch* novel_branch = new KD_Branch(_data);
		novel_branch->position = insert;

		//special root case
		if (root == nullptr) {
			novel_branch->x_oriented = true;
			root = novel_branch;
			return;
		}

		KD_Branch * focus = root;
		while (true) {
			if (focus->x_oriented) {
				if (novel_branch->position.X <= focus->position.X) {
					if (focus->left == nullptr) {
						novel_branch->x_oriented = false;
						focus->left = novel_branch;
						break;
					}
					else {
						focus = focus->left;
						continue;
					}
				}
				else {
					if (focus->right == nullptr) {
						novel_branch->x_oriented = false;
						focus->right = novel_branch;
						break;
					}
					else {
						focus = focus->right;
						continue;
					}
				}
			}
			else {
				if (novel_branch->position.Y <= focus->position.Y) {
					if (focus->left == nullptr) {
						novel_branch->x_oriented = true;
						focus->left = novel_branch;
						break;
					}
					else {
						focus = focus->left;
						continue;
					}
				}
				else {
					if (focus->right == nullptr) {
						novel_branch->x_oriented = true;
						focus->right = novel_branch;
						break;
					}
					else {
						focus = focus->right;
						continue;
					}
				}
			}
		}
	}
	KD_Branch const * FindNearest(const Pgrd& target) {

		//special root case
		if (root == nullptr) return nullptr;

		grd best_distance = (root->position - target).Size();
		KD_Branch const * result = root;

		std::stack<KD_Branch const *> to_search;
		to_search.push(root);

		while (!to_search.empty()) {
			const auto * focus = to_search.top();
			to_search.pop();

			const grd distance = (target - focus->position).Size();
			if (distance < best_distance) {
				best_distance = distance;
				result = focus;
			}

			if (focus->x_oriented) {
				const grd axis_distance = target.X - focus->position.X;
				if (axis_distance < best_distance) {
					if (focus->left != nullptr) {
						to_search.push(focus->left);
					}
				}
				if (-axis_distance < best_distance) {
					if (focus->right != nullptr) {
						to_search.push(focus->right);
					}
				}
			}
			else {
				const grd axis_distance = target.Y - focus->position.Y;
				if (axis_distance < best_distance) {
					if (focus->left != nullptr) {
						to_search.push(focus->left);
					}
				}
				if (-axis_distance < best_distance) {
					if (focus->right != nullptr) {
						to_search.push(focus->right);
					}
				}
			}
		}

		return result;
	}
	std::list<KD_Branch const *> CollectRange(const Pgrd target, const grd range) {
		std::list<KD_Branch const *> result;

		//special root case
		if (root == nullptr) return result;

		std::stack<KD_Branch const *> to_search;
		to_search.push(root);

		while (!to_search.empty()) {
			auto * focus = to_search.top();
			to_search.pop();

			const grd distance = (target - focus->position).Size();
			if (distance < range) {
				result.push_back(focus);
			}

			if (focus->x_oriented) {
				const grd axis_distance = target.X - focus->position.X;
				if (axis_distance < range) {
					if (focus->left != nullptr) {
						to_search.push(focus->left);
					}
				}
				if (-axis_distance < range) {
					if (focus->right != nullptr) {
						to_search.push(focus->right);
					}
				}
			}
			else {
				const grd axis_distance = target.Y - focus->position.Y;
				if (axis_distance < range) {
					if (focus->left != nullptr) {
						to_search.push(focus->left);
					}
				}
				if (-axis_distance < range) {
					if (focus->right != nullptr) {
						to_search.push(focus->right);
					}
				}
			}
		}

		return result;
	}
};