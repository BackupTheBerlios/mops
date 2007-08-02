/***********************************************************************
 * 	$Id: mpkg.cpp,v 1.101 2007/08/02 10:39:13 i27249 Exp $
 * 	MOPSLinux packaging system
 * ********************************************************************/
#include "mpkg.h"
#include "syscommands.h"
#include "DownloadManager.h"

#include "dialog.h"
/** Scans database and do actions. Actually, packages will install in SYS_ROOT folder.
 * In real (mean installed) systems, set SYS_ROOT to "/"
 * @**/


mpkgDatabase::mpkgDatabase()
{
	hasFileList=false;
}
mpkgDatabase::~mpkgDatabase(){}

int mpkgDatabase::sqlFlush()
{
	return db.sqlFlush();
}

PACKAGE mpkgDatabase::get_installed_package(string* pkg_name)
{
	PACKAGE_LIST packagelist;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", pkg_name);
	sqlSearch.addField("package_installed", ST_INSTALLED);

	get_packagelist(&sqlSearch, &packagelist);
	// We do NOT allow multiple packages with same name to be installed, so, we simply get first package of list.
	
	if (packagelist.size()>0)
		return *packagelist.get_package(0);
	else
	{
		PACKAGE ret;
		return ret;
	}
}
	

int mpkgDatabase::emerge_to_db(PACKAGE *package)
{
	mDebug("mpkgDatabase::emerge_to_db()");
	int pkg_id;
	pkg_id=get_package_id(package);
	if (pkg_id==0)
	{
		// New package, adding
		add_package_record(package);
		return 0;
	}
	if (pkg_id<0)
	{
		// Query error
		return MPKGERROR_CRITICAL;
	}
	// Раз пакет уже в базе (и в единственном числе - а иначе и быть не должно), сравниваем данные.
	// В случае необходимости, добавляем location.
	PACKAGE db_package;
	vector<LOCATION> new_locations;
	get_package(pkg_id, &db_package);
	package->set_id(pkg_id);

	for (unsigned int j=0; j<package->get_locations()->size(); j++)
	{
		for (unsigned int i=0; i<db_package.get_locations()->size(); i++)
		{
			if (!package->get_locations()->at(j).equalTo(&db_package.get_locations()->at(i)))
			{
				new_locations.push_back(package->get_locations()->at(j));
			}
		}
	}
	if (!new_locations.empty()) add_locationlist_record(pkg_id, &new_locations);
	return 0;
}


bool mpkgDatabase::check_cache(PACKAGE *package, bool clear_wrong)
{
	string fname = SYS_CACHE + "/" + *package->get_filename();
	string got_md5 = get_file_md5(SYS_CACHE + "/" + *package->get_filename());
	if (FileExists(fname) && *package->get_md5() == got_md5)
		return true;
	else
	{
		if (clear_wrong) unlink(fname.c_str());
		return false;
	}
}


