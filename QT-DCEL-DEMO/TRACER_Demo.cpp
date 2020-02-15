#include "TRACER_Demo.h"

#include "DCEL/Grid_Tools.h"
#include "TRACE/tracer.h"
#include <QDEBUG>
#include <QThread>

class TRACER_Demo::Data : public QThread {
	Q_OBJECT

public:
	virtual ~Data() override {};
	Data(QWidget* parent)
		: QThread(parent) {};

private:
	void run() override {
		qDebug() << "render working... " << QThread::currentThreadId();

		auto * target = new Buffer();
		if (current_config) {
			bool done = current_config->run(*target, step_count);

			if (done) {
				delete current_config;
				current_config = nullptr;
			}

		}
		emit update(target);
	}

signals:
	void update(Buffer*);
	
public:
	bool processing = true;
	Tracer* current_config = nullptr;
	int step_count = 0;
};

TRACER_Demo::~TRACER_Demo() {
	delete d;
}

TRACER_Demo::TRACER_Demo(QWidget *parent)
	: QMainWindow(parent)
	, d(new Data(this))
{
	ui.setupUi(this);

	ui_render = findChild<RenderArea *>(QString("renderWidget"));

	ui_seed = findChild<QSpinBox *>(QString("ui_seed"));
	ui_step = findChild<QSpinBox *>(QString("ui_step"));
		ui_step->setMinimum(1);
		ui_step->setMaximum(1000000);
		ui_step->setValue(50);
	ui_reset = findChild<QPushButton *>(QString("ui_reset"));
	ui_progress = findChild<QPushButton *>(QString("ui_progress"));

	bool res = true;

	connect(d, &Data::update, this, &TRACER_Demo::update_render);
	res &= !!connect(ui_reset, &QPushButton::pressed, this, &TRACER_Demo::start_render);
	res &= !!connect(ui_progress, &QPushButton::pressed, this, &TRACER_Demo::progress_render);

	Q_ASSERT(res);

	d->processing = false;
}

void TRACER_Demo::start_render()
{
	if (d->processing) return;
	qDebug() << "render started... " << QThread::currentThreadId();;
	d->processing = true;

	if (d->current_config)
		delete d->current_config;

	srand(ui_seed->value());

	d->current_config = new Tracer();
	d->step_count = ui_step->value();
	ui_render->resetDraw();

	d->start();
}

void TRACER_Demo::progress_render()
{
	if (d->current_config == nullptr) return;
	if (d->processing) return;
	qDebug() << "render progressed... " << QThread::currentThreadId();;
	d->processing = true;

	d->step_count = ui_step->value();

	d->start();
}

void TRACER_Demo::update_render(Buffer * packet)
{
	QColor inner = Qt::blue;
	inner.setAlphaF(.2);

	ui_render->resetDraw();
	ui_render->addBuffer(*packet, inner);
	delete packet;

	d->processing = false;
}

#include "TRACER_Demo.moc"