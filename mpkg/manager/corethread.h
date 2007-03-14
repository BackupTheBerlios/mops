/******************************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.h,v 1.1 2007/03/14 01:50:24 i27249 Exp $
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
#include <QtGui>
#include <mpkg/libmpkg.h>

class coreThread: public QThread
{
	Q_OBJECT
	public:
		void run();
		coreThread();
		~coreThread();

	public slots:
		queryPackageDatabase();
	signals:
		loadingStarted();
		loadingFinished();
		enableProgressBar();
		disableProgressBar();
		setProgressBarValue(int);
		SetTableItem(int, bool, TableLabel &);
		setTableItemVisible(int, bool);
		


	private:
		mpkg *databaseLib;
		PACKAGE_LIST *packageList;
};
