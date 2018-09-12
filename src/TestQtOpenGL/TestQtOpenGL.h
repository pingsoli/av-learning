#pragma once

#include <QtWidgets/QWidget>
#include "ui_TestQtOpenGL.h"

class TestQtOpenGL : public QWidget
{
    Q_OBJECT

public:
    TestQtOpenGL(QWidget *parent = Q_NULLPTR);

private:
    Ui::TestQtOpenGLClass ui;
};
