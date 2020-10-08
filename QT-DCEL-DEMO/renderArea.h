#pragma once
#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>

#include "./DCEL/Room_Boundary.h"

using LineBuffer = std::vector<std::pair<Pgrd, Pgrd>>;
using PolygonBuffer = std::vector<std::vector<Pgrd>>;
using RegionBuffer = std::list<Region *>;

class RenderArea : public QWidget
{
  Q_OBJECT

  using Shape = std::vector<QPolygonF>;
  using Group = std::pair<std::vector<Shape>, QColor>;
  using Segment = std::pair<LineBuffer, QColor>;

public:
  

  RenderArea(QWidget *parent = nullptr);

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

  void addRegionBuffer(const RegionBuffer& input, const QColor color);
  void addLineBuffer(const LineBuffer& input, const QColor color);
  void addPolygonBuffer(const PolygonBuffer& input, const QColor color);
  void resetDraw();
  void resetView();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  virtual void mouseMoveEvent(QMouseEvent * e) override;
  virtual void mousePressEvent(QMouseEvent * e) override;
  virtual void wheelEvent(QWheelEvent * e) override;

  std::vector<Group> Groupings;
  std::vector<Segment> Segments;

  QPointF last_mouse_pos;
  QPointF view_center;
  double view_zoom;
};

#endif