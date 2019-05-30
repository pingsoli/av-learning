#include "TestQtOpenGL.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestQtOpenGL w;

    w.show();
    return a.exec();
}
