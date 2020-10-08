#include "Voronoi.h"
#include "./DCEL/SimplexNoise.h"
#include "./DCEL/debugger.h"
//#include "./DCEL/citygen.h"

#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

#define JC_VORONOI_IMPLEMENTATION
#define JCV_REAL_TYPE double
#define JCV_ATAN2 atan2
#define JCV_FLT_MAX 1.7976931348623157E+308
#include "./DCEL/jc_voronoi.h"

double Random(double a, double b) {
  double random = ((double)rand()) / (double)RAND_MAX;
  double diff = b - a;
  double r = random * diff;
  return a + r;
}

double RandomBates(double a, double b, int strength = 3) {
  double sum = 0;
  for (int i = 0; i < strength; ++i) {
    sum += ((double)rand()) / (double)RAND_MAX;
  }
  sum /= (double)strength;

  double diff = b - a;
  double r = sum * diff;
  return a + r;
}

Pgrd circularUniformPoint(grd radius = 1) {
  double t = 2 * M_PI * Random(0.0, 1.0);
  double u = Random(0.0, 1.0) + Random(0.0, 1.0);
  double r = u;
  if (u > 1)
    r = 2 - u;
  return Pgrd(r*cos(t), r*sin(t)) * radius;
}

Pgrd circularBatesPoint(grd radius = 1, int strength = 3) {
  constexpr double quart_pi = (0.5 * M_PI);
  double offset = (double)std::floor(Random(0.0, 4.0)) * quart_pi;
  double t = RandomBates(0.0, offset, strength) + offset;
  double u = Random(0.0, 1.0) + Random(0.0, 1.0);
  double r = u;
  if (u > 1)
    r = 2 - u;
  return Pgrd(r*cos(t), r*sin(t)) * radius;
}

std::list<Pgrd> poissonSample(grd region_radius, int point_count, grd mid_seperation)
{
  //generate points
  std::list<Pgrd> point_list;

  int safety = 10 * point_count;
  while (point_list.size() < point_count && --safety > 0)
  {
    Pgrd suggestion = circularUniformPoint(region_radius);

    bool safe = true;
    for (auto point : point_list)
    {
      if ((point - suggestion).Size() < mid_seperation) {
        safe = false;
        break;
      }
    }
    if (safe)
      point_list.push_back(suggestion);
  }

  return point_list;
}

std::list<Pgrd> gridSample(grd region_radius, int point_count, grd mid_seperation)
{
  //generate points
  std::list<Pgrd> point_list;

  const int width = floor(((region_radius * 2) / mid_seperation).n);
  const grd offset = (region_radius * 2) / width;
  const grd allign = (offset / 2) - region_radius;
  
  for (int x = 0; x < width; ++x) {
    const grd xo = offset * x + allign;
    for (int y = 0; y < width; ++y) {
      const grd yo = offset * y + allign;
      point_list.push_back(Pgrd(xo,yo));
      if (--point_count < 1) break;
    }
    if (--point_count < 1) break;
  }

  return point_list;
}

std::list<Pgrd> gridRandomSample(grd region_radius, int point_count, grd mid_seperation)
{
  //generate points
  std::list<Pgrd> point_list;

  const int width = floor(((region_radius * 2) / mid_seperation).n);
  const grd offset = (region_radius * 2) / width;
  const grd allign = (offset / 2) - region_radius;

  for (int x = 0; x < width; ++x) {
    const grd xo = offset * x + allign;
    for (int y = 0; y < width; ++y) {
      const grd yo = offset * y + allign;
      point_list.push_back(Pgrd(xo, yo) + circularUniformPoint(mid_seperation / 3));
      if (--point_count < 1) break;
    }
    if (--point_count < 1) break;
  }

  return point_list;
}

