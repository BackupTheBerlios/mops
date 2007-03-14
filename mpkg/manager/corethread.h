/******************************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.h,v 1.3 2007/03/14 09:15:11 i27249 Exp $
 *
 * This thread contains:
 * 1. Database object
 * 2. _ALL_ functions that directly uses database
 * 3. Creation of composite interface elements
 *
 * SIGNAL/SLOT model.
 *
 * ****************************************************************************************/
#ifndef CORETHREAD_H_
#define CORETHREAD_H_

#include <QThread>
#include <mpkg/libmpkg.h>
//#include "tablelabel.h"

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
		void enableProgressBar();
		void disableProgressBar();
		void setProgressBarValue(int value);
		
		
		// Table operations
		void fitTable();
		void clearTable();
		void setTableSize(unsigned int size);
		//SetTableItem(int row, bool checkState, TableLabel *cellItem);
		void setTableItemVisible(int row, bool visible);
		


	private:
		mpkg *database;
		PACKAGE_LIST *packageList;
		vector<int> newStatus;
};

#endif
