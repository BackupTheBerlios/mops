/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.62 2007/05/28 14:17:19 i27249 Exp $
 * *************************************************************************/
#define USLEEP 5
#include "corethread.h"
//#define TIMER_RES 600
#define IDLE_RES 600
#define RUNNING_RES 200
errorBus::errorBus()
{
	action = eBUS_Pause;
	TIMER_RES = 400;
}

void errorBus::run()
{

	//setPriority(QThread::LowestPriority);
	forever
	{
		if (action == eBUS_Run)
		{
			if (getErrorCode()!=MPKG_OK)
			{
				switch(getErrorCode())
				{
					//-------- PACKAGE DOWNLOAD ERRORS ---------//

					case MPKG_DOWNLOAD_OK:
						setErrorReturn(MPKG_RETURN_CONTINUE);
						break;
					case MPKG_DOWNLOAD_TIMEOUT:
					case MPKG_DOWNLOAD_MD5:
					case MPKG_DOWNLOAD_HOST_NOT_FOUND:
					case MPKG_DOWNLOAD_FILE_NOT_FOUND:
					case MPKG_DOWNLOAD_LOGIN_INCORRECT:
					case MPKG_DOWNLOAD_FORBIDDEN:
					case MPKG_DOWNLOAD_OUT_OF_SPACE:
					case MPKG_DOWNLOAD_WRITE_ERROR:

					case MPKG_DOWNLOAD_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Download error"),tr("Some files failed to download. What to do?"),
								QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore, 
								QMessageBox::Retry);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Retry:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							case QMessageBox::Ignore:
								setErrorReturn(MPKG_RETURN_IGNORE);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					//---------------CD-ROM ERRORS/EVENTS---------------------//
					case MPKG_CDROM_WRONG_VOLNAME:
					case MPKG_CDROM_MOUNT_ERROR:
						userReply = QMessageBox::NoButton;
						txt = tr("Please insert disk with label ").toStdString()+CDROM_VOLUMELABEL+tr(" into ").toStdString()+CDROM_DEVICENAME;
						emit sendErrorMessage(tr("Please insert a disk"), \
								txt.c_str(), QMessageBox::Ok | QMessageBox::Abort, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Ok:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							default:
								setErrorReturn(MPKG_RETURN_ABORT);
						}
						break;


					//-------- INDEX ERRORS ---------------------------//
					
					case MPKG_INDEX_DOWNLOAD_TIMEOUT:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Download error"),tr("Unable to download repository index. WTF?"),
								QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore, 
								QMessageBox::Retry);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Retry:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							case QMessageBox::Ignore:
								setErrorReturn(MPKG_RETURN_IGNORE);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_INDEX_PARSE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Parse error"),tr("Error parsing repository index!"),
								QMessageBox::Ok, 
								QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Ok:
								setErrorReturn(MPKG_RETURN_SKIP);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							case QMessageBox::Ignore:
								setErrorReturn(MPKG_RETURN_IGNORE);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_INDEX_HOST_NOT_FOUND:
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INDEX_NOT_RECOGNIZED:
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INDEX_LOGIN_INCORRECT:
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INDEX_FORBIDDEN:
						setErrorReturn(MPKG_RETURN_SKIP);
					case MPKG_INDEX_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Repository index error"),tr("Error retrieving repository index!"),
								QMessageBox::Ok, 
								QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Ok:
								setErrorReturn(MPKG_RETURN_SKIP);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;



					//---------- INSTALLATION ERRORS --------------//
					case MPKG_INSTALL_OUT_OF_SPACE:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Out of space!"), tr("Error installing packages - out of space.\nFree some disk space and try again"), QMessageBox::Retry | QMessageBox::Abort, QMessageBox::Retry);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Retry:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_INSTALL_SCRIPT_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Script error"), tr("Error executing script"), QMessageBox::Ok, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INSTALL_EXTRACT_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Package extraction error"), tr("Error extracting package."), QMessageBox::Ok, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						setErrorReturn(MPKG_RETURN_SKIP);
						break;

					case MPKG_INSTALL_META_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Error extracting metadata"), tr("Error while extracting metadata from package. Seems that package is broken"), QMessageBox::Ok, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						setErrorReturn(MPKG_RETURN_SKIP);
						break;

					case MPKG_INSTALL_FILE_CONFLICT:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("File conflict detected"), tr("Unresolvable file conflict detected. You can force installation, but it is DANGEROUS (it may broke some components)"), QMessageBox::Ignore | QMessageBox::Abort, QMessageBox::Ignore);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Ignore:
								setErrorReturn(MPKG_RETURN_IGNORE);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;



					//---------STARTUP ERRORS---------------//
					case MPKG_STARTUP_COMPONENT_NOT_FOUND:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Some components not found!"), tr("Some components were not found, the program can fail during runtime. Continue?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Yes:
								setErrorReturn(MPKG_RETURN_CONTINUE);
								break;
							case QMessageBox::No:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
						}
						userReply = QMessageBox::NoButton;
						break;
					case MPKG_STARTUP_NOT_ROOT:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Access denied"), tr("You should run this program as root"), QMessageBox::Abort, QMessageBox::Abort);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						setErrorReturn(MPKG_RETURN_ABORT);
						break;

					
					//---------- SUBSYSTEM ERRORS ---------------------//
					case MPKG_SUBSYS_SQLDB_INCORRECT:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("SQL database error"),tr("Incorrect database structure. Create from scratch?"),
								QMessageBox::Yes | QMessageBox::No, 
								QMessageBox::No);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Yes:
								setErrorReturn(MPKG_RETURN_REINIT);
								break;
							case QMessageBox::No:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_SQLDB_OPEN_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("SQL database error"),tr("Unable to open database. This is critical, cannot continue"),
								QMessageBox::Ok,
								QMessageBox::Ok);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						setErrorReturn(MPKG_RETURN_ABORT);
						break;

					case MPKG_SUBSYS_XMLCONFIG_READ_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Error in configuration files"),
							       	tr("Error in configuration files. Try to recreate? WARNING: all your settings will be lost!|"),
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Yes:
								setErrorReturn(MPKG_RETURN_REINIT);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;
					case MPKG_SUBSYS_XMLCONFIG_WRITE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Error writing configuration files"),
							       	tr("Error writing configuration files. Retry?"),
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Yes:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_SQLQUERY_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Internal error"),
							       	tr("SQL query error detected. This is critical internal error, we exit now."),
								QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_TMPFILE_CREATE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Error creating a temporary file"),
							       	tr("Error while creating a temp file. In most cases this mean that no free file descriptors available. This is critical, cannot continue"),
								QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_FILE_WRITE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Error writing file"),
							       	tr("File write error! Check for free space. Retry?"),
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Yes:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_FILE_READ_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Error reading file"),
							       	tr("File read error! Retry?"),
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						switch(userReply)
						{
							case QMessageBox::Yes:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						userReply = QMessageBox::NoButton;
						break;
					default:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage(tr("Unknown error!!!"), tr("Unknown error occured!!"), QMessageBox::Ignore, QMessageBox::Ignore);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(TIMER_RES);
						}
						setErrorReturn(MPKG_RETURN_IGNORE);
						break;
				}
			}
		}
		
		if (action == eBUS_Stop)
		{
			return;
		}
		msleep(TIMER_RES);
	}
}
void errorBus::receiveUserReply(QMessageBox::StandardButton reply)
{
	userReply = reply;
}

