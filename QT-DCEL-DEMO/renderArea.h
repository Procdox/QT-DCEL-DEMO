#pragma once
#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>

#include "./DCEL/Room_Boundary.h"

using Buffer = std::list<std::pair<Pgrd, Pgrd>>;

class RenderArea : public QWidget
{
	Q_OBJECT

public:
	using Shape = std::list<QPolygonF>;
	using Group = std::pair<std::list<Shape>, QColor>;
	using Segment = std::pair<Buffer, QColor>;

	RenderArea(QWidget *parent = nullptr);

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	void addGrouping(const std::list<Region *>& input, const QColor color);
	void addBuffer(const Buffer& input, const QColor color);
	void resetDraw();
	void resetView();

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	virtual void mouseMoveEvent(QMouseEvent * e) override;
	virtual void mousePressEvent(QMouseEvent * e) override;
	virtual void wheelEvent(QWheelEvent * e) override;

	std::list<Group> Groupings;
	std::list<Segment> Segments;

	QPointF last_mouse_pos;
	QPointF view_center;
	double view_zoom;
};

#endif