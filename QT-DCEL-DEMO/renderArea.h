#pragma once
#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>

#include "./DCEL/Grid_Tools.h"

class RenderArea : public QWidget
{
	Q_OBJECT

public:

	RenderArea(QWidget *parent = nullptr);

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:
	void on_renderButton_clicked();

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	FLL<QPolygonF> shapes;
	void generate(DCEL<Pgrd> * system);
};

#endif