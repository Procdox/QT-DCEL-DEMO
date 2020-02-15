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
	void start_render();
	void progress_render();
	void update_render(Buffer*);

private:
	class Data;
	Ui::TRACER_DemoClass ui;

	Data* d;

	RenderArea * ui_render;

	QSpinBox * ui_seed = 0;
	QSpinBox * ui_step = 0;

	QPushButton * ui_reset = 0;
	QPushButton * ui_progress = 0;
};