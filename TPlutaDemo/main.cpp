#include "TPlutaDemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TPlutaDemo w;
	w.show();
	return a.exec();
}