void errorBus::Start()
{
	action = eBUS_Run;
	start();
}

void errorBus::Pause()
{
	action = eBUS_Pause;
}

void errorBus::Stop()
{
	action = eBUS_Stop;
}


//======================================================STATUS THREAD==================================================

void statusThread::recvRedrawReady(bool flag)
{
	redrawReady=flag;
}

void statusThread::setPDataActive(bool flag)
{
	pDataActive=flag;
}

statusThread::statusThread()
{
	TIMER_RES = 400;
	idleTime=0;
	idleThreshold=40;
	enabledBar = false;
	enabledBar2 = false;
	show();
	pDataActive=false;
	redrawReady=true;
}

void statusThread::run()
{
	int tmp_c, tmp_c2;
	double dtmp, dtmp2;
	string dlStatus;
	forever 
	{
		switch(action)
		{
			case STT_Run:
				if (pData.size()>0 && redrawReady)
				{
					emit showProgressWindow(true);
					emit loadProgressData();
				}
				if (pData.size()==0) emit showProgressWindow(false);
			
				emit setStatus((QString) currentStatus.c_str());
				if (!actionBus.idle())
				{
					emit setIdleButtons(false);
					emit setSkipButton(actionBus.skippable(actionBus.currentProcessingID()));
					if (pData.size()>0) TIMER_RES = 200;
					else TIMER_RES=50;
					dtmp = 100 * (actionBus.progress()/actionBus.progressMaximum());
					tmp_c = (int) dtmp;
					
					if (!enabledBar)
					{
						emit enableProgressBar();
						if (actionBus.progressMaximum()==0)
						{
							emit initProgressBar(0);
						}
						else
						{	emit initProgressBar(100);
							enabledBar = true;
						}
					}
					else
					{
						if (actionBus.progress()>=0) emit setBarValue(tmp_c);
					}
				}
				else
				{
					emit setSkipButton(false);
					emit setIdleButtons(true);
					TIMER_RES=400;
					if (enabledBar)
					{
						emit disableProgressBar();
						enabledBar = false;
					}
				}
				
				if (pData.size()>0)
				{
					dtmp2 = 100 * (pData.getTotalProgress()/pData.getTotalProgressMax());
					tmp_c2 = (int) dtmp2;
					if (!enabledBar2)
					{
						emit initProgressBar2(100);
						emit enableProgressBar2();
						enabledBar2 = true;
					}
					else
					{
						emit setBarValue2(tmp_c2);
					}
				}
				else
				{
					if (enabledBar)
					{
						emit disableProgressBar2();
						enabledBar2 = false;
					}
				}

				break;
			case STT_Pause:
				break;
			case STT_Stop:
				return;
		}
		msleep(TIMER_RES);
	}
}

