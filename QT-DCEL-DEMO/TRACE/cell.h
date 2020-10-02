#pragma once
#include "DCEL/Grid_Point.h"
#include <stack>
#include <vector>

// allows for fast lookup of nearby positional elements <t>
// IS NOT responsible for data lifetime
template<typename t>
class CellSearch {
  const Pgrd offset;
  const Pgrd cell_width;
  const int width;
  const int height;
  t null;

  std::vector<std::vector<std::pair<Pgrd,t>>> cells; 
public:
  CellSearch(Pgrd area, int _width, int _height, t _null) 
  : offset(area / 2)
  , cell_width(area.X/_width, area.Y/_height) 
  , width(_width)
  , height(_height) 
  , null(_null) {
    cells.resize(width * height);
  }
private:
  struct coord : public std::pair<int, int> {
    int& x() {return first;}
    int& y() {return second;}
    const int& x() const {return first;}
    const int& y() const { return second; }
    coord(int x, int y) : std::pair<int,int>(x,y) {}
  };
  coord to_coord(const Pgrd& point) const {
    const Pgrd adjust = point + offset;
    const int x = std::floor((adjust.X / cell_width.X).n);
    const int y = std::floor((adjust.Y / cell_width.Y).n);

    return coord(x,y);
  }
  int index(const coord& c) const {
     return c.x()*width + c.y();
  }
  int index(const Pgrd& point) const {
    return index(to_coord(point));
  }

  std::vector<int> cellsInRange(const Pgrd& target, grd distance_squared) const {
    std::vector<int> result;

    const Pgrd adjust = target + offset;
    //find closest point (clamp target)
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        Pgrd min(cell_width.X * x, cell_width.Y * y);
        Pgrd max(cell_width.X * (x+1), cell_width.Y * (y+1));
        Pgrd closest_point = adjust.clamp(min, max);
        if((adjust - closest_point).SizeSquared() < distance_squared)
          result.push_back(index(coord(x, y)));
      }
    }
    return result;
  }
    
public:
  void AddPoint(const Pgrd& insert, t _data) {
    cells[index(insert)].push_back(std::make_pair(insert,_data));
  }
    //USE THIS ONE
  t FindNearest(const Pgrd& target, grd max_distance) const {
    grd best_distance = max_distance * max_distance;
    t best = null;

    auto search = [&target, &best_distance, &best, this](int cell_index) {
      const auto & cell = cells[cell_index];
      for (auto pair : cell) {
        grd distance = (target - pair.first).SizeSquared();
        if (distance < best_distance) {
          best = pair.second;
          best_distance = distance;
        }
      }
    };
    const int t_idx = index(target);
    search(t_idx);
    
    for (auto cell_index : cellsInRange(target, best_distance)) {
      if(cell_index == t_idx) continue;
      search(cell_index);
    }

    return best;
  }
  //NOT RECOMMENED, queries the whole space if the initial cell is empty
  t FindNearest(const Pgrd& target) const {
    return FindNearest(target, offset.Size() * 2);
  }
  std::vector<t> CollectRange(const Pgrd& target, const grd range) const {
    std::vector<t> result;
    const grd range_square = range * range;

    for (auto cell_index : cellsInRange(target, range_square)) {
      const auto & cell = cells[cell_index];
      for (auto pair : cell) {
        grd distance = (target - pair.first).SizeSquared();
        if (distance < range_square)
          result.push_back(pair.second);
      }
    }

    return result;
  }
};