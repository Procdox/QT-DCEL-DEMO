#include "renderarea.h"
#include "./DCEL/Room_Boundary.h"

#include <QPainter>

#include <cstring>
#include <cstdlib>

#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

#define JC_VORONOI_IMPLEMENTATION
#define JCV_REAL_TYPE double
#define JCV_ATAN2 atan2
#define JCV_FLT_MAX 1.7976931348623157E+308
#include "./DCEL/jc_voronoi.h"

#define render_scale 3

RenderArea::RenderArea(QWidget *parent)
	: QWidget(parent)
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

QSize RenderArea::minimumSizeHint() const
{
	return QSize(100, 100);
}

QSize RenderArea::sizeHint() const
{
	return QSize(400, 200);
}

void RenderArea::paintEvent(QPaintEvent * /* event */)
{
	static const QPoint points[4] = {
		QPoint(10, 80),
		QPoint(20, 10),
		QPoint(80, 30),
		QPoint(90, 70)
	};

	QRect rect(10, 20, 80, 60);

	QPainterPath path;
	path.moveTo(20, 80);
	path.lineTo(20, 30);
	path.cubicTo(80, 0, 50, 50, 80, 80);

	int startAngle = 20 * 16;
	int arcLength = 120 * 16;

	QPainter painter(this);
	painter.setPen(Qt::SolidLine);
	painter.setBrush(Qt::LinearGradientPattern);
	painter.setRenderHint(QPainter::Antialiasing, true);

	painter.save();
	painter.translate(width() / 2, height() / 2);

	for (auto polygon : shapes)
		painter.drawPolygon(polygon);

	painter.restore();

	painter.setRenderHint(QPainter::Antialiasing, false);
	painter.setPen(palette().dark().color());
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}


float Random(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

Pgrd circularUniformPoint(grd radius = 1) {
	float t = 2 * M_PI * Random(0.f, 1.f);
	float u = Random(0.f, 1.f) + Random(0.f, 1.f);
	float r = u;
	if (u > 1)
		r = 2 - u;
	return Pgrd(r*cos(t), r*sin(t)) * radius;

}

FLL<Pgrd> poissonSample(grd region_radius, int point_count, grd mid_seperation)
{
	//generate points
	FLL<Pgrd> point_list;

	int safety = 10 * point_count;
	while (point_list.size() < point_count && --safety > 0)
	{
		Pgrd suggestion = circularUniformPoint(region_radius);

		bool safe = true;
		for (auto point : point_list)
		{
			if ((point - suggestion).Size() < 10) {
				safe = false;
				break;
			}
		}
		if (safe)
			point_list.append(suggestion);
	}

	return point_list;
}

void RenderArea::generate(DCEL<Pgrd> * system)
{
	Region_List Exteriors;
	Region_List cells;
	Exteriors.append(system->region());

	const grd region(70);
	const int max_point_count = 100;
	const grd span(10);

	jcv_rect bounding_box = { { -region.n, -region.n }, { region.n, region.n } };

	jcv_point points[max_point_count];
	jcv_diagram diagram;

	memset(&diagram, 0, sizeof(jcv_diagram));

	auto point_list = poissonSample(region, max_point_count, span);

	int i = 0;
	for (auto point : point_list)
	{
		points[i].x = point.X.n;
		points[i].y = point.Y.n;
		++i;
	}

	jcv_diagram_generate(point_list.size(), (const jcv_point *)points, &bounding_box, &diagram);

	point_list.clear();

	const jcv_site* sites = jcv_diagram_get_sites(&diagram);

	for (int i = 0; i < diagram.numsites; ++i) {
		FLL<Pgrd> cell_boundary;

		const jcv_site* site = &sites[i];

		const jcv_graphedge* e = site->edges;

		while (e)
		{
			cell_boundary.push(Pgrd(e->pos[0].x, e->pos[0].y));
			e = e->next;
		}

		Region_List novel_cells;
		allocateBoundaryFromInto(cell_boundary, Exteriors, novel_cells);
		cells.absorb(novel_cells);
		point_list.append(Pgrd(site->p.x, site->p.y));
	}


	Region_List central;
	Region_List peripheral;

	auto p = point_list.begin();
	for (auto cell : cells)
	{
		auto size = (*p).Size();
		if (size < 30)
		{
			central.append(cell);
		}
		else if (size < 50)
		{
			peripheral.append(cell);
		}
		++p;
	}

	mergeGroup(peripheral);

	shapes.clear();

	for (auto cell : central)
		for (auto boundary : cell->getBounds()) {
			Room_Boundary focus = Room_Boundary(boundary);
			auto border = focus.Inset(.4);

			QPolygonF polygon;
			for (auto point : border)
				polygon.append(QPointF(point.X.n * render_scale, point.Y.n * render_scale));

			shapes.append(std::move(polygon));
		}

	for (auto cell : peripheral)
		for (auto boundary : cell->getBounds()) {
			Room_Boundary focus = Room_Boundary(boundary);
			auto border = focus.Inset(.4);

			QPolygonF polygon;
			for (auto point : border)
				polygon.append(QPointF(point.X.n * render_scale, point.Y.n * render_scale));

			shapes.append(std::move(polygon));
		}
}

void RenderArea::on_renderButton_clicked()
{
	DCEL<Pgrd> * system = new DCEL<Pgrd>();

	generate(system);

	update();
}