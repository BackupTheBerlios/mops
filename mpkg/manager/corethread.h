/******************************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.h,v 1.2 2007/03/14 02:22:17 i27249 Exp $
 *
 * This thread contains:
 * 1. Database object
 * 2. _ALL_ functions that directly uses database
 * 3. Creation of composite interface elements
 *
 * SIGNAL/SLOT model.
 *
 * ****************************************************************************************/
#include <QThread>
//#include <QtGui>
#include <mpkg/libmpkg.h>
// ACTION DEFINERS
#define CTH_LOAD_PACKAGE_LIST 0x01


class coreThread: public QThread
{
	Q_OBJECT
	public:
		void run();
		coreThread();
		~coreThread();

	public slots:
		void tellAreYouRunning();
		//queryPackageDatabase();
	signals:
		void yesImRunning();
		//loadingStarted();
		//loadingFinished();
		//enableProgressBar();
		//disableProgressBar();
		//setProgressBarValue(int);
		//SetTableItem(int, bool, TableLabel &);
		//setTableItemVisible(int, bool);
		


	private:
		//unsigned int actionRequested;
		//loadPackageList();
		mpkg *database;
		//PACKAGE_LIST *packageList;
};
