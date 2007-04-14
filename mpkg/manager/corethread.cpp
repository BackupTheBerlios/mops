/****************************************************************************
 * MOPSLinux packaging system
 * Package manager - core functions thread
 * $Id: corethread.cpp,v 1.25 2007/04/14 15:53:52 i27249 Exp $
 * *************************************************************************/
#define USLEEP 15
#include "corethread.h"


errorBus::errorBus()
{
	action = eBUS_Pause;
}

void errorBus::run()
{
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
						emit sendErrorMessage("Download error","Some files failed to download. What to do?",
								QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore, 
								QMessageBox::Retry);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					//---------------CD-ROM ERRORS/EVENTS---------------------//
					case MPKG_CDROM_WRONG_VOLNAME:
					case MPKG_CDROM_MOUNT_ERROR:
						userReply = QMessageBox::NoButton;
						txt = "Please insert disk with label "+CDROM_VOLUMELABEL+" into "+CDROM_DEVICENAME;
						emit sendErrorMessage("Please insert disk", \
								txt.c_str(), QMessageBox::Ok | QMessageBox::Abort, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
						emit sendErrorMessage("Download error","Unable to download repository index. WTF?",
								QMessageBox::Retry | QMessageBox::Abort | QMessageBox::Ignore, 
								QMessageBox::Retry);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_INDEX_PARSE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Parse error","Error parsing repository index!",
								QMessageBox::Ok, 
								QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
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
						emit sendErrorMessage("Repository index error","Error retrieving repository index!",
								QMessageBox::Ok, 
								QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						switch(userReply)
						{
							case QMessageBox::Ok:
								setErrorReturn(MPKG_RETURN_SKIP);
								break;
							default:
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;



					//---------- INSTALLATION ERRORS --------------//
					case MPKG_INSTALL_OUT_OF_SPACE:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Out of space!", "Error installing packages - out of space.\nFree some disk space and try again", QMessageBox::Retry | QMessageBox::Abort, QMessageBox::Retry);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						switch(userReply)
						{
							case QMessageBox::Retry:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
							default:
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_INSTALL_SCRIPT_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Script error", "Error executing script", QMessageBox::Ok, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INSTALL_EXTRACT_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Package extraction error", "Error extracting package.", QMessageBox::Ok, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						setErrorReturn(MPKG_RETURN_SKIP);
						break;

					case MPKG_INSTALL_META_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Error extracting metadata", "Error while extracting metadata from package. Seems that package is broken", QMessageBox::Ok, QMessageBox::Ok);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						setErrorReturn(MPKG_RETURN_SKIP);
						break;

					case MPKG_INSTALL_FILE_CONFLICT:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("File conflict detected", "File conflict detected. You can force installation, but it is DANGEROUS (it may broke some components)", QMessageBox::Ignore | QMessageBox::Abort, QMessageBox::Ignore);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;



					//---------STARTUP ERRORS---------------//
					case MPKG_STARTUP_COMPONENT_NOT_FOUND:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Some components not found!", "Some components were not found, the program can fail during runtime. Continue?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
						emit sendErrorMessage("UID != 0", "You should run this program as root!", QMessageBox::Abort, QMessageBox::Abort);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						setErrorReturn(MPKG_RETURN_ABORT);
						break;

					
					//---------- SUBSYSTEM ERRORS ---------------------//
					case MPKG_SUBSYS_SQLDB_INCORRECT:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("SQL database error","Incorrect database structure. Reinitialize?",
								QMessageBox::Yes | QMessageBox::No, 
								QMessageBox::No);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_SQLDB_OPEN_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("SQL database error","Unable to open database. This is critical, cannot continue",
								QMessageBox::Ok,
								QMessageBox::Ok);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						setErrorReturn(MPKG_RETURN_ABORT);
						break;

					case MPKG_SUBSYS_XMLCONFIG_READ_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Error in configuration files",
							       	"Error in configuration files. Try to recreate? WARNING: all your settings will be lost!|",
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;
					case MPKG_SUBSYS_XMLCONFIG_WRITE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Error writing configuration files",
							       	"Error writing configuration files. Retry?",
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_SQLQUERY_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Internal error",
							       	"SQL query error detected. This is critical internal error, we exit now.",
								QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						switch(userReply)
						{
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_TMPFILE_CREATE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Error creating temp file",
							       	"Error while creating a temp file. In most cases this mean that no free file descriptors available. This is critical, cannot continue",
								QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
						}
						switch(userReply)
						{
							case QMessageBox::Abort:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_FILE_WRITE_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Error writing file",
							       	"File write error! Check for free space. Retry?",
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;

					case MPKG_SUBSYS_FILE_READ_ERROR:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Error reading file",
							       	"File read error! Retry?",
								QMessageBox::Yes | QMessageBox::Abort, 
								QMessageBox::Abort);
						while (userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
								printf("Unknown reply\n");
						}
						userReply = QMessageBox::NoButton;
						break;
					default:
						userReply = QMessageBox::NoButton;
						emit sendErrorMessage("Unknown error!!!", "Unknown error occured!!", QMessageBox::Ignore, QMessageBox::Ignore);
						while(userReply == QMessageBox::NoButton)
						{
							msleep(100);
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
		msleep(100);
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








statusThread::statusThread()
{
	enabledBar = false;
	enabledBar2 = false;
	show();
}

void statusThread::run()
{
	printf("Status thread started\n");
	setPriority(QThread::LowPriority);
	int tmp_c, tmp_c2;
	double dtmp, dtmp2;
	//int tmp_t, tmp_t2;
	string dlStatus;
	forever 
	{
		switch(action)
		{
			case STT_Run:
				if (!progressEnabled2) emit setStatus((QString) currentStatus.c_str());
				
				if (progressEnabled)
				{
					dtmp = 100 * (currentProgress/progressMax);
					tmp_c = (int) dtmp;
					
					if (!enabledBar)
					{
						emit initProgressBar(100);
						emit enableProgressBar();
						enabledBar = true;
					}
					else
					{
						emit setBarValue(tmp_c);
					}
				}
				else
				{
					if (enabledBar)
					{
						emit disableProgressBar();
						enabledBar = false;
					}
				}
				
				if (progressEnabled2)
				{
					dtmp2 = 100 * (currentProgress2/progressMax2);
					tmp_c2 = (int) dtmp2;
					dlStatus = "[" + IntToStr(currentProgress2) +"/" + IntToStr(progressMax2)+"] " + "Downloading "+currentItem+"... (" + IntToStr(currentProgress) + "/" + IntToStr(progressMax) + ")" ;
					emit setStatus ((QString) dlStatus.c_str());
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
		msleep(100);
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
	//printf("Core thread created\n");
	database = new mpkg;
	currentAction = CA_Idle;
}

coreThread::~coreThread()
{
	delete database;
	//printf("Thread destroyed correctly\n");
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
	setPriority(QThread::LowestPriority);
	//printf("Running...\n");
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
				emit setStatus(database->current_status().c_str());
				msleep(50);
				break;
			case CA_Quit:
				delete database;
				//printf("Called quit!\n");
				return; // Exiting!
			default:
				//printf("Out of loop! WARNING!!!\n");
				msleep(1);
		}
	}
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
	//vector<int> *tmpNewStatus;
	
	if (database->get_packagelist(sqlSearch, tmpPackageList, false)!=0)
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
	for (int i=0; i<packageList->size(); i++)
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
	//emit applyFilters();
	emit disableProgressBar();
	emit loadingFinished();
	currentAction=CA_Idle;
}

void coreThread::insertPackageIntoTable(unsigned int package_num)
{
	bool checked = false;
	string package_icon;
	if (packageList->get_package(package_num)->get_vstatus().find("INSTALLED") != std::string::npos || \
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
	//string pName;
	string pName = "<table><tbody><tr><td><img src = \"icons/"+package_icon+"\"></img></td><td><b>"+_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
	
	emit setTableItem(package_num, checked, pName);
}

PACKAGE_LIST *coreThread::getPackageList()
{
	emit sendPackageList(*packageList, newStatus);
	return packageList;
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
	vector<int> reset_queue;
	for (unsigned int i = 0; i< newStatus.size(); i++)
	{
		if (packageList->get_package(i)->get_status()!=newStatus[i])
		{
			switch(newStatus[i])
			{
				case PKGSTATUS_AVAILABLE:
					reset_queue.push_back(packageList->get_package(i)->get_id());
					break;
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
	//printf("install_queue size = %d\n", install_queue.size());
	//printf("remove_queue size = %d\n", remove_queue.size());
	database->uninstall(remove_queue);
	database->install(install_queue);
	database->purge(purge_queue);
	for (unsigned int i = 0; i<reset_queue.size(); i++)
	{
		database->unqueue(reset_queue[i]);
	}
	emit setStatus("Committing changes...");
	database->commit();
	emit setStatus("All operations completed");
	delete database;
	database = new mpkg;
	currentAction = CA_LoadDatabase;
}

void coreThread::syncData()
{
}

void coreThread::cleanCache()
{
	database->clean_cache();
}
