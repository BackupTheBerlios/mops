/******************************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.h,v 1.9 2007/03/21 15:30:14 i27249 Exp $
 *
 * This thread contains:
 * 1. Database object
 * 2. _ALL_ functions that directly uses database
 * 3. Creation of composite interface elements
 *
 * SIGNAL/SLOT model.
 *
 * ****************************************************************************************/
#ifndef CZORETHREAD_H_
#define CZORETHREAD_H_

//#include <mpkg/libmpkg.h>
//#include <QLabel>
//#include <QTableWidget>

#include "ui_pkgmanager.h"
//#include "tablelabel.h"
//#include "mainwindow.h"
#include <mpkg/libmpkg.h>

#include <QThread>

#define STT_Run 0
#define STT_Pause 1
#define STT_Stop 2

#define CA_Idle 0
#define CA_LoadDatabase 1
#define CA_CommitQueue 2
#define CA_Quit 3
#define CA_UpdateDatabase 4

class statusThread: public QThread
{
	Q_OBJECT
	public:
		void run();
		statusThread();
		int action;
	public slots:
		void show();
		void hide();
		void halt();
	signals:
		void setStatus(QString status);
};

class coreThread: public QThread
{
	Q_OBJECT
	public:
		void run();
		void sync();
		coreThread();
		~coreThread();

	public slots:
		void callQuit();
		void updatePackageDatabase(); 	// Call to update repositories data
		void loadPackageDatabase();	// Call to load data from database and display
		void commitQueue(vector<int> nStatus);		// Call to commit actions (install, remove, etc)
		void syncData();		// Call to sync data between GUI and thread (temporary solution)
		void tellAreYouRunning();	// Debug call: prints "yes i'm running" to console
	private:
		void _loadPackageDatabase();	// loading data from database - real implementation
		void _updatePackageDatabase();	// updating data from repositories - real implementation
		void insertPackageIntoTable(unsigned int package_num); // Displaying function
		void _commitQueue();


	signals:
		void setStatus(QString msg);
		void loadData();
		// Debug signals
		void yesImRunning();
		
		// Errors and status messages
		void errorLoadingDatabase();
		void sqlQueryBegin();
		void sqlQueryEnd();
		void loadingStarted();
		void loadingFinished();
		
		// Progress bar
		void initProgressBar(unsigned int stepCount);
		void enableProgressBar();
		void disableProgressBar();
		void setProgressBarValue(unsigned int value);
		
		
		// Table operations
		void fitTable();
		void clearTable();
		void setTableSize(unsigned int size);
		void setTableItem(unsigned int row, bool checkState, string cellItemText);
		void setTableItemVisible(unsigned int row, bool visible);

		// Data sync
		void sendPackageList(PACKAGE_LIST pkgList, vector<int> nStatus);
		
	public slots:
		PACKAGE_LIST * getPackageList();

	private:
		int currentAction;
		mpkg *database;
		PACKAGE_LIST *packageList;
		vector<int> newStatus;
};

#endif
