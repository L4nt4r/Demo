#pragma once
#include <QtWidgets>
#include "OpenGLWnd.h"

class BackWidget : public QWidget{
Q_OBJECT
public:
	BackWidget(QWidget *widget = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()){}
	~BackWidget(){}
	QWidget *leftContainer;
	QWidget *foto;
	QWidget *TopBar;
	QLabel *lPluta;
	QLabel *lTomasz;
	QLabel *lJobTitle;
	QPushButton *linkButton;
	OpenGLWnd *openGLWidget;
private:
	void resizeEvent(QResizeEvent *event);
public slots:
	void OpenWebsite();
};