std::list<Pgrd> gridSimplexSample(grd region_radius, int point_count, grd mid_seperation)
{
  //generate points
  std::list<Pgrd> point_list;

  const int width = floor(((region_radius * 2) / mid_seperation).n);
  const grd offset = (region_radius * 2) / width;
  const grd allign = (offset / 2) - region_radius;

  for (int x = 0; x < width; ++x) {
    const grd xo = offset * x + allign;
    for (int y = 0; y < width; ++y) {
      const grd yo = offset * y + allign;
      const double sn = SimplexNoise::noise(xo.n, yo.n);
      const Pgrd sno(sin(sn), cos(sn));
      point_list.push_back(Pgrd(xo, yo) + sno * (mid_seperation/3));
      if (--point_count < 1) break;
    }
    if (--point_count < 1) break;
  }

  return point_list;
}

std::list<Pgrd> poissonAllignedSample(grd region_radius, int point_count, grd mid_seperation, int strength = 3)
{
  //generate points
  std::list<Pgrd> point_list;

  int safety = 10 * point_count;
  while (point_list.size() < point_count && --safety > 0)
  {
    Pgrd suggestion = circularBatesPoint(region_radius, strength);

    bool safe = true;
    for (auto point : point_list)
    {
      if ((point - suggestion).Size() < mid_seperation) {
        safe = false;
        break;
      }
    }
    if (safe)
      point_list.push_back(suggestion);
  }

  return point_list;
}

