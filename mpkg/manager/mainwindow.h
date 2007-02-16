/*****************************************************
 * MOPSLinux packaging system
 * Package manager UI - header
 * $Id: mainwindow.h,v 1.2 2007/02/16 06:23:30 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_pkgmanager.h"
#include "ui_aboutbox.h"

class MainWindow: public QMainWindow
{
	Q_OBJECT
	public:
		MainWindow (QMainWindow *parent = 0);
	public slots:
		void showPreferences();
		void showAbout();
		void quitApp();
		void showCoreSettings();
		void commitChanges();
		void resetChanges();
		void resetQueue();
		void saveQueue();
		void showAddRemoveRepositories();
		void setInstalledFilter(bool showThis=false);
		void setAvailableFilter(bool showThis=false);
		void setBrokenFilter(bool showThis=false);
		void setUnavailableFilter(bool showThis=false);
		void setRemovedFilter(bool showThis=false);
		void showHelpTopics();
		void showFaq();
	
	public:
		Ui::MainWindow ui;
		Ui::aboutBox _aboutBox;
	private:
		void loadData();
};
#endif
