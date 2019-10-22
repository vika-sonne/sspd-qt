#include <iostream>
#include <QCoreApplication>
#include <QDateTime>
#include <QSerialPortInfo>
#include <QSerialPort>
#include "applicationclass.h"

int main(int argc, char *argv[])
{
	ApplicationClass a(argc, argv);

	if(a.isArgsError)
	{
		std::cout << "Run to help: \"" << argv[0] << "\" -h" << std::endl;
		return -1;
	}

	if (a.isExit)
		return 0;

	return a.exec();
}