int mpkgDatabase::commit_actions()
{
	mDebug("Committing");
	Dialog dialogItem;
	delete_tmp_files();
	sqlFlush();
	// Zero: purging required packages
	// First: removing required packages
	unsigned int installFailures=0;
	unsigned int removeFailures=0;
	PACKAGE_LIST remove_list;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", ST_REMOVE);
	sqlSearch.addField("package_action", ST_PURGE);
	if (get_packagelist(&sqlSearch, &remove_list)!=0) return MPKGERROR_SQLQUERYERROR;
	remove_list.sortByPriority(true);
	PACKAGE_LIST install_list;
	sqlSearch.clear();
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", ST_INSTALL);
	sqlSearch.addField("package_action", ST_REPAIR);
	if (get_packagelist(&sqlSearch, &install_list)!=0) return MPKGERROR_SQLQUERYERROR;
	install_list.sortByPriority();
	// Checking available space
	long double rem_size=0;
	long double ins_size=0;

	for (int i=0; i<remove_list.size(); i++)
	{
		remove_list.get_package(i)->itemID = pData.addItem(*remove_list.get_package(i)->get_name(), 10);
		rem_size+=strtod(remove_list.get_package(i)->get_installed_size()->c_str(), NULL);
	}
	for (int i=0; i<install_list.size(); i++)
	{
		install_list.get_package(i)->itemID = pData.addItem(*install_list.get_package(i)->get_name(), atoi(install_list.get_package(i)->get_compressed_size()->c_str()));
		ins_size += strtod(install_list.get_package(i)->get_installed_size()->c_str(), NULL);
	}
	long double freespace = get_disk_freespace(SYS_ROOT);
	mDebug("Freespace is " + IntToStr((long long) freespace));
	
	if (freespace < (ins_size - rem_size))
	{
		mDebug("Not enough free space, show warning");
		if (dialogMode)
		{
			Dialog dialogItem;
			if (!dialogItem.execYesNo("Судя по всему, для установки места на диске не хватит.\nНа корневой файловой системе " + SYS_ROOT+" имеется " + humanizeSize(freespace) + "\nДля установки требуется: " + humanizeSize(ins_size - rem_size) + "\nВсе равно продолжить?"))
			{
				return MPKGERROR_COMMITERROR;
			}
		}
		else mError(_("Seems that free space is not enough to install. Required: ") + humanizeSize(ins_size - rem_size) + _(", available: ") + humanizeSize(freespace));
	}
	else mDebug("Freespace OK, proceeding");

	// Building action list
	mDebug("Building action list");
	actionBus.clear();
	if (remove_list.size()>0)
	{
		actionBus.addAction(ACTIONID_REMOVE);
		actionBus.setActionProgressMaximum(ACTIONID_REMOVE, remove_list.size());
	}
	if (install_list.size()>0)
	{
		actionBus.addAction(ACTIONID_CACHECHECK);
		actionBus.setSkippable(ACTIONID_CACHECHECK, true);
		actionBus.setActionProgressMaximum(ACTIONID_CACHECHECK, install_list.size());
		actionBus.addAction(ACTIONID_DOWNLOAD);
		actionBus.setActionProgressMaximum(ACTIONID_DOWNLOAD, install_list.size());
		actionBus.addAction(ACTIONID_MD5CHECK);
		actionBus.setSkippable(ACTIONID_MD5CHECK, true);
		actionBus.setActionProgressMaximum(ACTIONID_MD5CHECK, install_list.size());
		actionBus.addAction(ACTIONID_INSTALL);
		actionBus.setActionProgressMaximum(ACTIONID_INSTALL, install_list.size());

	}
	// Done

	mDebug("Building remove queue");
	if (remove_list.size()>0)
	{
		actionBus.setCurrentAction(ACTIONID_REMOVE);

		currentStatus = _("Looking for remove queue");
			mDebug ("Calling REMOVE for "+IntToStr(remove_list.size())+" packages");
		currentStatus = _("Removing ") + IntToStr(remove_list.size()) + _(" packages");
		pData.setCurrentAction("Removing packages");
	
		int removeItemID=0;
		for (int i=0; i<remove_list.size(); i++)
		{
			removeItemID=remove_list.get_package(i)->itemID;
			pData.setItemState(removeItemID, ITEMSTATE_WAIT);
			pData.setItemCurrentAction(removeItemID, _("Waiting"));
			pData.resetIdleTime(removeItemID);
			pData.setItemProgress(removeItemID, 0);
			pData.setItemProgressMaximum(removeItemID,8);
		}
	
		for (int i=0;i<remove_list.size();i++)
		{
			if (dialogMode)
			{
				dialogItem.execGauge("[" + IntToStr(i+1) + "/" + IntToStr(remove_list.size()) + _("] Removing package ") + \
						*remove_list.get_package(i)->get_name() + "-" + \
						remove_list.get_package(i)->get_fullversion(), 10,80, \
						(unsigned int) round((double)(i)/(double)((double)(remove_list.size())/(double) (100))));
			}	
			delete_tmp_files();
			actionBus.setActionProgress(ACTIONID_REMOVE,i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_REMOVE, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			for (int t=0; t<install_list.size(); t++)
			{
				if (*install_list.get_package(t)->get_name() == *remove_list.get_package(i)->get_name())
				{
					say(_("Updating package %s\n"), remove_list.get_package(i)->get_name()->c_str()); 
					remove_list.get_package(i)->isUpdating=true;
					break;
				}
			}
			currentStatus = _("Removing package ") + *remove_list.get_package(i)->get_name();
			if (remove_package(remove_list.get_package(i))!=0)
			{
				removeFailures++;
				pData.setItemCurrentAction(remove_list.get_package(i)->itemID, _("Remove failed"));
				pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_FAILED);
			}
			else
			{
				pData.setItemCurrentAction(remove_list.get_package(i)->itemID, _("Removed"));
				pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}

			if (dialogMode) 
				dialogItem.closeGauge();
		
		}
		sqlSearch.clear();

		clean_backup_directory();
		actionBus.setActionState(ACTIONID_REMOVE);
	}


	currentStatus = _("Looking for install queue");

	if (install_list.size()>0)
	{
		actionBus.setCurrentAction(ACTIONID_CACHECHECK);
		// Building download queue
		currentStatus = _("Looking for package locations...");
		DownloadsList downloadQueue;
		DownloadItem tmpDownloadItem;
		vector<string> itemLocations;
		//double totalDownloadSize=0;
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
		pData.setCurrentAction(_("Checking cache"));
		bool skip=false;
		if (dialogMode)
		{
			dialogItem.execGauge(_("Checking package cache"), 10,80,0);
		}

		for (int i=0; i<install_list.size(); i++)
		{
			if (dialogMode)
			{
				dialogItem.setGaugeValue((unsigned int) round((double) (i)/((double) (install_list.size())/(double) (100))));
			}
			delete_tmp_files();

			actionBus.setActionProgress(ACTIONID_CACHECHECK, i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				return MPKGERROR_ABORTED;
			}
			if (actionBus.skipped(ACTIONID_CACHECHECK))
			{
				skip=true;
			}

			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Checking package cache"));
			pData.setItemProgressMaximum(install_list.get_package(i)->itemID, 1);
			pData.setItemProgress(install_list.get_package(i)->itemID, 0);
	
			currentStatus = _("Checking cache and building download queue: ") + *install_list.get_package(i)->get_name();
	
	
			if (skip || !check_cache(install_list.get_package(i), false))
			{
				if (!skip) pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("not cached"));
				else pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("check skipped"));

				itemLocations.clear();
				
				tmpDownloadItem.expectedSize=strtod(install_list.get_package(i)->get_compressed_size()->c_str(), NULL);
				tmpDownloadItem.file = SYS_CACHE + *install_list.get_package(i)->get_filename();
				tmpDownloadItem.name = *install_list.get_package(i)->get_name();
				tmpDownloadItem.priority = 0;
				tmpDownloadItem.status = DL_STATUS_WAIT;
				tmpDownloadItem.itemID = install_list.get_package(i)->itemID;
	
				install_list.get_package(i)->sortLocations();
				for (unsigned int k = 0; k < install_list.get_package(i)->get_locations()->size(); k++)
				{
					itemLocations.push_back(*install_list.get_package(i)->get_locations()->at(k).get_server_url() \
						     + *install_list.get_package(i)->get_locations()->at(k).get_path() \
						     + *install_list.get_package(i)->get_filename());
	
				}
				tmpDownloadItem.url_list = itemLocations;
				downloadQueue.push_back(tmpDownloadItem);
			}
			else pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("cached"));
	
			pData.increaseItemProgress(install_list.get_package(i)->itemID);
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
		}
		if (dialogMode) dialogItem.closeGauge();
		actionBus.setActionState(ACTIONID_CACHECHECK);
		actionBus.setCurrentAction(ACTIONID_DOWNLOAD);
		mpkgErrorReturn errRet;
		bool do_download = true;
	
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
		while(do_download)
		{
			do_download = false;
			pData.downloadAction=true;

			if (CommonGetFileEx(downloadQueue, &currentItem) == DOWNLOAD_ERROR)
			{
				mError(_("Download failed"));
				errRet = waitResponce (MPKG_DOWNLOAD_ERROR);
				switch(errRet)
				{
					case MPKG_RETURN_IGNORE:
						say(_("Download errors ignored, continue installing\n"));
						goto installProcess;
						break;
				
					case MPKG_RETURN_RETRY:
						say(_("retrying...\n"));
						do_download = true;
						break;
					case MPKG_RETURN_ABORT:
						say(_("aborting...\n"));
						return MPKGERROR_ABORTED;
						break;
					default:
						mError(_("Unknown value, don't know what to do, aborting"));
						return MPKGERROR_ABORTED;
				}
					
			}
		
		}
		actionBus.setActionState(ACTIONID_DOWNLOAD);
		pData.downloadAction=false;
