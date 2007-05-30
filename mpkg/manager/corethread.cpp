/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.64 2007/05/30 14:29:08 i27249 Exp $
 * *************************************************************************/
#include "corethread.h"

coreThread::coreThread()
{
	readyState=true;
	packageList = new PACKAGE_LIST;
	TIMER_RES=50;
	idleTime=0;
	idleThreshold=40;
	database = new mpkg;
	currentAction = CA_Idle;
}

coreThread::~coreThread()
{
	delete database;
	delete packageList;
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
	forever 
	{
		if (currentAction == CA_Idle)
		{
			if (idleTime>idleThreshold) TIMER_RES = IDLE_RES;
			else idleTime++;
		}
		else
		{
			idleTime=0;
			TIMER_RES=RUNNING_RES;
		}
		switch(currentAction)
		{
			case CA_LoadDatabase:
				emit initState(false);
				_loadPackageData();
				_loadPackageDatabase();
				sync();
				break;
			case CA_LoadItems:
				_loadPackageDatabase();
				break;
			case CA_CommitQueue:
				_commitQueue();
				sync();
				break;
			case CA_UpdateDatabase:
				_updatePackageDatabase();
				sync();
				break;

			case CA_GetCdromName:
				_getCdromName();
				sync();
				break;

			case CA_GetAvailableTags:
				_getAvailableTags();
				break;

			case CA_GetRequiredPackages:
				_getRequiredPackages();
				break;

			case CA_GetDependantPackages:
				_getDependantPackages();
				break;

			case CA_Idle:
				emit setStatus(database->current_status().c_str());
				msleep(TIMER_RES);
				break;
			case CA_Quit:
				delete database;
				return; // Exiting!
			default:
				msleep(100);
		}
	}
}

void coreThread::tellAreYouRunning()
{
	emit yesImRunning();
}

void coreThread::loadItems(vector<bool> _showMask)
{
	showMask=_showMask;
	//_loadPackageDatabase();
	currentAction = CA_LoadItems;
	
}

void coreThread::loadPackageDatabase()
{
	showMask.clear();
	currentAction = CA_LoadDatabase;
}

void coreThread::getAvailableTags()
{
	_getAvailableTags();
}

void coreThread::_getAvailableTags()
{
	vector<string> output;
	database->get_available_tags(&output);
	emit sendAvailableTags(output);
	currentAction=CA_Idle;
}
void coreThread::_loadPackageData()
{

	actionBus.clear();
	actionBus.addAction(ACTIONID_GETPKGLIST);
	pData.clear();

	emit resetProgressBar();
	currentStatus = tr("Loading package database").toStdString();
	emit loadingStarted();
	PACKAGE_LIST *tmpPackageList = new PACKAGE_LIST;
	SQLRecord sqlSearch;
	if (database->get_packagelist(&sqlSearch, tmpPackageList)!=0)
	{
		mError("error returned from get_packagelist\n");
		emit errorLoadingDatabase();
		emit sqlQueryEnd();
		delete tmpPackageList;
		return;
	}
	delete packageList;
	currentStatus = tr("Building version list").toStdString();

	tmpPackageList->initVersioning();
	actionBus.setActionState(ACTIONID_GETPKGLIST);
	packageList = tmpPackageList;
	newStatus.clear();
	for (int i=0; i<packageList->size(); i++)
	{
		newStatus.push_back(packageList->get_package(i)->action());
	}
	sync();
	currentStatus = tr("Preparing dependency engine").toStdString();
	database->DepTracker->renderData();
}

void coreThread::_loadPackageDatabase()
{
	actionBus.clear();
	actionBus.addAction(ACTIONID_DBLOADING);
	actionBus.setCurrentAction(ACTIONID_DBLOADING);
	while (!readyState) usleep(1);
	readyState=false;
	emit clearTable();
	emit setTableSize(0);
	int tableSize=0;// = packageList->size();
	if (!showMask.empty())
	{
		tableSize = 0;
		for (unsigned int i=0; i<showMask.size(); i++)
		{
			if (showMask[i]) tableSize++;
		}
	}
	emit setTableSize(tableSize);

	actionBus.setActionProgressMaximum(ACTIONID_DBLOADING, tableSize);
	currentStatus = tr("Loading packages into table").toStdString();
	unsigned int tPos=0;
	if (!showMask.empty())
	{
		for (int i=0; i<packageList->size(); i++)
		{
			if (showMask.empty() || showMask[i])
			{
				insertPackageIntoTable(tPos, i);
				usleep(USLEEP);
				tPos++;
				actionBus.setActionProgress(ACTIONID_DBLOADING, tPos);
			}
			if (!actionBus.idle() && actionBus._abortActions)
			{
				readyState=true;
				actionBus.setActionState(ACTIONID_DBLOADING, ITEMSTATE_ABORTED);

				break;
			}
		}
	}

	emit disableProgressBar();
	emit loadingFinished();
	emit initState(true);
	emit applyFilters();
	currentStatus = tr("Loading finished").toStdString();
	currentAction=CA_Idle;
	actionBus.clear();
	pData.clear();
	
	actionBus._abortComplete = true;
}

void coreThread::getCdromName()
{
	currentAction = CA_GetCdromName;
}

void coreThread::_getCdromName()
{
	currentStatus = "Checking CD-ROM media...";
	string volname = getCdromVolname();
	emit sendCdromName(volname);
	currentAction = CA_Idle;
}


