/*****************************************************
 * MOPSLinux packaging system
 * Package manager UI - header
 * $Id: mainwindow.h,v 1.8 2007/02/21 11:51:10 i27249 Exp $
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
		void clearForm();
		void updateData();
		void markToInstall();
		void execMenu();
		void showPackageInfo();
		void fitTable();
	
	public:
		Ui::MainWindow ui;
		Ui::aboutBox _aboutBox;
		LoadingBox *loadBox;
		QMenu *tableMenu;
		QAction *installPackageAction;
		QAction *removePackageAction;
		QAction *purgePackageAction;
		QAction *upgradePackageAction;
		
		
		//Ui::loadingBox loadBox;
	private:
		void loadData(bool internal=false);
		mpkg *mDb;
		void initProgressBar(QProgressBar *Bar, int stepCount = 100);
		void setBarValue(QProgressBar *Bar, int stepValue);
		PACKAGE_LIST packagelist;
		void insertPackageIntoTable(unsigned int package_num);
		void searchPackagesByTag(QString tag);
		
		
		void initPackageTable();

};
#endif