installProcess:
	
		actionBus.setCurrentAction(ACTIONID_MD5CHECK);
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
	
		//bool hasErrors=false;
		skip=false;
		currentStatus = _("Checking files (comparing MD5):");
		pData.setCurrentAction(_("Checking md5"));
		if (dialogMode)
		{
			dialogItem.execGauge(_("Checking packages integrity"), 10,80,0);
		}
		for (int i=0; i<install_list.size(); i++)
		{
			actionBus.setActionProgress(ACTIONID_MD5CHECK, i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_MD5CHECK, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			if (actionBus.skipped(ACTIONID_MD5CHECK) || forceSkipLinkMD5Checks) break;

			if (dialogMode)
			{
				dialogItem.setGaugeValue((unsigned int) round(((double)(i)/(double) ((double) (install_list.size())/(double) (100)))));

			}
			else say(_("Checking MD5 for %s\n"), install_list.get_package(i)->get_filename()->c_str());
			currentStatus = _("Checking md5 of downloaded files: ") + *install_list.get_package(i)->get_name();
	
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Checking md5"));
			pData.setItemProgressMaximum(install_list.get_package(i)->itemID, 1);
			pData.setItemProgress(install_list.get_package(i)->itemID, 0);
	
			if (!check_cache(install_list.get_package(i), true))
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("md5 incorrect"));
				pData.increaseItemProgress(install_list.get_package(i)->itemID);
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FAILED);
	
				errRet = waitResponce(MPKG_DOWNLOAD_ERROR);
				switch(errRet)
				{
					case MPKG_RETURN_IGNORE:
						say(_("Wrong checksum ignored, continuing...\n"));
						break;
					case MPKG_RETURN_RETRY:
						say(_("Re-downloading...\n"));
						break;
					case MPKG_RETURN_ABORT:
						say(_("Aborting installation\n"));
						return MPKGERROR_ABORTED;
						break;
					default:
						mError(_("Unknown reply, aborting"));
						return MPKGERROR_ABORTED;
						break;
				}
			}
			else
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, "MD5 OK");
				pData.increaseItemProgress(install_list.get_package(i)->itemID);
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}
		}
		if (dialogMode) dialogItem.closeGauge();
		actionBus.setActionState(ACTIONID_MD5CHECK);
	
		actionBus.setCurrentAction(ACTIONID_INSTALL);
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
	
		pData.setCurrentAction(_("Installing packages"));
		int installItemID;
		for (int i=0; i<install_list.size(); i++)
		{
			installItemID=install_list.get_package(i)->itemID;
			pData.setItemState(installItemID, ITEMSTATE_WAIT);
			pData.setItemCurrentAction(installItemID, _("Waiting"));
			pData.resetIdleTime(installItemID);
			pData.setItemProgress(installItemID, 0);
			pData.setItemProgressMaximum(installItemID,8);
		}
		for (int i=0;i<install_list.size();i++)
		{
			
			actionBus.setActionProgress(ACTIONID_INSTALL, i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			currentStatus = _("Installing package ") + *install_list.get_package(i)->get_name();
			if (dialogMode)
			{
				dialogItem.execGauge("[" + IntToStr(i+1) + "/" + IntToStr(install_list.size()) + _("] Installing package ") + \
						*install_list.get_package(i)->get_name() + "-" + \
						install_list.get_package(i)->get_fullversion(), 10,80, \
						(unsigned int) round((double) (i)/(double) ((double) (install_list.size())/(double) (100))));
			}	
			if (install_package(install_list.get_package(i))!=0)
			{
				installFailures++;
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Installation failed"));
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FAILED);
			}
			else
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Installed"));
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}
		}
		currentStatus = _("Installation complete.");
		actionBus.setActionState(ACTIONID_INSTALL);
	}
	if (removeFailures!=0 && installFailures!=0) return MPKGERROR_COMMITERROR;
	actionBus.clear();
	return 0;
}

