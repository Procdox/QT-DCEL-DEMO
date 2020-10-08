#include "debugger.h"
#include <QDebug>

void debug_proxy(const char * input) {
  qDebug() << input;
}

void debug_proxy(double input) {
  qDebug() << input;
}

void debug_proxy(int input) {
  qDebug() << input;
}

void debug_proxy(const char * what, double input) {
  qDebug() << what << input;
}

void debug_proxy(const char * what, int input) {
  qDebug() << what << input;
}

void debug_proxy(const char * what, double x, double y) {
  qDebug() << x << "," << y;
}

void debug_proxy(double x, double y) {
  qDebug().nospace() << qSetRealNumberPrecision(10) << x << "," << y;
}
