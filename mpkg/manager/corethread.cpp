/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.11 2007/03/21 14:30:10 i27249 Exp $
 * *************************************************************************/
#define USLEEP 15
#include "corethread.h"
coreThread::coreThread()
{
	printf("Core thread created\n");
	//packageList = pkgList;
	// Basic initializations here...
	database = new mpkg;	// Database initialization
	currentAction = CA_Idle;
}

coreThread::~coreThread()
{
	delete database;
	printf("Thread destroyed correctly\n");
	//delete database;
}

void coreThread::callQuit()
{
	currentAction = CA_Quit;
}

void coreThread::sync()
{
	emit sendPackageList(*packageList, newStatus);
}

void coreThread::run()
{
	printf("Running...\n");
	forever 
	{
		switch(currentAction)
		{
			case CA_LoadDatabase:
				_loadPackageDatabase();
				sync();
				break;

			case CA_CommitQueue:
				_commitQueue();
				sync();
				break;
			case CA_UpdateDatabase:
				_updatePackageDatabase();
				sync();
				break;

			case CA_Idle:
				msleep(50);
				break;
			case CA_Quit:
				delete database;
				printf("Called quit!\n");
				if (database)
				{
					printf("Seems that database still open...\n");
				}
				return; // Exiting!
			default:
				printf("Out of loop! WARNING!!!\n");
				msleep(10);
		}
	}
				
				
				
				
				
				
				
	/*			setPriority(QThread::LowestPriority);
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
					currentAction = CA_None;
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
	*/

	// Just entering a loop to receive/emit signals...
}

void coreThread::tellAreYouRunning()
{
	printf("Thread is running and received a signal\n");
	emit yesImRunning();
}

void coreThread::loadPackageDatabase()
{
	currentAction = CA_LoadDatabase;
}

void coreThread::_loadPackageDatabase()
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
	currentAction=CA_Idle;
}

void coreThread::insertPackageIntoTable(unsigned int package_num)
{
	bool checked = false;
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
	emit sendPackageList(*packageList, newStatus);
}

void coreThread::_updatePackageDatabase()
{
	emit loadingStarted();
	database->update_repository_data();
	emit loadingFinished();
	emit loadData();
}

void coreThread::updatePackageDatabase()
{
	currentAction = CA_UpdateDatabase;
}

void coreThread::commitQueue(vector<int> nStatus)
{
	newStatus = nStatus;
	currentAction = CA_CommitQueue;
}
void coreThread::_commitQueue()
{
	vector<string> install_queue;
	vector<string> remove_queue;
	vector<string> purge_queue;
	for (unsigned int i = 0; i< newStatus.size(); i++)
	{
		if (packageList->get_package(i)->get_status()!=newStatus[i])
		{
			switch(newStatus[i])
			{
				case PKGSTATUS_INSTALL:
					install_queue.push_back(packageList->get_package(i)->get_name());
					break;
				case PKGSTATUS_REMOVE:
					remove_queue.push_back(packageList->get_package(i)->get_name());
					break;
				case PKGSTATUS_PURGE:
					purge_queue.push_back(packageList->get_package(i)->get_name());
					break;
				default:
					printf("Unknown status %d\n", newStatus[i]);
			}
		}
	}
	printf("install_queue size = %d\n", install_queue.size());
	printf("remove_queue size = %d\n", remove_queue.size());
	database->uninstall(remove_queue);
	database->install(install_queue);
	database->purge(purge_queue);
	emit setStatus("Committing changes...");
	database->commit();
	emit setStatus("All operations completed");
	currentAction = CA_LoadDatabase;
}

void coreThread::syncData()
{
}