int mpkgDatabase::install_package(PACKAGE* package)
{

	pData.setItemCurrentAction(package->itemID, _("installing"));
	if (!dialogMode) say(_("Installing %s %s\n"),package->get_name()->c_str(), package->get_fullversion().c_str());
	string statusHeader = "["+IntToStr((int) actionBus.progress())+"/"+IntToStr((int)actionBus.progressMaximum())+"] "+_("Installing package ") + *package->get_name()+": ";
	currentStatus = statusHeader + _("initialization");
	// First of all: EXTRACT file list and scripts!!!
	LocalPackage lp(SYS_CACHE + *package->get_filename());
	bool no_purge=true;
	vector<FILES> old_config_files;
	mDebug("purge check");
	int purge_id=get_purge(package->get_name()); // returns package id if this previous package config files are not removed, or 0 if purged.
	mDebug("purge check complete");
	mDebug("purge_id="+IntToStr(purge_id));
	if (purge_id==0)
	{
		no_purge=false;
	}
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	currentStatus = statusHeader + _("extracting installation scripts");
	pData.increaseItemProgress(package->itemID);
	lp.fill_scripts(package);
	currentStatus = statusHeader + _("extracting file list");
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}
	lp.fill_filelist(package);
	currentStatus = statusHeader + _("detecting configuration files");
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	lp.fill_configfiles(package);
	mDebug("fill ended");
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	if (fileConflictChecking==CHECKFILES_PREINSTALL) 
	{
		currentStatus = statusHeader + _("checking file conflicts");
		pData.increaseItemProgress(package->itemID);

	}
	mDebug("Checking file conflicts\n");
	if (!force_skip_conflictcheck)
	{
		if (fileConflictChecking == CHECKFILES_PREINSTALL && check_file_conflicts(package)!=0)
		{
			mDebug("Check failed (dupes present)");
			currentStatus = _("Error: Unresolved file conflict on package ") + *package->get_name();
			mError(_("Unresolved file conflict on package ") + *package->get_name() + _(", it will be skipped!"));
			return -5;
		}
		mDebug("Check conflicts ok");
	}
	else mDebug("Conflict check skipped");
	
	currentStatus = statusHeader + _("installing...");
	pData.increaseItemProgress(package->itemID);


