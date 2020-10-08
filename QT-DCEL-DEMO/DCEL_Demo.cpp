#include "DCEL_Demo.h"

#include "./DCEL/Grid_Tools.h"
#include "Voronoi.h"
#include <QDEBUG>
#include <QThread>

class DCEL_Demo::Data : public QThread {
  Q_OBJECT

public:
  virtual ~Data() override {};
  Data(QWidget* parent)
    : QThread(parent) {};

private:
  void run() override {
    qDebug() << "render working... " << QThread::currentThreadId();;
    current_config->run();
    emit resultReady();
  }

signals:
  void resultReady();

public:
  bool processing = true;
  config_gen* current_config;
};

DCEL_Demo::~DCEL_Demo() {
  delete d;
}

DCEL_Demo::DCEL_Demo(QWidget *parent)
  : QMainWindow(parent)
  , d(new Data(this))
{
  ui.setupUi(this);

  ui_render = findChild<RenderArea *>(QString("renderWidget"));

  ui_seed = findChild<QSpinBox *>(QString("seed"));
  ui_global_bound = findChild<QLineEdit *>(QString("globalRadius"));
    ui_global_bound->setValidator(new QDoubleValidator(0.5, 200, 2, this));

  ui_point_option = findChild<QComboBox *>(QString("pointTypeBox"));
  ui_spacing = findChild<QLineEdit *>(QString("pointSpacing"));
    ui_spacing->setValidator(new QDoubleValidator(.5, 100, 2, this));
  ui_max_count = findChild<QLineEdit *>(QString("pointCount"));
    ui_max_count->setValidator(new QIntValidator(0, 100, this));
  ui_throttle = findChild<QSpinBox *>(QString("throttle"));
    ui_throttle->setMinimum(1);
    ui_throttle->setMaximum(1000000);
    ui_throttle->setValue(200);

  ui_bound_option = findChild<QComboBox *>(QString("boundTypeBox"));
  ui_center_bound = findChild<QLineEdit *>(QString("centerRadius"));
    ui_center_bound->setValidator(new QDoubleValidator(0.5, 200, 2, this));

  connect(d, &Data::resultReady, this, &DCEL_Demo::render_finished);


  bool res = true;
  res &= !!connect(ui_seed, qOverload<int>(&QSpinBox::valueChanged), this, &DCEL_Demo::update_render);
  res &= !!connect(ui_global_bound, &QLineEdit::editingFinished, this, &DCEL_Demo::update_render);

  res &= !!connect(ui_point_option, qOverload<int>(&QComboBox::currentIndexChanged), this, &DCEL_Demo::update_render);
  res &= !!connect(ui_spacing, &QLineEdit::editingFinished, this, &DCEL_Demo::update_render);
  res &= !!connect(ui_max_count, &QLineEdit::editingFinished, this, &DCEL_Demo::update_render);
  res &= !!connect(ui_throttle, qOverload<int>(&QSpinBox::valueChanged), this, &DCEL_Demo::update_render);

  res &= !!connect(ui_bound_option, qOverload<int>(&QComboBox::currentIndexChanged), this, &DCEL_Demo::update_render);
  res &= !!connect(ui_center_bound, &QLineEdit::editingFinished, this, &DCEL_Demo::update_render);

  Q_ASSERT(res);

  d->processing = false;
  update_render();
}

void DCEL_Demo::update_render()
{
  if (d->processing) return;
  qDebug() << "render started... " << QThread::currentThreadId();;
  d->processing = true;

  srand(ui_seed->value());

  d->current_config = new config_gen();
  d->current_config->GlobalRadius = ui_global_bound->text().toDouble();

  d->current_config->PointType = (config_gen::pointtypeenum)ui_point_option->currentIndex();
  d->current_config->PointSpacing = ui_spacing->text().toDouble();
  d->current_config->MaxPoints = ui_max_count->text().toDouble();
  d->current_config->throttle = ui_throttle->value();

  d->current_config->BoundaryType = (config_gen::boundarytypeenum)ui_bound_option->currentIndex();
  d->current_config->BoundaryRadius = ui_center_bound->text().toDouble();

  d->start();
}

void DCEL_Demo::render_finished() {
  qDebug() << "render recieved... " << QThread::currentThreadId();;

  QColor inner = Qt::blue;
  inner.setAlphaF(.2);
  QColor outer = Qt::red;
  outer.setAlphaF(.2);

  ui_render->resetDraw();
  ui_render->addRegionBuffer(d->current_config->halls(), inner);
  ui_render->addRegionBuffer(d->current_config->blocks(), outer);

  delete d->current_config;
  d->processing = false;
}

#include "DCEL_Demo.moc"