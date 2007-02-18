/*****************************************************
 * MOPSLinux packaging system
 * Package manager UI - header
 * $Id: mainwindow.h,v 1.4 2007/02/18 03:10:34 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_pkgmanager.h"
#include "ui_aboutbox.h"
#include "ui_preferencesbox.h"
#include "ui_loading.h"
#include <mpkg/libmpkg.h>
#include "loading.h"
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
		void showCustomFilter();
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
		LoadingBox *loadBox;
		//Ui::loadingBox loadBox;
	private:
		void loadData();
		mpkg *mDb;
		
		PACKAGE_LIST packagelist;

};
#endif