// Filtering file list...
	vector<FILES> package_files;
	if (!no_purge) add_filelist_record(package->get_id(), package->get_files());
	string sys;
	mDebug("Preparing scripts");
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}


	if (!DO_NOT_RUN_SCRIPTS)
	{
		currentStatus = statusHeader + _("executing pre-install scripts");
		if (FileExists(package->get_scriptdir() + "preinst.sh"))
		{
			string preinst="cd " + SYS_ROOT + " ; sh "+package->get_scriptdir() + "preinst.sh";
			if (!simulate) system(preinst.c_str());
		}
	}

	// Extracting package
	currentStatus = statusHeader + _("extracting...");
	pData.increaseItemProgress(package->itemID);


	mDebug("calling extract");
	string sys_cache=SYS_CACHE;
	string sys_root=SYS_ROOT;
	string create_root="mkdir -p "+sys_root+" 2>/dev/null";
	if (!simulate) system(create_root.c_str());
	sys="(cd "+sys_root+"; tar zxf "+sys_cache + *package->get_filename()+" --exclude install";
	//If previous version isn't purged, do not overwrite config files
	if (no_purge)
	{
		mDebug("retrieving old config files");
		get_filelist(purge_id, &old_config_files, true);
		mDebug("no_purge flag IS SET, config files count = "+IntToStr(package->get_config_files()->size()));
		for (unsigned int i=0; i<package->get_config_files()->size(); i++)
		{
			// Writing new config files, skipping old
			for (unsigned int k=0; k < old_config_files.size(); k++)
			{
				if (*package->get_config_files()->at(i).get_name()==*old_config_files[k].get_name())
				{
					mDebug("excluding file "+*package->get_config_files()->at(i).get_name());
					sys+=" --exclude "+*package->get_config_files()->at(i).get_name();
				}
			}
		}
		for (unsigned int i=0; i<package->get_files()->size(); i++)
		{
			for (unsigned int k=0; k <= old_config_files.size(); k++)
			{
				if (k==old_config_files.size()) 
				{
					package_files.push_back(package->get_files()->at(i));
					break;
				}
				if (*package->get_files()->at(i).get_name()==*old_config_files[k].get_name())
				{
					mDebug("Skipping file " + *package->get_files()->at(i).get_name());
					break;
				}
			}
		}
		mDebug("package_files size: "+IntToStr(package_files.size())+", package->get_files size: "+IntToStr(package->get_files()->size()));
		add_filelist_record(package->get_id(), &package_files);
	}
	package->get_files()->clear();
	package_files.clear();
	sys+=" > /dev/null)";
//#ifdef ACTUAL_EXTRACT
	if (!simulate)
	{
		if (system(sys.c_str()) == 0) currentStatus = statusHeader + _("executing post-install scripts...");
		else {
			currentStatus = _("Failed to extract!");
			return -10;
		}
	}
//#endif
	pData.increaseItemProgress(package->itemID);


	
	// Creating and running POST-INSTALL script
	if (!DO_NOT_RUN_SCRIPTS)
	{
		if (FileExists(package->get_scriptdir() + "doinst.sh"))
		{
			string postinst="cd " + SYS_ROOT + " ; sh "+package->get_scriptdir() + "doinst.sh";
			if (!simulate) system(postinst);
		}
	}

	set_installed(package->get_id(), ST_INSTALLED);
	set_configexist(package->get_id(), ST_CONFIGEXIST);
	set_action(package->get_id(), ST_NONE);
	sqlFlush();
	mDebug("*********************************************\n*        Package installed sussessfully     *\n*********************************************");
	pData.increaseItemProgress(package->itemID);


	return 0;
}	//End of install_package


