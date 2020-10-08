#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DCEL_Demo.h"

class DCEL_Demo : public QMainWindow
{
  Q_OBJECT

public:
  virtual ~DCEL_Demo() override;
  DCEL_Demo(QWidget *parent = Q_NULLPTR);

public slots:
  void update_render();
  void render_finished();

private:
  class Data;

  Ui::DCEL_DemoClass ui;

  Data* d;

  RenderArea * ui_render;

  QSpinBox * ui_seed = 0;
  QLineEdit * ui_global_bound = 0;

  QComboBox * ui_point_option = 0;
  QLineEdit * ui_spacing = 0;
  QLineEdit * ui_max_count = 0;
  QSpinBox * ui_throttle = 0;

  QComboBox * ui_bound_option = 0;
  QLineEdit * ui_center_bound = 0;
};