#include "renderarea.h"


#include <QPainter>
#include <QComboBox>

#include <QMouseEvent>
#include <QWheelEvent>

std::list<Pgrd> inset(Face * target) {
  Room_Boundary focus = Room_Boundary(target);
  auto border = focus.Inset(.4);

  std::list<Pgrd> polygon;
  for (auto point : border)
    polygon.push_back(point);

  return polygon;
}

//--------------------------------------------------------------------------------

RenderArea::RenderArea(QWidget *parent)
  : QWidget(parent)
{
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);

  view_center = QPointF(0.0, 0.0);
  view_zoom = 1.0;
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

  QPainter painter(this);
  painter.setPen(Qt::SolidLine);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QMatrix offset(1, 0, 0, 1, view_center.x(), view_center.y());
  QMatrix frustrum(view_zoom, 0, 0, view_zoom, 0, 0);
  QMatrix center(1, 0, 0, 1, width() / 2, height() / 2);

  QBrush brush;
  
  
  brush.setStyle(Qt::SolidPattern);

  for (auto set : Segments) {
    painter.setPen(set.second);

    for (auto line : set.first) {
      QPolygonF temp;
      auto A = ((QPointF(line.first.X.n, line.first.Y.n) * offset) * frustrum) * center;
      auto B = ((QPointF(line.second.X.n, line.second.Y.n) * offset) * frustrum) * center;

      temp.append(A);
      temp.append(B);

      QPainterPath positive;
      positive.addPolygon(temp);
      painter.drawPath(positive);
    }
  }

  painter.setPen(Qt::black);

  for (auto group : Groupings) {
    brush.setColor(group.second);

    for (auto polygon : group.first) {
      QPainterPath positive;
      for (auto bound : polygon) {
        positive.addPolygon(((bound * offset) * frustrum) * center);
      }
      //painter.drawPath(positive);
      painter.fillPath(positive, brush);
      //painter.drawPath(path);
    }
  }
}

void RenderArea::addRegionBuffer(const RegionBuffer& input, const QColor color) {
  Group result;
  result.second = color;

  for (auto region : input) {
    Shape temp;

    for (auto face : region->getBounds()) {
      QPolygonF poly;
      //for (auto point : inset(face))
      for (auto point : face->getLoopPoints())
        poly.append(QPointF(point.X.clamp(-1000, 1000).n, point.Y.clamp(-1000, 1000).n));

      temp.push_back(std::move(poly));
    }
    result.first.push_back(std::move(temp));
  }

  Groupings.push_back(std::move(result));

  update();
}

void RenderArea::addLineBuffer(const LineBuffer& input, const QColor color) {
  Segments.push_back(std::make_pair(input, color));

  update();
}

void RenderArea::addPolygonBuffer(const PolygonBuffer& input, const QColor color) {
  Group result;
  result.second = color;

  for (auto region : input) {
    Shape temp;

    QPolygonF poly;
    for (auto point : region)
      poly.append(QPointF(point.X.clamp(-1000,1000).n, point.Y.clamp(-1000, 1000).n));

    std::vector<QPolygonF> shape;
    shape.push_back(poly);
    result.first.push_back(std::move(shape));
  }

  Groupings.push_back(std::move(result));

  update();
}

void RenderArea::resetDraw() {
  Groupings.clear();
  Segments.clear();

  update();
}

void RenderArea::resetView() {
  view_center = QPointF(0.0, 0.0);
  view_zoom = 1.0;

  update();
}

void RenderArea::mouseMoveEvent(QMouseEvent * e) {
  const QPointF delta = e->pos() - last_mouse_pos;
  last_mouse_pos = e->pos();

  view_center += delta / view_zoom;

  update();
}
void RenderArea::mousePressEvent(QMouseEvent * e) {
  last_mouse_pos = e->pos();
}
void RenderArea::wheelEvent(QWheelEvent * e) {
  view_zoom *= e->angleDelta().y() > 0 ? 1.1 : 0.9;

  update();
}