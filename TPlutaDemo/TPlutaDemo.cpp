#include "TPlutaDemo.h"

TPlutaDemo::TPlutaDemo(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.openGLWidget, SIGNAL(showStatusText(QString)), this, SLOT(SetStatusText(QString)));
	this->showMaximized();

}

void TPlutaDemo::SetStatusText(QString str){
	ui.statusBar->showMessage(str);
}