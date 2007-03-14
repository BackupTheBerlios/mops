/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.3 2007/03/14 09:15:11 i27249 Exp $
 * *************************************************************************/

#include "corethread.h"

coreThread::coreThread()
{
	printf("Core thread created\n");
	// Basic initializations here...
	//database = new mpkg;	// Database initialization
}

coreThread::~coreThread()
{
	printf("Thread destroyed correctly\n");
	//delete database;
}

void coreThread::run()
{
	printf("Thread started\n");
	// Just entering a loop to receive/emit signals...
	exec();
}

void coreThread::tellAreYouRunning()
{
	printf("Thread is running and received a signal\n");
	emit yesImRunning();
}

void coreThread::loadPackageDatabase()
{
	PACKAGE_LIST *tmpPackageList = new PACKAGE_LIST;
	emit loadingStarted();
	emit sqlQueryBegin();
	SQLRecord sqlSearch;
	vector<int> *tmpNewStatus;
	
	if (database->get_packagelist(sqlSearch, tmpPackageList, true)!=0)
	{
		emit errorLoadingDatabase();
		emit sqlQueryEnd();
		delete tmpPackageList;
		return;
	}
	packageList = tmpPackageList;
	emit sqlQueryEnd();
	
	newStatus.clear();

	unsigned int count = packageList->size();
	emit enableProgressBar();
	emit clearTable();
	emit setTableSize(0);
	for (unsigned int i=0; i<packageList->size(); i++)
	{
		newStatus.push_back(packageList->get_package(i)->get_status());
		//insertPackageIntoTable(i);
		emit setProgressBarValue(i);
	}
	emit disableProgressBar();
	emit loadingFinished();
	emit fitTable();
}