int mpkgDatabase::remove_package(PACKAGE* package)
{
	if (!package->isUpdating && package->isRemoveBlacklisted())
	{
		mError(_("Cannot remove package ") + *package->get_name() + _(", because it is an important system component."));
		set_action(package->get_id(), ST_NONE);
		return MPKGERROR_IMPOSSIBLE;
	}
	get_filelist(package->get_id(), package->get_files());
	package->sync();
	pData.setItemProgressMaximum(package->itemID, package->get_files()->size()+8);
	pData.setItemCurrentAction(package->itemID, "removing");
	if (!dialogMode)
	{
		if (package->isUpdating) say(_("Updating package %s %s...\n"),package->get_name()->c_str(), package->get_fullversion().c_str());
		else say(_("Removing package %s %s...\n"),package->get_name()->c_str(), package->get_fullversion().c_str());
	}

	string statusHeader = "["+IntToStr((int)actionBus.progress())+"/"+IntToStr((int)actionBus.progressMaximum()) + "] " + _("Removing package ") + *package->get_name()+": ";
	currentStatus = statusHeader + _("initialization");
	
	if (package->action()==ST_REMOVE || package->action()==ST_PURGE)
	{
		if (!package->isRemoveBlacklisted())
		{
			// Running pre-remove scripts
			mDebug("REMOVE PACKAGE::Preparing scripts");
			if(!DO_NOT_RUN_SCRIPTS)
			{
				if (FileExists(package->get_scriptdir() + "preremove.sh"))
				{
					currentStatus = statusHeader + _("executing pre-remove scripts");
					string prerem="cd " + SYS_ROOT + " ; sh "+package->get_scriptdir() + "preremove.sh";
					if (!simulate) system(prerem.c_str());
				}
			}
		
			pData.increaseItemProgress(package->itemID);
			// removing package
			mDebug("calling remove");
			string sys_cache=SYS_CACHE;
			string sys_root=SYS_ROOT;
			string fname;
			mDebug("Package has "+IntToStr(package->get_files()->size())+" files");

			// Purge is now implemented here; checking all
			currentStatus = statusHeader + _("building file list");
			vector<FILES> *remove_files=package->get_files();
			currentStatus = statusHeader + _("removing files...");
			bool removeThis;
			for (unsigned int i=0; i<remove_files->size(); i++)
			{
				removeThis = false;
				if (package->action()==ST_PURGE || remove_files->at(i).get_type()==FTYPE_PLAIN) removeThis = true;
				fname=sys_root + *remove_files->at(i).get_name();
				if (removeThis && fname[fname.length()-1]!='/')
				{
					pData.increaseItemProgress(package->itemID);
					if (!simulate) unlink (fname.c_str());
				}
			}

			currentStatus = statusHeader + _("removing empty directories...");
	
			// Run 2: clearing empty directories
			vector<string>empty_dirs;
			string edir;
		
			pData.increaseItemProgress(package->itemID);
			
			for (unsigned int i=0; i<remove_files->size(); i++)
			{
				fname=sys_root + *remove_files->at(i).get_name();
				for (unsigned int d=0; d<fname.length(); d++)
				{
					edir+=fname[d];
					if (fname[d]=='/')
					{
						empty_dirs.resize(empty_dirs.size()+1);
						empty_dirs[empty_dirs.size()-1]=edir;
					}
				}
	
				for (int x=empty_dirs.size()-1;x>=0; x--)
				{
					if (!simulate) rmdir(empty_dirs[x].c_str());
				}
				edir.clear();
				empty_dirs.clear();
			}
	
			// Creating and running POST-REMOVE script
			if (!DO_NOT_RUN_SCRIPTS)
			{
				if (FileExists(package->get_scriptdir() + "postremove.sh"))
				{
					currentStatus = statusHeader + _("executing post-removal scripts");
					string postrem="cd " + SYS_ROOT + " ; sh " + package->get_scriptdir() + "postremove.sh";
					if (!simulate) system(postrem.c_str());
				}
			}
		
	
			// Restoring backups
			vector<FILES>restore;
       			get_conflict_records(package->get_id(), &restore);
			if (!restore.empty())
			{
				string cmd;
				string tmpName;
				for (unsigned int i=0; i<restore.size(); i++)
				{
					cmd = "mkdir -p ";
		       			cmd += SYS_ROOT + restore[i].get_backup_file()->substr(0, restore[i].get_backup_file()->find_last_of("/"));
					if (!simulate) system(cmd.c_str());
					cmd = "mv ";
				        cmd += *restore[i].get_backup_file() + " ";
					tmpName = restore[i].get_backup_file()->substr(strlen(SYS_BACKUP));
					tmpName = tmpName.substr(tmpName.find("/"));
				        cmd += SYS_ROOT	+ tmpName;
					if (!simulate) system(cmd.c_str());
					delete_conflict_record(package->get_id(), restore[i].get_backup_file());
				}
			}
		}
		pData.increaseItemProgress(package->itemID);
		set_installed(package->get_id(), ST_NOTINSTALLED);
		if (package->action()==ST_PURGE) set_configexist(package->get_id(), 0);
		set_action(package->get_id(), ST_NONE);
		currentStatus = statusHeader + _("cleaning file list");
		pData.increaseItemProgress(package->itemID);
		cleanFileList(package->get_id());
		pData.increaseItemProgress(package->itemID);
		sqlFlush();
		currentStatus = statusHeader + _("remove complete");
		mDebug("Package removed sussessfully");
		package->get_files()->clear();
		return 0;
	}
	else
	{
		mError(_("Weird status of package, i'm afraid to remove this..."));
		return -1;
	}
}	// End of remove_package

