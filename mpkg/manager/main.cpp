/****************************************************************
 *     MOPSLinux packaging system
 *     Package manager - main file
 *     $Id: main.cpp,v 1.1 2007/02/16 01:36:36 i27249 Exp $
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

     return app.exec();
 } 
