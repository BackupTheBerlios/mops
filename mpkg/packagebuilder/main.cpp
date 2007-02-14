/*     Mockup for QT face of mpkg
 *     $Id: main.cpp,v 1.1 2007/02/14 14:33:58 i27249 Exp $
 *     */

#include <QApplication>
 #include <QPushButton>
 #include <QListWidget>
 #include <QWidget>
#include "mainwindow.h"
 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);

     Form mw;
	mw.show();
	QObject::connect(mw.ui.LoadButton, SIGNAL(clicked()), &mw, SLOT(loadData()));
	QObject::connect(mw.ui.TagAddButton, SIGNAL(clicked()), &mw, SLOT(addTag()));
	QObject::connect(mw.ui.TagDeleteButton, SIGNAL(clicked()), &mw, SLOT(deleteTag()));
	QObject::connect(mw.ui.DepAddButton, SIGNAL(clicked()), &mw, SLOT(addDependency()));
	QObject::connect(mw.ui.BuildButton, SIGNAL(clicked()), &mw, SLOT(saveData()));


     return app.exec();
 } 
