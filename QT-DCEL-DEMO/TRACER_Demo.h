#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TRACER_Demo.h"

class TRACER_Demo : public QMainWindow
{
  Q_OBJECT

public:
  virtual ~TRACER_Demo() override;
  TRACER_Demo(QWidget *parent = Q_NULLPTR);

public slots:
  void start();

  void render_linebuffer(LineBuffer*, QColor);
  void render_polygonbuffer(PolygonBuffer*, QColor);
  void update_state(int);

private:
  class Data;
  Ui::TRACER_DemoClass ui;

  Data* d;

  RenderArea * ui_render;

  QSpinBox * ui_seed = 0;

  QPushButton * ui_start = 0;
  QComboBox * ui_state = 0;
  QLabel * ui_label = 0;
};