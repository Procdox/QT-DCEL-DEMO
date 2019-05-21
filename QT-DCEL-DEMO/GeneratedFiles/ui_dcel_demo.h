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
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
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
    QPushButton *renderButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *DCEL_DemoClass)
    {
        if (DCEL_DemoClass->objectName().isEmpty())
            DCEL_DemoClass->setObjectName(QString::fromUtf8("DCEL_DemoClass"));
        DCEL_DemoClass->resize(600, 407);
        centralWidget = new QWidget(DCEL_DemoClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        renderWidget = new RenderArea(centralWidget);
        renderWidget->setObjectName(QString::fromUtf8("renderWidget"));
        renderWidget->setGeometry(QRect(10, 10, 581, 311));
        renderButton = new QPushButton(centralWidget);
        renderButton->setObjectName(QString::fromUtf8("renderButton"));
        renderButton->setGeometry(QRect(260, 330, 75, 23));
        DCEL_DemoClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(DCEL_DemoClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 21));
        DCEL_DemoClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(DCEL_DemoClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        DCEL_DemoClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(DCEL_DemoClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        DCEL_DemoClass->setStatusBar(statusBar);

        retranslateUi(DCEL_DemoClass);
        QObject::connect(renderButton, SIGNAL(clicked()), renderWidget, SLOT(on_renderButton_clicked()));

        QMetaObject::connectSlotsByName(DCEL_DemoClass);
    } // setupUi

    void retranslateUi(QMainWindow *DCEL_DemoClass)
    {
        DCEL_DemoClass->setWindowTitle(QApplication::translate("DCEL_DemoClass", "DCEL_Demo", nullptr));
        renderButton->setText(QApplication::translate("DCEL_DemoClass", "Generate", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DCEL_DemoClass: public Ui_DCEL_DemoClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DCEL_DEMO_H