class config_gen::config_gen_data {
  void allocate_hall(const Pgrd a, const Pgrd b) {
    //generate hallway "box" for edge

    auto par = (b - a);
    if (par.Size() > grd(0)) {
      par.Normalize();
      par *= options.HallWidth / 2;
      Pgrd perp(par.Y, -par.X);

      std::list<Pgrd> cell_boundary;
      cell_boundary.push_back((a - par) - perp);
      cell_boundary.push_back((b + par) - perp);
      cell_boundary.push_back((b + par) + perp);
      cell_boundary.push_back((a - par) + perp);

      allocateBoundaryFromInto(cell_boundary, blocks, halls);
    }
  }
  void gen_fill_halls() {
    const jcv_site* sites = jcv_diagram_get_sites(diagram);

    int count = 1;
    for (int i = 0; i < diagram->numsites; ++i) {

      const jcv_site* site = &sites[i];

      const jcv_graphedge* e = site->edges;

      while (e)
      {
        if (e->neighbor == nullptr || e->neighbor > site) {
          const auto a = Pgrd(e->pos[0].x, e->pos[0].y);
          const auto b = Pgrd(e->pos[1].x, e->pos[1].y);

          allocate_hall(a, b);
        }
        if (++count > options.throttle) break;

        e = e->next;
      }
      if (count > options.throttle) break;
    }
  }
  void gen_radius_halls() {
    const jcv_site* sites = jcv_diagram_get_sites(diagram);

    int count = 1;
    for (int i = 0; i < diagram->numsites; ++i) {

      const jcv_site* site = &sites[i];

      const jcv_graphedge* e = site->edges;

      const auto site_offset = Pgrd(site->p.x, site->p.y).Size();
      if (site_offset > options.BoundaryRadius) continue;

      while (e)
      {
        if (e->neighbor == nullptr || e->neighbor > site || ( Pgrd(e->neighbor->p.x, e->neighbor->p.y).Size()) > options.BoundaryRadius ){
          //generate hallway "box" for edge
          const auto a = Pgrd(e->pos[0].x, e->pos[0].y);
          const auto b = Pgrd(e->pos[1].x, e->pos[1].y);

          allocate_hall(a, b);
        }
        if (++count > options.throttle) break;

        e = e->next;
      }
      if (count > options.throttle) break;
    }
  }
  void gen_box_halls() {
    const jcv_site* sites = jcv_diagram_get_sites(diagram);

    int count = 1;
    for (int i = 0; i < diagram->numsites; ++i) {

      const jcv_site* site = &sites[i];

      const jcv_graphedge* e = site->edges;

      const auto site_offset = Pgrd(site->p.x, site->p.y);
      if (grd::abs(site_offset.X) > options.BoundaryRadius || grd::abs(site_offset.Y) > options.BoundaryRadius) continue;

      while (e)
      {
        if (e->neighbor == nullptr || e->neighbor > site || (grd::abs(e->neighbor->p.x) > options.BoundaryRadius || grd::abs(e->neighbor->p.y) > options.BoundaryRadius)) {
          //generate hallway "box" for edge
          const auto a = Pgrd(e->pos[0].x, e->pos[0].y);
          const auto b = Pgrd(e->pos[1].x, e->pos[1].y);

          allocate_hall(a, b);
        }
        if (++count > options.throttle) break;

        e = e->next;
      }
      if (count > options.throttle) break;
    }
  }
public:
  config_gen_data(config_gen &_options)
    : options(_options) {

  };
  ~config_gen_data() {
    //if (points) {
    //  delete points;
    //}
    if (diagram) {
      jcv_diagram_free(diagram);
      delete(diagram);
    }
    if (system) {
      delete system;
    }
  }
  void gen_points() {
    switch (options.PointType) {
    case ptPoisson:
      raw_points = poissonSample(options.GlobalRadius, options.MaxPoints, options.PointSpacing);
      break;
    case ptGrid:
      raw_points = gridSample(options.GlobalRadius, options.MaxPoints, options.PointSpacing);
      break;
    case ptRandomOffsetGrid:
      raw_points = gridRandomSample(options.GlobalRadius, options.MaxPoints, options.PointSpacing);
      break;
    case ptSimplexOffsetGrid:
      raw_points = gridSimplexSample(options.GlobalRadius, options.MaxPoints, options.PointSpacing);
      break;
    default:
      break;
    }
  }
  void gen_jcv() {
    jcv_rect bounding_box = { { -options.GlobalRadius.n, -options.GlobalRadius.n }, { options.GlobalRadius.n, options.GlobalRadius.n } };
    
    v_points = (jcv_point *)malloc(sizeof(jcv_point) * options.MaxPoints);
    int i = 0;
    for (auto point : raw_points)
    {
      v_points[i].x = point.X.n;
      v_points[i].y = point.Y.n;
      ++i;
    }

    diagram = new jcv_diagram();
    memset(diagram, 0, sizeof(jcv_diagram));
    jcv_diagram_generate(raw_points.size(), v_points, &bounding_box, diagram);
  }
  void gen_from_buffer(const std::vector<std::pair<Pgrd, Pgrd>>& buffer) {
    system = new DCEL();
    blocks.push_back(system->region());

    int count = 1;
    for(auto e : buffer){
      allocate_hall(e.first, e.second);
      if (++count > options.throttle) break;
    }

    for (auto iter = blocks.begin(); iter != blocks.end();) {
      auto p = iter;
      ++iter;

      if ((*p)->area() < 0) {
        exteriors.splice(exteriors.end(), blocks, p);
      }
    }
  }
  void gen_halls() {
    system = new DCEL();
    blocks.push_back(system->region());

    switch (options.BoundaryType) {
    case btFill:
      gen_fill_halls();
      break;
    case btRadius:
      gen_radius_halls();
      break;

    case btBox:
      gen_box_halls();
      break;
    }

    for (auto iter = blocks.begin(); iter != blocks.end();) {
      auto p = iter;
      ++iter;

      if ((*p)->area() < 0) {
        exteriors.splice(exteriors.end(), blocks, p);
      }
    }
  }

  std::list<Region *> const & get_halls() { return halls; };
  std::list<Region *> const & get_blocks() { return blocks; };
  std::list<Region *> const & get_exteriors() { return exteriors; };

private:
  config_gen const& options;

  std::list<Pgrd> raw_points;

  jcv_point *  v_points = nullptr;
  jcv_diagram * diagram = nullptr;

  DCEL * system = nullptr;

  std::list<Region *> halls;
  std::list<Region *> blocks;
  std::list<Region *> exteriors;
};


config_gen::config_gen() 
: data(new config_gen_data(*this)) {

}

config_gen::~config_gen() {
  delete data;
}

void config_gen::run() {
  if (PointType == ptRoadOverride) {
    ///CityGen temp;
    //data->gen_from_buffer(temp.Fill());
  }
  else {
    data->gen_points();
    data->gen_jcv();
    data->gen_halls();
  }
}

std::list<Region *> const & config_gen::halls() {
  return data->get_halls();
}

std::list<Region *> const & config_gen::blocks() {
  return data->get_blocks();
}

std::list<Region *> const & config_gen::exteriors() {
  return data->get_exteriors();
}