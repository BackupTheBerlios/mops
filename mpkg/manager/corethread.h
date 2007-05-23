/******************************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.h,v 1.25 2007/05/23 18:02:18 i27249 Exp $
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
#include <QMessageBox>
//#include "tablelabel.h"
//#include "mainwindow.h"
#include <mpkg/libmpkg.h>

#include <QThread>

#define eBUS_Run 0
#define eBUS_Pause 1
#define eBUS_Stop 2

#define STT_Run 0
#define STT_Pause 1
#define STT_Stop 2

#define CA_Idle 0
#define CA_LoadDatabase 1
#define CA_CommitQueue 2
#define CA_Quit 3
#define CA_UpdateDatabase 4
#define CA_GetCdromName 5
#define CA_GetAvailableTags 6
#define CA_GetRequiredPackages 7
#define CA_GetDependantPackages 8
// Default group definitions



class errorBus: public QThread
{
	Q_OBJECT
	public:
		errorBus();
		int action;
		void run();

	public slots:
		void Start();
		void Pause();
		void Stop();
		void receiveUserReply(QMessageBox::StandardButton reply);

	signals:
		void sendErrorMessage(QString headerText, QString bodyText, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

	private:

		unsigned int TIMER_RES;
		QMessageBox::StandardButton userReply;
		string txt;
		unsigned long idleTime;
		unsigned long idleThreshold;

};

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
		void setPDataActive(bool flag);
		void recvRedrawReady(bool flag);
	signals:
		void setIdleButtons(bool flag);
		void setSkipButton(bool flag);
		void showProgressWindow(bool flag);
		void setStatus(QString status);
		void loadProgressData();
		void enableProgressBar();
		void disableProgressBar();
		void initProgressBar(unsigned int maxvalue);
		void setBarValue(unsigned int value);
		void enableProgressBar2();
		void disableProgressBar2();
		void initProgressBar2(unsigned int maxvalue);
		void setBarValue2(unsigned int value);

	private:
		bool redrawReady;
		unsigned long idleTime;
		unsigned long idleThreshold;
		bool pDataActive;

		unsigned int TIMER_RES;
		bool enabledBar;
		bool enabledBar2;
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
		void getRequiredPackages(unsigned int package_num);
		void getDependantPackages(unsigned int package_num);
		void getAvailableTags();
		void cleanCache();
		void callQuit();
		void updatePackageDatabase(); 	// Call to update repositories data
		void loadPackageDatabase();	// Call to load data from database and display
		void commitQueue(vector<int> nStatus);		// Call to commit actions (install, remove, etc)
		void tellAreYouRunning();	// Debug call: prints "yes i'm running" to console
		void getCdromName();
	private:
		unsigned long idleTime;
		unsigned long idleThreshold;

		unsigned int TIMER_RES;
		void _getRequiredPackages();
		void _getDependantPackages();
		void _loadPackageDatabase();	// loading data from database - real implementation
		void _updatePackageDatabase();	// updating data from repositories - real implementation
		void insertPackageIntoTable(unsigned int package_num); // Displaying function
		void _getCdromName();
		void _commitQueue();
		void _getAvailableTags();


	signals:
		void sendRequiredPackages(unsigned int package_num, PACKAGE_LIST req);
		void sendDependantPackages(unsigned int package_num, PACKAGE_LIST dep);
		void showMessageBox(QString header, QString text);
		void resetProgressBar();
		void initState(bool flag);
		void sendCdromName(string volname);
		void applyFilters();
		void setStatus(QString msg);
		void loadData();
		void sendAvailableTags(vector<string> output);
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
		unsigned int packageProcessingNumber;
		int currentAction;
		mpkg *database;
		PACKAGE_LIST *packageList;
		vector<int> newStatus;
};

#endif
