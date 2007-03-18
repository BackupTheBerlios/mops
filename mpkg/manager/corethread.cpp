/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.8 2007/03/18 05:43:37 i27249 Exp $
 * *************************************************************************/
#define USLEEP 5
#include "corethread.h"
coreThread::coreThread()
{
	printf("Core thread created\n");
	// Basic initializations here...
	database = new mpkg;	// Database initialization
}

coreThread::~coreThread()
{
	printf("Thread destroyed correctly\n");
	//delete database;
}

void coreThread::run()
{
	setPriority(QThread::LowestPriority);
	printf("Thread started\n");
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
	emit initProgressBar(count);
	emit enableProgressBar();
	emit clearTable();
	emit setTableSize(0);
	emit setTableSize(packageList->size());
	for (unsigned int i=0; i<packageList->size(); i++)
	{
		newStatus.push_back(packageList->get_package(i)->get_status());
		insertPackageIntoTable(i);
#ifdef USLEEP
		usleep(USLEEP);
#endif
#ifdef MSLEEP
		msleep(MSLEEP);
#endif
#ifdef SLEEP
		sleep(SLEEP);
#endif
		emit setProgressBarValue(i);
	}
	emit disableProgressBar();
	emit loadingFinished();
	emit fitTable();
	

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
/*	PACKAGE_LIST *tmpPackageList = new PACKAGE_LIST;
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
	emit initProgressBar(count);
	emit enableProgressBar();
	emit clearTable();
	emit setTableSize(0);
	emit setTableSize(packageList->size());
	for (unsigned int i=0; i<packageList->size(); i++)
	{
		newStatus.push_back(packageList->get_package(i)->get_status());
		insertPackageIntoTable(i);
		emit setProgressBarValue(i);
	}
	emit disableProgressBar();
	emit loadingFinished();
	emit fitTable();
*/	
}

void coreThread::insertPackageIntoTable(unsigned int package_num)
{
	bool checked;
	string package_icon;
	if (packageList->get_package(package_num)->get_vstatus().find("INSTALLED") != std::string::npos && \
			packageList->get_package(package_num)->get_vstatus().find("INSTALL") != std::string::npos)
	{
		checked = true;
	}

	PACKAGE *_p = packageList->get_package(package_num);
	switch(_p->get_status())
	{
		case PKGSTATUS_INSTALLED:
			package_icon = "installed.png";
			break;
		case PKGSTATUS_INSTALL:
			package_icon = "install.png";
			break;
		case PKGSTATUS_REMOVE:
			package_icon = "remove.png";
			break;
		case PKGSTATUS_PURGE:
			package_icon = "purge.png";
			break;
		case PKGSTATUS_REMOVED_AVAILABLE:
			package_icon = "removed_available.png";
			break;
		case PKGSTATUS_AVAILABLE:
			package_icon = "available.png";
			break;
		default:
			package_icon = "unknown.png";
	}
	string pName = "<table><tbody><tr><td><img src = \"icons/"+package_icon+"\"></img></td><td><b>"+_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
	
	emit setTableItem(package_num, checked, pName);
}

PACKAGE_LIST *coreThread::getPackageList()
{
	emit sendPackageList(*packageList);
}

void coreThread::updatePackageDatabase()
{
	emit loadingStarted();
	database->update_repository_data();
	emit loadingFinished();
	emit loadData();
}
