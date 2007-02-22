/****************************************************************
 *     MOPSLinux packaging system
 *     Package manager - main file
 *     $Id: main.cpp,v 1.9 2007/02/22 14:32:24 i27249 Exp $
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

	QObject::connect(mw.ui.actionQuit, SIGNAL(triggered()), &mw, SLOT(quitApp()));
	 QObject::connect(mw.ui.quitButton, SIGNAL(clicked()), &mw, SLOT(quitApp()));
	 QObject::connect(mw.ui.applyButton, SIGNAL(clicked()), &mw, SLOT(commitChanges()));
	QObject::connect(mw.ui.actionAbout, SIGNAL(triggered()), &mw, SLOT(showAbout()));
	QObject::connect(mw.ui.actionReset_all_queue, SIGNAL(triggered()), &mw, SLOT(resetQueue()));
	QObject::connect(mw.ui.actionPreferences, SIGNAL(triggered()), &mw, SLOT(showPreferences()));
	QObject::connect(mw.ui.actionAdd_remove_repositories, SIGNAL(triggered()), &mw, SLOT(showAddRemoveRepositories()));
	QObject::connect(mw.ui.actionCore_settings, SIGNAL(triggered()), &mw, SLOT(showCoreSettings()));
	QObject::connect(mw.ui.actionCustom_filter, SIGNAL(triggered()), &mw, SLOT(showCustomFilter()));
	 QObject::connect(mw.ui.updateButton, SIGNAL(clicked()), &mw, SLOT(updateData()));
	 //QObject::connect(mw.ui.packageTable, SIGNAL(cell(int, int)), &mw, SLOT(markChanges(int, int)));
	 QObject::connect(mw.ui.packageTable, SIGNAL(itemSelectionChanged()), &mw, SLOT(showPackageInfo()));
     return app.exec();
 } 
