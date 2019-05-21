#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DCEL_Demo.h"

class DCEL_Demo : public QMainWindow
{
	Q_OBJECT

public:
	DCEL_Demo(QWidget *parent = Q_NULLPTR);

private:
	Ui::DCEL_DemoClass ui;
};
