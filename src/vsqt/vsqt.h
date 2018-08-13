#pragma once

#include <QtWidgets/QWidget>
#include "ui_vsqt.h"

class Vsqt : public QWidget
{
	Q_OBJECT

public:
	Vsqt(QWidget *parent = Q_NULLPTR);

private:
	Ui::VsqtClass ui;
};
