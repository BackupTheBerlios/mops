/****************************************************************
 *     MOPSLinux packaging system
 *     Package manager - main file
 *     $Id: main.cpp,v 1.22 2007/05/16 06:43:19 i27249 Exp $
 ***************************************************************/

#include <QApplication>
#include <QPushButton>
#include <QListWidget>
#include <QWidget>
#include "tablelabel.h"
#include "mainwindow.h"
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainWindow mw;
	
	QObject::connect(mw.ui.selectAllButton, SIGNAL(clicked()), &mw, SLOT(selectAll()));
	QObject::connect(mw.ui.deselectAllButton, SIGNAL(clicked()), &mw, SLOT(deselectAll()));
	QObject::connect(mw.ui.actionQuit, SIGNAL(triggered()), &mw, SLOT(quitApp()));
	QObject::connect(mw.ui.quitButton, SIGNAL(clicked()), &mw, SLOT(quitApp()));
	QObject::connect(mw.ui.applyButton, SIGNAL(clicked()), &mw, SLOT(commitChanges()));
	QObject::connect(mw.ui.actionAbout, SIGNAL(triggered()), &mw, SLOT(showAbout()));
	QObject::connect(mw.ui.actionReset_changes, SIGNAL(triggered()), &mw, SLOT(resetChanges()));
	QObject::connect(mw.ui.actionReset_all_queue, SIGNAL(triggered()), &mw, SLOT(resetQueue()));
	QObject::connect(mw.ui.actionPreferences, SIGNAL(triggered()), &mw, SLOT(showPreferences()));
	QObject::connect(mw.ui.actionAdd_remove_repositories, SIGNAL(triggered()), &mw, SLOT(showAddRemoveRepositories()));
	QObject::connect(mw.ui.actionClean_cache, SIGNAL(triggered()), &mw, SLOT(cleanCache()));
	QObject::connect(mw.ui.actionShow_installed, SIGNAL(triggered()), &mw, SLOT(applyPackageFilter()));
	QObject::connect(mw.ui.actionShow_available, SIGNAL(triggered()), &mw, SLOT(applyPackageFilter()));
	QObject::connect(mw.ui.actionShow_queue, SIGNAL(triggered()), &mw, SLOT(applyPackageFilter()));
	QObject::connect(mw.ui.actionShow_configexist, SIGNAL(triggered()), &mw, SLOT(applyPackageFilter()));
	QObject::connect(mw.ui.actionShow_unavailable, SIGNAL(triggered()), &mw, SLOT(applyPackageFilter()));
	QObject::connect(mw.ui.actionCore_settings, SIGNAL(triggered()), &mw, SLOT(showCoreSettings()));
	QObject::connect(mw.ui.actionUpdate_data, SIGNAL(triggered()), &mw, SLOT(updateData()));
	QObject::connect(mw.ui.packageTable, SIGNAL(itemSelectionChanged()), &mw, SLOT(showPackageInfo()));
	QObject::connect(mw.ui.quickPackageSearchEdit, SIGNAL(textEdited(const QString &)), &mw, SLOT(applyPackageFilter()));


	return app.exec();
}

