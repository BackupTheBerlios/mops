/****************************************************************
 *     MOPSLinux packaging system
 *     Package manager - main file
 *     $Id: main.cpp,v 1.2 2007/02/16 04:23:21 i27249 Exp $
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
	//QObject::connect(mw.ActionQuit(), SIGNAL(clicked()), &app, SLOT(quit()));

     return app.exec();
 } 
