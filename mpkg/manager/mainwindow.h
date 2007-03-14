/*****************************************************
 * MOPSLinux packaging system
 * Package manager UI - header
 * $Id: mainwindow.h,v 1.14 2007/03/14 02:22:17 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_pkgmanager.h"
#include "ui_aboutbox.h"
#include "ui_preferencesbox.h"
#include "ui_loading.h"
#include <mpkg/libmpkg.h>
#include "loading.h"
#include "db.h"
#include "corethread.h"
//#include "checkbox.h";
#include <QThread>
// TABLE INDEX
#define PT_INSTALLCHECK 0
#define PT_STATUS 1
#define PT_NAME 2
//#define PT_VERSION 3
//#define PT_ARCH 4
//#define PT_BUILD 5
//#define PT_MAXAVAILABLE 6
//#define PT_INFO 7
#define PT_ID 3



class MainWindow: public QMainWindow
{
	Q_OBJECT
	public:
		MainWindow (QMainWindow *parent = 0);
		~MainWindow();
		coreThread thread;
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
		void markChanges(int x, Qt::CheckState state);
		void quickPackageSearch();
		void showAllPackages();

		//void MainWindow::resetQueue();
	
	public:
		Ui::MainWindow ui;
		Ui::aboutBox _aboutBox;
		LoadingBox *loadBox;
		DatabaseBox *dbBox;
		QMenu *tableMenu;
		QAction *installPackageAction;
		QAction *removePackageAction;
		QAction *purgePackageAction;
		QAction *upgradePackageAction;
		
		
		//Ui::loadingBox loadBox;
		void loadData(bool internal=false);
	private:
		mpkg *mDb;
		void initProgressBar(QProgressBar *Bar, int stepCount = 100);
		void setBarValue(QProgressBar *Bar, int stepValue);
		PACKAGE_LIST packagelist;
		void insertPackageIntoTable(unsigned int package_num);
		void searchPackagesByTag(QString tag);
		vector<string> install_queue;
		vector<string> remove_queue;
		vector<string> purge_queue;
		vector<bool>stateChanged;
		vector<int>newStatus;
		
		void initPackageTable();

};

class CheckBox: public QCheckBox
{
	Q_OBJECT
	public:
		CheckBox(MainWindow *parent);
		//CheckBox(const QString & text, QWidget *parent = 0);
	public slots:
		void markChanges();
	public:
	int row;
	MainWindow *mw;
	
};


/*
class thLoadData: public QThread
{
	Q_OBJECT
	public:
		void run();
	signals:
		void packageAdded(const 
};
*/

#endif
