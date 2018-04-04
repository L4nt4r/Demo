#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TPlutaDemo.h"

class TPlutaDemo : public QMainWindow
{
	Q_OBJECT

public:
	TPlutaDemo(QWidget *parent = Q_NULLPTR);

private:
	Ui::TPlutaDemoClass ui;
};
