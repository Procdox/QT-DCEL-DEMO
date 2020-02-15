/********************************************************************************
** Form generated from reading UI file 'DCEL_Demo.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DCEL_DEMO_H
#define UI_DCEL_DEMO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "renderArea.h"

QT_BEGIN_NAMESPACE

class Ui_DCEL_DemoClass
{
public:
    QWidget *centralWidget;
    RenderArea *renderWidget;
    QComboBox *pointTypeBox;
    QSpinBox *throttle;
    QSpinBox *seed;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QComboBox *boundTypeBox;
    QLabel *label_4;
    QLineEdit *pointSpacing;
    QLabel *label_5;
    QLineEdit *pointCount;
    QLabel *label_6;
    QLineEdit *centerRadius;
    QLabel *label_7;
    QLabel *label_8;
    QLineEdit *globalRadius;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DCEL_DemoClass)
    {
        if (DCEL_DemoClass->objectName().isEmpty())
            DCEL_DemoClass->setObjectName(QString::fromUtf8("DCEL_DemoClass"));
        DCEL_DemoClass->resize(1070, 888);
        centralWidget = new QWidget(DCEL_DemoClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        renderWidget = new RenderArea(centralWidget);
        renderWidget->setObjectName(QString::fromUtf8("renderWidget"));
        renderWidget->setGeometry(QRect(10, 10, 1051, 761));
        pointTypeBox = new QComboBox(centralWidget);
        pointTypeBox->addItem(QString());
        pointTypeBox->addItem(QString());
        pointTypeBox->addItem(QString());
        pointTypeBox->addItem(QString());
        pointTypeBox->addItem(QString());
        pointTypeBox->addItem(QString());
        pointTypeBox->addItem(QString());
        pointTypeBox->setObjectName(QString::fromUtf8("pointTypeBox"));
        pointTypeBox->setGeometry(QRect(330, 780, 151, 22));
        throttle = new QSpinBox(centralWidget);
        throttle->setObjectName(QString::fromUtf8("throttle"));
        throttle->setGeometry(QRect(550, 780, 71, 22));
        seed = new QSpinBox(centralWidget);
        seed->setObjectName(QString::fromUtf8("seed"));
        seed->setGeometry(QRect(130, 779, 81, 22));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(80, 780, 51, 21));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(490, 780, 51, 21));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(290, 780, 41, 21));
        boundTypeBox = new QComboBox(centralWidget);
        boundTypeBox->addItem(QString());
        boundTypeBox->addItem(QString());
        boundTypeBox->addItem(QString());
        boundTypeBox->setObjectName(QString::fromUtf8("boundTypeBox"));
        boundTypeBox->setGeometry(QRect(790, 779, 71, 22));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(740, 780, 51, 20));
        pointSpacing = new QLineEdit(centralWidget);
        pointSpacing->setObjectName(QString::fromUtf8("pointSpacing"));
        pointSpacing->setGeometry(QRect(340, 812, 71, 21));
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(290, 810, 51, 20));
        pointCount = new QLineEdit(centralWidget);
        pointCount->setObjectName(QString::fromUtf8("pointCount"));
        pointCount->setGeometry(QRect(550, 810, 71, 20));
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(490, 810, 61, 20));
        centerRadius = new QLineEdit(centralWidget);
        centerRadius->setObjectName(QString::fromUtf8("centerRadius"));
        centerRadius->setGeometry(QRect(790, 810, 71, 20));
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(740, 810, 51, 21));
        label_8 = new QLabel(centralWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(80, 809, 51, 21));
        globalRadius = new QLineEdit(centralWidget);
        globalRadius->setObjectName(QString::fromUtf8("globalRadius"));
        globalRadius->setGeometry(QRect(130, 810, 81, 20));
        DCEL_DemoClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(DCEL_DemoClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1070, 21));
        DCEL_DemoClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DCEL_DemoClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        DCEL_DemoClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(DCEL_DemoClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        DCEL_DemoClass->setStatusBar(statusBar);

        retranslateUi(DCEL_DemoClass);

        QMetaObject::connectSlotsByName(DCEL_DemoClass);
    } // setupUi

    void retranslateUi(QMainWindow *DCEL_DemoClass)
    {
        DCEL_DemoClass->setWindowTitle(QApplication::translate("DCEL_DemoClass", "DCEL_Demo", nullptr));
        pointTypeBox->setItemText(0, QApplication::translate("DCEL_DemoClass", "Poisson", nullptr));
        pointTypeBox->setItemText(1, QApplication::translate("DCEL_DemoClass", "Biased Poisson", nullptr));
        pointTypeBox->setItemText(2, QApplication::translate("DCEL_DemoClass", "Grid", nullptr));
        pointTypeBox->setItemText(3, QApplication::translate("DCEL_DemoClass", "Random Offset Grid", nullptr));
        pointTypeBox->setItemText(4, QApplication::translate("DCEL_DemoClass", "Simplex Offset Grid", nullptr));
        pointTypeBox->setItemText(5, QApplication::translate("DCEL_DemoClass", "Complex Grid", nullptr));
        pointTypeBox->setItemText(6, QApplication::translate("DCEL_DemoClass", "Road Override", nullptr));

        label->setText(QApplication::translate("DCEL_DemoClass", "Seed:", nullptr));
        label_2->setText(QApplication::translate("DCEL_DemoClass", "Throttle:", nullptr));
        label_3->setText(QApplication::translate("DCEL_DemoClass", "Point:", nullptr));
        boundTypeBox->setItemText(0, QApplication::translate("DCEL_DemoClass", "Radius", nullptr));
        boundTypeBox->setItemText(1, QApplication::translate("DCEL_DemoClass", "Box", nullptr));
        boundTypeBox->setItemText(2, QApplication::translate("DCEL_DemoClass", "Fill", nullptr));

        label_4->setText(QApplication::translate("DCEL_DemoClass", "Bound:", nullptr));
        pointSpacing->setText(QApplication::translate("DCEL_DemoClass", "15.0", nullptr));
        label_5->setText(QApplication::translate("DCEL_DemoClass", "Spacing:", nullptr));
        pointCount->setText(QApplication::translate("DCEL_DemoClass", "300", nullptr));
        label_6->setText(QApplication::translate("DCEL_DemoClass", "Block Limit:", nullptr));
        centerRadius->setText(QApplication::translate("DCEL_DemoClass", "30.0", nullptr));
        label_7->setText(QApplication::translate("DCEL_DemoClass", "Radius:", nullptr));
        label_8->setText(QApplication::translate("DCEL_DemoClass", "Radius:", nullptr));
        globalRadius->setText(QApplication::translate("DCEL_DemoClass", "70.0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DCEL_DemoClass: public Ui_DCEL_DemoClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DCEL_DEMO_H
