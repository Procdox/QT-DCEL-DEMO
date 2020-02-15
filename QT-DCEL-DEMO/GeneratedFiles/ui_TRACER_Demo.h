/********************************************************************************
** Form generated from reading UI file 'TRACER_Demo.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRACER_DEMO_H
#define UI_TRACER_DEMO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "renderArea.h"

QT_BEGIN_NAMESPACE

class Ui_TRACER_DemoClass
{
public:
    QWidget *centralwidget;
    QSpinBox *ui_seed;
    RenderArea *renderWidget;
    QPushButton *ui_reset;
    QPushButton *ui_progress;
    QSpinBox *ui_step;
    QLabel *label;
    QLabel *label_2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *TRACER_DemoClass)
    {
        if (TRACER_DemoClass->objectName().isEmpty())
            TRACER_DemoClass->setObjectName(QString::fromUtf8("TRACER_DemoClass"));
        TRACER_DemoClass->resize(1069, 886);
        centralwidget = new QWidget(TRACER_DemoClass);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        ui_seed = new QSpinBox(centralwidget);
        ui_seed->setObjectName(QString::fromUtf8("ui_seed"));
        ui_seed->setGeometry(QRect(120, 780, 81, 22));
        renderWidget = new RenderArea(centralwidget);
        renderWidget->setObjectName(QString::fromUtf8("renderWidget"));
        renderWidget->setGeometry(QRect(10, 10, 1051, 761));
        ui_reset = new QPushButton(centralwidget);
        ui_reset->setObjectName(QString::fromUtf8("ui_reset"));
        ui_reset->setGeometry(QRect(120, 810, 75, 23));
        ui_progress = new QPushButton(centralwidget);
        ui_progress->setObjectName(QString::fromUtf8("ui_progress"));
        ui_progress->setGeometry(QRect(430, 810, 75, 23));
        ui_step = new QSpinBox(centralwidget);
        ui_step->setObjectName(QString::fromUtf8("ui_step"));
        ui_step->setGeometry(QRect(430, 780, 81, 22));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(70, 780, 47, 13));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(340, 780, 81, 16));
        TRACER_DemoClass->setCentralWidget(centralwidget);
        menubar = new QMenuBar(TRACER_DemoClass);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1069, 21));
        TRACER_DemoClass->setMenuBar(menubar);
        statusbar = new QStatusBar(TRACER_DemoClass);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        TRACER_DemoClass->setStatusBar(statusbar);

        retranslateUi(TRACER_DemoClass);

        QMetaObject::connectSlotsByName(TRACER_DemoClass);
    } // setupUi

    void retranslateUi(QMainWindow *TRACER_DemoClass)
    {
        TRACER_DemoClass->setWindowTitle(QApplication::translate("TRACER_DemoClass", "MainWindow", nullptr));
        ui_reset->setText(QApplication::translate("TRACER_DemoClass", "Reset", nullptr));
        ui_progress->setText(QApplication::translate("TRACER_DemoClass", "Progress", nullptr));
        label->setText(QApplication::translate("TRACER_DemoClass", "Seed", nullptr));
        label_2->setText(QApplication::translate("TRACER_DemoClass", "Number of Steps", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TRACER_DemoClass: public Ui_TRACER_DemoClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRACER_DEMO_H
