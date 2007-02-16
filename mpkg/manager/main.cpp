/****************************************************************
 *     MOPSLinux packaging system
 *     Package manager - main file
 *     $Id: main.cpp,v 1.3 2007/02/16 06:23:30 i27249 Exp $
 ***************************************************************/

#include <QApplication>
 #include <QPushButton>
 #include <QListWidget>
 #include <QWidget>
#include "mainwindow.h"
 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);

     MainWindow mw;

	mw.show();
	QObject::connect(mw.ui.actionQuit, SIGNAL(triggered()), &app, SLOT(quit()));
	QObject::connect(mw.ui.actionAbout, SIGNAL(triggered()), &mw, SLOT(showAbout()));

     return app.exec();
 } 
