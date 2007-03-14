/******************************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.h,v 1.4 2007/03/14 13:28:18 i27249 Exp $
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
class coreThread: public QThread
{
	Q_OBJECT
	public:
		void run();
		coreThread();
		~coreThread();

	public slots:
		void tellAreYouRunning();
		void loadPackageDatabase();
		void insertPackageIntoTable(unsigned int package_num);
	signals:
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
		


	private:
		mpkg *database;
		PACKAGE_LIST *packageList;
		vector<int> newStatus;
};

#endif