void statusThread::show()
{
	action = STT_Run;
}

void statusThread::hide()
{
	action = STT_Pause;
}

void statusThread::halt()
{
	action = STT_Stop;
}




coreThread::coreThread()
{
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

	//setPriority(QThread::LowPriority);
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

void coreThread::loadPackageDatabase()
{

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

void coreThread::_loadPackageDatabase()
{

	actionBus.clear();
	actionBus.addAction(ACTIONID_DBLOADING);
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
	actionBus.actions.at(actionBus.getActionPosition(ACTIONID_DBLOADING))._progressMaximum = tmpPackageList->size();

	tmpPackageList->initVersioning();
	actionBus.setActionState(ACTIONID_GETPKGLIST);
	actionBus.setCurrentAction(ACTIONID_DBLOADING);
	packageList = tmpPackageList;
	newStatus.clear();
	for (int i=0; i<packageList->size(); i++)
	{
		newStatus.push_back(packageList->get_package(i)->action());
	}
	sync();
	currentStatus = tr("Preparing dependency engine").toStdString();
	database->DepTracker->renderData();

	emit clearTable();
	emit setTableSize(0);
	emit setTableSize(packageList->size());

	currentStatus = tr("Loading packages into table").toStdString();

	for (int i=0; i<packageList->size(); i++)
	{
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

		
		actionBus.actions.at(actionBus.currentProcessing())._currentProgress=i;
	}

	emit disableProgressBar();
	emit loadingFinished();
	emit initState(true);
	emit applyFilters();
	currentStatus = tr("Loading finished").toStdString();
	currentAction=CA_Idle;
	actionBus.clear();
	pData.clear();
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


void coreThread::insertPackageIntoTable(unsigned int package_num)
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
	emit setTableItem(package_num, checked, pName);
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
					install_queue.add(packageList->get_package(i));//->get_name());
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

