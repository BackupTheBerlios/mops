/****************************************************************
 *     MOPSLinux packaging system
 *     Update monitor
 *     $Id: main.cpp,v 1.1 2007/05/15 22:28:58 i27249 Exp $
 ***************************************************************/

#include <QApplication>
#include "trayapp.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	TrayApp *trayApp = new TrayApp;
	return app.exec();
}

