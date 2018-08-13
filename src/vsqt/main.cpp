#include "vsqt.h"
#include <QtWidgets/QApplication>

#include <iostream>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Vsqt w;
	w.show();

	// print output on UI program
	std::cout << "this is testing project" << std::endl;

	return a.exec();
}
