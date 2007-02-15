/****************************************************************
 *     MOPSLinux packaging system
 *     Package builder - main file
 *     $Id: main.cpp,v 1.3 2007/02/15 09:48:40 i27249 Exp $
 ***************************************************************/

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
	QObject::connect(mw.ui.DepDeleteButton, SIGNAL(clicked()), &mw, SLOT(deleteDependency()));
	QObject::connect(mw.ui.NameEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.VersionEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.BuildEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.ArchComboBox, SIGNAL(currentIndexChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));



     return app.exec();
 } 
