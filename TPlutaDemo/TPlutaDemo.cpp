#include "TPlutaDemo.h"

TPlutaDemo::TPlutaDemo(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.centralWidget->leftContainer = ui.leftContainer;
	ui.centralWidget->foto = ui.foto;
	ui.centralWidget->lPluta = ui.lPluta;
	ui.centralWidget->lTomasz = ui.lTomasz;
	ui.centralWidget->lJobTitle = ui.lJobTitle;
	ui.centralWidget->linkButton = ui.linkButton;
	ui.centralWidget->openGLWidget = ui.openGLWidget;
	ui.centralWidget->TopBar = ui.TopBar;


	connect(ui.openGLWidget, SIGNAL(showStatusText(QString)), this, SLOT(SetStatusText(QString)));
	connect(ui.linkButton, SIGNAL(clicked()), ui.centralWidget, SLOT(OpenWebsite()));
	//this->showMaximized();

}


void TPlutaDemo::SetStatusText(QString str){
	ui.statusBar->showMessage(str);
}