void coreThread::insertPackageIntoTable(int tablePos, unsigned int package_num)
{
	currentStatus = tr("Loading packages into table: ").toStdString()+IntToStr(package_num);

	bool checked = false;
	string package_icon;
	if (packageList->get_package(package_num)->action()==ST_INSTALL || \
			packageList->get_package(package_num)->installed())
	{
		checked = true;
	}

	PACKAGE *_p = packageList->get_package(package_num);
	string cloneHeader;
	if (_p->isUpdate()) cloneHeader = "<b><font color=\"red\">["+tr("update").toStdString()+"]</font></b>";
	
	switch (_p->action())
	{
		case ST_NONE:
			if (_p->installed()) package_icon = "installed.png";
			else
			{
				if (_p->available()){
				       if (_p->isUpdate()) package_icon = "update.png";
				       else {
					       package_icon="available.png";
					       if (_p->configexist()) package_icon="removed_available.png";
				       }
				}
				else package_icon="unknown.png";
			}
			break;
		case ST_INSTALL:
			package_icon="install.png";
			break;
		case ST_UPDATE:
		case ST_REMOVE:
			package_icon="remove.png";
			break;
		case ST_PURGE:
			package_icon="purge.png";
			break;
	}
	if (_p->deprecated()) package_icon = (string) "deprecated_" + package_icon;
		string pName = "<table><tbody><tr><td><img src = \"/usr/share/mpkg/icons/"+package_icon+"\"></img></td><td><b>"+ *_p->get_name()+"</b> "\
			+_p->get_fullversion()\
			+" <font color=\"green\"> \t["+humanizeSize(*_p->get_compressed_size()) + "]     </font>" + cloneHeader+\
		       	+ "<br>"+*_p->get_short_description() + "</td></tr></tbody></table>";
	//while (!readyState) usleep(1);
	//readyState = false;
	packageList->setTableID(package_num, tablePos);
	emit setTableItem(tablePos, package_num, checked, pName);
}
void coreThread::recvReadyFlag()
{
	readyState = true;
}
PACKAGE_LIST *coreThread::getPackageList()
{
	emit sendPackageList(*packageList, newStatus);
	return packageList;
}

void coreThread::_updatePackageDatabase()
{
	currentStatus = tr("Updating package database from repositories...").toStdString();
	emit loadingStarted();
	database->update_repository_data();
	currentAction = CA_LoadDatabase;
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
	currentStatus = tr("Committing...").toStdString();
	PACKAGE_LIST install_queue;
	vector<string> remove_queue;
	vector<string> purge_queue;
	vector<int> reset_queue;
	for (unsigned int i = 0; i< newStatus.size(); i++)
	{
		if (packageList->get_package(i)->action()!=newStatus[i])
		{
			switch(newStatus[i])
			{
				case ST_NONE:
					reset_queue.push_back(packageList->get_package(i)->get_id());
					break;
				case ST_INSTALL:
					install_queue.add(packageList->get_package(i));
					break;
				case ST_UPDATE:
				case ST_REMOVE:
					remove_queue.push_back(*packageList->get_package(i)->get_name());
					break;
				case ST_PURGE:
					purge_queue.push_back(*packageList->get_package(i)->get_name());
					break;
				default:
					mError((string) __func__ + "Unknown action " + IntToStr(newStatus[i]));
			}
		}
	}
	database->uninstall(remove_queue);
	database->install(&install_queue);
	database->purge(purge_queue);
	for (unsigned int i = 0; i<reset_queue.size(); i++)
	{
		database->unqueue(reset_queue[i]);
	}
	currentStatus = tr("Committing changes...").toStdString();
	int ret = database->commit();

	if (ret==0) currentStatus = tr("All operations completed successfully").toStdString();
	delete database;
	database = new mpkg;
	currentAction = CA_LoadDatabase;
	actionBus.clear();
	pData.clear();
	QString body;
	switch(ret)
	{
		case 0:
			body = tr("All operations completed successfully");
			break;
		case MPKGERROR_UNRESOLVEDDEPS:
			body = tr("Operations completed with errors: unresolved dependencies");
			break;
		case MPKGERROR_SQLQUERYERROR:
			body = tr("Error while processing operations: internal SQL error");
			break;
		case MPKGERROR_ABORTED:
			body = tr("Processing aborted");
			break;
		default:
			body = tr("Operations completed with errors");
			break;
	}
	
	emit showMessageBox(tr("All operations completed"), body);
}

void coreThread::cleanCache()
{
	currentStatus = tr("Cleaning package cache").toStdString();
	database->clean_cache();
	currentStatus = tr("Cleanup complete").toStdString();
	sleep(1);
	emit loadingFinished();
}

// Realtime dependency tracking

void coreThread::getRequiredPackages(unsigned int package_num)
{
	packageProcessingNumber = package_num;
	currentAction = CA_GetRequiredPackages;
}

void coreThread::getDependantPackages(unsigned int package_num)
{
	packageProcessingNumber = package_num;
	currentAction = CA_GetDependantPackages;
}
void coreThread::_getRequiredPackages()
{
	PACKAGE_LIST query;
	query.add(packageList->get_package(packageProcessingNumber));
	PACKAGE_LIST req = database->DepTracker->renderRequiredList(&query);
	filterDupes(&req);
	emit sendRequiredPackages(packageProcessingNumber, req);
	currentAction = CA_Idle;
}

void coreThread::_getDependantPackages()
{
	PACKAGE_LIST query;
	query.add(packageList->get_package(packageProcessingNumber));
	PACKAGE_LIST dep = database->DepTracker->renderRemoveQueue(&query);
	filterDupes(&dep);
	emit sendDependantPackages(packageProcessingNumber, dep);
	currentAction = CA_Idle;
}

