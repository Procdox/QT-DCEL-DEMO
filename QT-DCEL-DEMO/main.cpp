//#include "DCEL_Demo.h"
#include "TRACER_Demo.h"
#include <QtWidgets/QApplication>
//#include <Voronoi.h>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TRACER_Demo w;
	w.show();
	return a.exec();
}