int mpkgDatabase::delete_packages(PACKAGE_LIST *pkgList)
{
	if (pkgList->IsEmpty())
	{
		return 0;
	}
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	for (int i=0; i<pkgList->size(); i++)
	{
		sqlSearch.addField("package_id", pkgList->get_package(i)->get_id());
	}
	db.sql_delete("packages", sqlSearch);
	sqlSearch.clear();
	for (int i=0; i<pkgList->size(); i++)
	{
		sqlSearch.addField("packages_package_id", pkgList->get_package(i)->get_id());
	}
	db.sql_delete("tags_links", sqlSearch);
	db.sql_delete("dependencies", sqlSearch);
#ifdef ENABLE_INTERNATIONAL
	db.sql_delete("descriptions", sqlSearch);
	db.sql_delete("changelogs", sqlSearch);
	db.sql_delete("ratings", sqlSearch);
#endif

	// Removing unused tags
	sqlSearch.clear();
	SQLTable available_tags;
	SQLRecord fields;

	db.get_sql_vtable(&available_tags, sqlSearch, "tags", fields);
	SQLTable used_tags;
	db.get_sql_vtable(&used_tags, sqlSearch, "tags_links", fields);
	vector<string> toDelete;
	bool used;
	if (available_tags.size()>0)
	{
		for (int i=0; i<available_tags.size(); i++)
		{
			used=false;
			for (int u=0; u<used_tags.size(); u++)
			{
				if (*used_tags.getValue(u, "tags_tag_id")==*available_tags.getValue(i, "tags_id"))
				{
					used=true;
				}
			}
			
			if (!used)
			{
				say(_("Deleting tag %s as unused\n"), available_tags.getValue(i, "tags_name")->c_str());
				toDelete.push_back(*available_tags.getValue(i,"tags_id"));
			}
		}
		available_tags.clear();
		sqlSearch.clear();
		sqlSearch.setSearchMode(SEARCH_OR);
		if (toDelete.size()>0)
		{
			for (unsigned int i=0; i<toDelete.size(); i++)
			{
				sqlSearch.addField("tags_id", &toDelete[i]);
			}
			db.sql_delete("tags", sqlSearch);
		}
	}
	return 0;
}



int mpkgDatabase::cleanFileList(int package_id)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", package_id);
	if (get_configexist(package_id)) sqlSearch.addField("file_type", FTYPE_PLAIN);
	return db.sql_delete("files", sqlSearch);
}

int mpkgDatabase::update_package_data(int package_id, PACKAGE *package)
{
	PACKAGE old_package;
	if (get_package(package_id, &old_package)!=0)
	{
		mDebug("mpkg.cpp: update_package_data(): get_package error: no package or error while querying database");
		return -1;
	}
	
	SQLRecord sqlUpdate;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);

	mDebug("mpkg.cpp: update_package_data(): updating direct package data");
	// 1. Updating direct package data
	if (*package->get_md5()!=*old_package.get_md5())
	{
		mDebug("mpkg.cpp: update_package_data(): md5 mismatch, updating description fields");
		sqlUpdate.addField("package_description", package->get_description());
		sqlUpdate.addField("package_short_description", package->get_short_description());
		sqlUpdate.addField("package_compressed_size", package->get_compressed_size());
		sqlUpdate.addField("package_installed_size", package->get_installed_size());
		sqlUpdate.addField("package_changelog", package->get_changelog());
		sqlUpdate.addField("package_packager", package->get_packager());
		sqlUpdate.addField("package_packager_email", package->get_packager_email());
		sqlUpdate.addField("package_md5", package->get_md5());
	}

	// 2. Updating filename
	if (*package->get_filename()!=*old_package.get_filename())
	{
		mDebug("mpkg.cpp: update_package_data(): filename mismatch, updating");
		sqlUpdate.addField("package_filename", package->get_filename());
	}

	// 3. Updating status. Seems that somewhere here is an error causing double scan is required
	sqlUpdate.addField("package_available", package->available());


	// 4. Updating locations
	
	// Note about method used for updating locations:
	// If locations are not identical:
	// 	Step 1. Remove all existing package locations from "locations" table. Servers are untouched.
	// 	Step 2. Add new locations.
	// Note: after end of updating procedure for all packages, it will be good to do servers cleanup - delete all servers who has no locations.
	mDebug("mpkg.cpp: update_package_data(): checking locations");	
	if (!package->locationsEqualTo(&old_package))
	{
		mDebug("mpkg.cpp: update_package_data(): locations mismatch, cleanup");
		mDebug("mpkg.cpp: update_package_data(): old has "+IntToStr(old_package.get_locations()->size())+" locations, but new has "+\
				IntToStr(package->get_locations()->size())+" ones");
		SQLRecord loc_sqlDelete;
		loc_sqlDelete.addField("packages_package_id", package_id);
		mDebug("mpkg.cpp: update_package_data(): deleting old locations relating this package");
		int sql_del=db.sql_delete("locations", loc_sqlDelete);
		if (sql_del!=0)
		{
			mDebug("mpkg.cpp: update_package_data(): unable to delete old locations: SQL error "+IntToStr(sql_del));
			return -2;
		}
		if (add_locationlist_record(package_id, package->get_locations())!=0)
		{
			mDebug("mpkg.cpp: update_package_data(): unable to add new locations: error in add_locationlist_record()");
			return -3;
		}
		if (package->get_locations()->empty())
		{
			sqlUpdate.addField("package_available", ST_NOTAVAILABLE);
		}
	}

	// 5. Updating tags
	if (!package->tagsEqualTo(&old_package))
	{
		SQLRecord taglink_sqlDelete;
		taglink_sqlDelete.addField("packages_package_id", package_id);

		if (db.sql_delete("tags_links", taglink_sqlDelete)!=0) return -4;
		if (add_taglist_record(package_id, package->get_tags())!=0) return -5;
	}

	// 6. Updating dependencies
	if (!package->depsEqualTo(&old_package))
	{
		SQLRecord dep_sqlDelete;
		dep_sqlDelete.addField("packages_package_id", package_id);

		if(db.sql_delete("dependencies", dep_sqlDelete)!=0) return -6;
		if (add_dependencylist_record(package_id, package->get_dependencies())!=0) return -7;
	}

	// 7, 8 - update scripts and file list. It is skipped for now, because we don't need this here (at least, for now).
	if (!sqlUpdate.empty())
	{
		if (db.sql_update("packages", sqlUpdate, sqlSearch)!=0)
		{
			mDebug("mpkg.cpp: update_package_data(): db.sql_update failed");
			return -8;
		}
	}
	else mDebug("mpkg.cpp: update_package_data(): sqlUpdate empty, nothing to update");
	mDebug("mpkg.cpp: update_package_data(): successful end");
	return 0;
}




