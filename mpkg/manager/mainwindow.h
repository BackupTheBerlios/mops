/*****************************************************
 * MOPSLinux packaging system
 * Package manager UI - header
 * $Id: mainwindow.h,v 1.38 2007/05/10 02:39:08 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_pkgmanager.h"
#include "ui_aboutbox.h"
#include "ui_preferencesbox.h"
#include "preferencesbox.h"
#include "ui_loading.h"
#include <mpkg/libmpkg.h>
#include "loading.h"
#include "db.h"
#include "corethread.h"

//#include "progress.h"
//#include "checkbox.h";
#include <QThread>
// TABLE INDEX
#define PT_INSTALLCHECK 0
//#define PT_STATUS 1
#define PT_NAME 1
//#define PT_VERSION 3
//#define PT_ARCH 4
//#define PT_BUILD 5
//#define PT_MAXAVAILABLE 6
//#define PT_INFO 7
#define PT_ID 2


class MainWindow: public QMainWindow
{
	Q_OBJECT
	public:
		double installQueueSize;
		bool initializeOk;
		MainWindow (QMainWindow *parent = 0);
		~MainWindow();
		coreThread *thread;
		statusThread *StatusThread;
		errorBus *ErrorBus;
		PreferencesBox *prefBox;

	signals:
		void redrawReady(bool flag);
		void loadPackageDatabase();
		void startThread();
		void startStatusThread();
		void startErrorBus();
		void sendUserReply(QMessageBox::StandardButton reply);
		void syncData();
		void updateDatabase();
		void quitThread();
		void callCleanCache();
		void commit(vector<int> nStatus);
	public slots:
		void resetProgressBar();
		void setInitOk(bool flag);
		void updateProgressData();
		void showProgressWindow(bool flag);
		void hideEntireTable();
		void showErrorMessage(QString headerText, QString bodyText, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);
		void initCategories();
		void filterCategory(int category_id);
		void applyPackageFilter();
		void setTableSize();
		void filterCloneItems();

		void receivePackageList(PACKAGE_LIST pkgList, vector<int> nStatus);
		void setStatus(QString status);
		void loadData();
		void errorLoadingDatabase();
		void sqlQueryBegin();
		void sqlQueryEnd();
		void loadingStarted();
		void loadingFinished();
		
		// Progress bar
		void enableProgressBar();
		void disableProgressBar();
		void setProgressBarValue(unsigned int value);
		void initProgressBar(unsigned int stepCount = 100);
		
		void enableProgressBar2();
		void disableProgressBar2();
		void setProgressBarValue2(unsigned int value);
		void initProgressBar2(unsigned int stepCount = 100);



		
		// Table operations
		//void fitTable();
		void clearTable();
		void selectAll();
		void deselectAll();
		void setTableSize(unsigned int size);
		void setTableItem(unsigned int row, bool checkState, string cellItemText);
		void setTableItemVisible(unsigned int row, bool visible);

//

		void showPreferences();
		void showAbout();
		void quitApp();
		void showCoreSettings();
		void commitChanges();
		//void resetChanges();
		void resetQueue();
		void cleanCache();
		//void saveQueue();
		void showAddRemoveRepositories();
		//void showCustomFilter();
		void setInstalledFilter();
		//void setAvailableFilter(bool showThis=false);
		//void setBrokenFilter(bool showThis=false);
		//void setUnavailableFilter(bool showThis=false);
		//void setRemovedFilter(bool showThis=false);
		//void showHelpTopics();
		//void showFaq();
		void clearForm();
		void updateData();
		void markToInstall();
		void execMenu();
		void showPackageInfo();
		//void fitTable();
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
		XMLNode _categories;
		
		//Ui::loadingBox loadBox;
	private:
		//QLabel *indicator;
		QMovie *movie; 
		mpkg *mDb;
		void setBarValue(QProgressBar *Bar, int stepValue);
		PACKAGE_LIST *packagelist;
		//void insertPackageIntoTable(unsigned int package_num);
		//void searchPackagesByTag(QString tag);
		vector<string> install_queue;
		vector<string> remove_queue;
		vector<string> purge_queue;
		vector<bool>stateChanged;
		vector<int>newStatus;
		void initPackageTable();
		int currentCategoryID;

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
