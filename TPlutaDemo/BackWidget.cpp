#include "BackWidget.h"

void BackWidget::resizeEvent(QResizeEvent *event){

	leftContainer->setFixedHeight(this->height());
	foto->move(25, this->height() / 2 - 400);
	lPluta->move(30, this->height() / 2 + 50);
	lTomasz->move(25, this->height() / 2 + 100);
	lJobTitle->move(30, this->height() / 2 + 150);
	linkButton->move(40, this->height() / 2 + 200);
	openGLWidget->setFixedSize(this->width() - leftContainer->width(), this->height() - 50);
	TopBar->setFixedWidth(this->width() - 450);
}

void BackWidget::OpenWebsite(){
	ShellExecuteA(NULL, ("open"), LPCSTR("www.linkedin.com/in/tomasz-pluta-programista-c"), NULL, NULL, SW_SHOWNORMAL);
}