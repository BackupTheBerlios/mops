/****************************************************************
 *     MOPSLinux packaging system
 *     Update monitor
 *     $Id: main.cpp,v 1.2 2007/05/21 19:25:45 i27249 Exp $
 ***************************************************************/

#include <QApplication>
#include <QtGui>
#include "trayapp.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QTranslator translator;

	translator.load("updatechecker_ru");
	app.installTranslator(&translator);
	TrayApp *trayApp = new TrayApp;
	return app.exec();
}