int mpkgDatabase::updateRepositoryData(PACKAGE_LIST *newPackages)
{
	// Одна из самых страшных функций в этой программе.
	// Попытаемся применить принципиально новый алгоритм.
	// 
	// Для этого введем некоторые тезисы:
	// 1. Пакет однозначно идентифицируется по его контрольной сумме.
	// 2. Пакет без контрольной суммы - не пакет а мусор, выкидываем такое нахрен.
	// 
	// Алгоритм:
	// 1. Стираем все записи о locations и servers в базе.
	// 2. Забираем из базы весь список пакетов. Этот список и будет рабочим.
	// 3. Для каждого пакета из нового списка ищем соответствие в старой базе.
	// 	3а. В случае если такое соответствие найдено, вписываем в список записи о locations. Остальное остается неизменным, ибо MD5 та же.
	// 	3б. В случае если соответствие не найдено, пакет добавляется в конец рабочего списка с флагом new = true
	// 4. Вызывается синхронизация рабочего списка и данных в базе (это уже отдельная тема).
	//
	// ////////////////////////////////////////////////
	mDebug("Retrieving current package list, clearing tables");	
	// Стираем locations и servers
	db.clear_table("locations");
	

	// Забираем текущий список пакетов
	PACKAGE_LIST *pkgList = new PACKAGE_LIST;
	SQLRecord sqlSearch;
	get_packagelist(&sqlSearch, pkgList);
	
	//say("Merging data\n");
	// Ищем соответствия
	int pkgNumber;
	for (int i=0; i<newPackages->size(); i++)
	{
		pkgNumber = pkgList->getPackageNumberByMD5(newPackages->get_package(i)->get_md5());
		
		if (pkgNumber!=-1)	// Если соответствие найдено...
		{
			pkgList->get_package(pkgNumber)->set_locations(newPackages->get_package(i)->get_locations());	// Записываем locations
		}
		else			// Если соответствие НЕ найдено...
		{
			newPackages->get_package(i)->newPackage=true;
			pkgList->add(newPackages->get_package(i));
		}
	}

	//say("Clean up...\n");
	// Вызываем синхронизацию данных.
	// Вообще говоря, ее можно было бы делать прямо здесь, но пусть таки будет универсальность.
	delete newPackages;//->clear();
	syncronize_data(pkgList);
	return 0;
}
int mpkgDatabase::syncronize_data(PACKAGE_LIST *pkgList)
{
	// Идея:
	// Добавить в базу пакеты, у которых флаг newPackage
	// Добавить locations к тем пакетам, которые такого флага не имеют
	// 
	// Алгоритм:
	// Бежим по списку пакетов.
	// 	Если пакет имеет влаг newPackage, то сразу добавляем его в базу функцией add_package_record()
	//	Если флага нету, то сразу добавляем ему locations функцией add_locationlist_record()
	// 
	for (int i=0; i<pkgList->size(); i++)
	{
		if (pkgList->get_package(i)->newPackage) add_package_record(pkgList->get_package(i));
		else add_locationlist_record(pkgList->get_package(i)->get_id(), pkgList->get_package(i)->get_locations());
	}
	delete pkgList;
	mDebug("Cleanup\n");

	// Дополнение от 10 мая 2007 года: сносим нафиг все недоступные пакеты, которые не установлены. Нечего им болтаться в базе.
	PACKAGE_LIST *allList = new PACKAGE_LIST;
	SQLRecord sqlSearch;
	mDebug("retrievin pkglist");
	get_packagelist(&sqlSearch, allList);
	mDebug("retrieved package list");
	PACKAGE_LIST deleteQueue;
	for (int i=0; i<allList->size(); i++)
	{
		if (!allList->get_package(i)->reachable(true))
		{
			deleteQueue.add(allList->get_package(i));
		}
	}
	mDebug("built delete queue");
	if (!deleteQueue.IsEmpty()) delete_packages(&deleteQueue);
	mDebug("removed wrong packages");
	delete allList;
	mDebug("deleted object, returning");
	return 0;

}

