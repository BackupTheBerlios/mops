/***********************************************************************
 * 	$Id: mpkg.cpp,v 1.75 2007/05/16 12:45:52 i27249 Exp $
 * 	MOPSLinux packaging system
 * ********************************************************************/
#include "mpkg.h"
#include "syscommands.h"
#include "DownloadManager.h"

/** Scans database and do actions. Actually, packages will install in SYS_ROOT folder.
 * In real (mean installed) systems, set SYS_ROOT to "/"
 * @**/


mpkgDatabase::mpkgDatabase()
{
}
mpkgDatabase::~mpkgDatabase(){}

int mpkgDatabase::sqlFlush()
{
	return db.sqlFlush();
}

PACKAGE mpkgDatabase::get_installed_package(string pkg_name)
{
	PACKAGE_LIST packagelist;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", pkg_name);
	sqlSearch.addField("package_installed", IntToStr(ST_INSTALLED));

	get_packagelist(sqlSearch, &packagelist);
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
	debug("mpkgDatabase::emerge_to_db()");
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
	LOCATION_LIST new_locations;
	get_package(pkg_id, &db_package, true);
	package->set_id(pkg_id);

	for (int j=0; j<package->get_locations()->size(); j++)
	{
		for (int i=0; i<db_package.get_locations()->size(); i++)
		{
			if (package->get_locations()->get_location(j)->get_server()!=db_package.get_locations()->get_location(i)->get_server() || \
					package->get_locations()->get_location(j)->get_path()!=db_package.get_locations()->get_location(i)->get_path())
			{
				new_locations.add(*package->get_locations()->get_location(j));
			}
		}
	}
	if (!new_locations.IsEmpty()) add_locationlist_record(pkg_id, &new_locations);
	return 0;
}


bool mpkgDatabase::check_cache(PACKAGE *package, bool clear_wrong)
{
	string fname = SYS_CACHE+"/"+package->get_filename();
	string got_md5 = get_file_md5(SYS_CACHE + "/" + package->get_filename());
	if (FileExists(fname) && package->get_md5() == got_md5)
		return true;
	else
	{
		if (clear_wrong)unlink(fname.c_str());
		return false;
	}
}


int mpkgDatabase::commit_actions()
{
	sqlFlush();
	// Zero: purging required packages
	// First: removing required packages

	PACKAGE_LIST remove_list;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", IntToStr(ST_REMOVE));
	sqlSearch.addField("package_action", IntToStr(ST_PURGE));
	if (get_packagelist(sqlSearch, &remove_list)!=0) return -1;
	remove_list.sortByPriority(true);
	PACKAGE_LIST install_list;
	sqlSearch.clear();
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", IntToStr(ST_INSTALL));
	if (get_packagelist(sqlSearch, &install_list,false)!=0) return -5;
	install_list.sortByPriority();

	for (int i=0; i<remove_list.size(); i++)
	{
		remove_list.get_package(i)->itemID = pData.addItem(remove_list.get_package(i)->get_name(), 10);
	}
	for (int i=0; i<install_list.size(); i++)
	{
		install_list.get_package(i)->itemID = pData.addItem(install_list.get_package(i)->get_name(), atoi(install_list.get_package(i)->get_compressed_size().c_str()));
	}
	
	// Building action list
	actionBus.clear();
	if (remove_list.size()>0)
	{
		actionBus.addAction(ACTIONID_REMOVE);
		actionBus.actions.at(actionBus.getActionPosition(ACTIONID_REMOVE))._progressMaximum=remove_list.size();
	}
	if (install_list.size()>0)
	{
		actionBus.addAction(ACTIONID_CACHECHECK);
		actionBus.setSkippable(ACTIONID_CACHECHECK, true);
		actionBus.actions.at(actionBus.getActionPosition(ACTIONID_CACHECHECK))._progressMaximum=install_list.size();
		actionBus.addAction(ACTIONID_DOWNLOAD);
		actionBus.actions.at(actionBus.getActionPosition(ACTIONID_DOWNLOAD))._progressMaximum=install_list.size();
		actionBus.addAction(ACTIONID_MD5CHECK);
		actionBus.setSkippable(ACTIONID_MD5CHECK, true);
		actionBus.actions.at(actionBus.getActionPosition(ACTIONID_MD5CHECK))._progressMaximum=install_list.size();
		actionBus.addAction(ACTIONID_INSTALL);
		actionBus.actions.at(actionBus.getActionPosition(ACTIONID_INSTALL))._progressMaximum=install_list.size();

	}
	// Done

	if (remove_list.size()>0)
	{
		actionBus.setCurrentAction(ACTIONID_REMOVE);

		currentStatus = "Looking for remove queue";
			debug ("Calling REMOVE for "+IntToStr(remove_list.size())+" packages");
		currentStatus = "Removing " + IntToStr(remove_list.size()) + " packages";
		pData.setCurrentAction("Removing packages");
	
		int removeItemID=0;
		for (int i=0; i<remove_list.size(); i++)
		{
			removeItemID=remove_list.get_package(i)->itemID;
			pData.setItemState(removeItemID, ITEMSTATE_WAIT);
			pData.setItemCurrentAction(removeItemID, "Waiting");
			pData.resetIdleTime(removeItemID);
			pData.setItemProgress(removeItemID, 0);
			pData.setItemProgressMaximum(removeItemID,8);
		}
	
		for (int i=0;i<remove_list.size();i++)
		{
			actionBus.actions.at(actionBus.currentProcessing())._currentProgress=i;
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_REMOVE, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			currentStatus = "Removing package " + remove_list.get_package(i)->get_name();
			if (remove_package(remove_list.get_package(i))!=0)
			{
				pData.setItemCurrentAction(remove_list.get_package(i)->itemID, "Remove failed");
				pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_FAILED);
			}
			else
			{
				pData.setItemCurrentAction(remove_list.get_package(i)->itemID, "Removed");
				pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}
		
		}
		sqlSearch.clear();

		clean_backup_directory();
		actionBus.setActionState(ACTIONID_REMOVE);
	}


	currentStatus = "Looking for install queue";

	if (install_list.size()>0)
	{
		actionBus.setCurrentAction(ACTIONID_CACHECHECK);
		// Building download queue
		currentStatus = "Looking for package locations...";
		DownloadsList downloadQueue;
		DownloadItem tmpDownloadItem;
		vector<string> itemLocations;
		double totalDownloadSize=0;
		pData.resetItems("waiting", 0, 1, ITEMSTATE_WAIT);
		pData.setCurrentAction("Checking cache");
		bool skip=false;
		for (int i=0; i<install_list.size(); i++)
		{

			actionBus.actions.at(actionBus.currentProcessing())._currentProgress=i;
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
			pData.setItemCurrentAction(install_list.get_package(i)->itemID, "Checking cache");
			pData.setItemProgressMaximum(install_list.get_package(i)->itemID, 1);
			pData.setItemProgress(install_list.get_package(i)->itemID, 0);
	
			currentStatus = "Checking cache and building download queue: " + install_list.get_package(i)->get_name();
	
	
			if (skip || !check_cache(install_list.get_package(i), false))
			{
				if (!skip) pData.setItemCurrentAction(install_list.get_package(i)->itemID, "not cached");
				else pData.setItemCurrentAction(install_list.get_package(i)->itemID, "check skipped");

				itemLocations.clear();
				
				tmpDownloadItem.expectedSize=strtod(install_list.get_package(i)->get_compressed_size().c_str(), NULL);
				tmpDownloadItem.file = SYS_CACHE + install_list.get_package(i)->get_filename();
				tmpDownloadItem.name = install_list.get_package(i)->get_name();
				tmpDownloadItem.priority = 0;
				tmpDownloadItem.status = DL_STATUS_WAIT;
				tmpDownloadItem.itemID = install_list.get_package(i)->itemID;
	
				install_list.get_package(i)->sortLocations();
				for (int k = 0; k < install_list.get_package(i)->get_locations()->size(); k++)
				{
					itemLocations.push_back(install_list.get_package(i)->get_locations()->get_location(k)->get_server()->get_url() \
						     + install_list.get_package(i)->get_locations()->get_location(k)->get_path() \
						     + install_list.get_package(i)->get_filename());
	
				}
				tmpDownloadItem.url_list = itemLocations;
				downloadQueue.push_back(tmpDownloadItem);
			}
			else pData.setItemCurrentAction(install_list.get_package(i)->itemID, "cached");
	
			pData.increaseItemProgress(install_list.get_package(i)->itemID);
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
		}
		actionBus.setActionState(ACTIONID_CACHECHECK);
		actionBus.setCurrentAction(ACTIONID_DOWNLOAD);
		mpkgErrorReturn errRet;
		bool do_download = true;
	
		pData.resetItems("waiting", 0, 1, ITEMSTATE_WAIT);
		double dlProgress;
		double dlProgressMax;
		while(do_download)
		{
			do_download = false;
			pData.downloadAction=true;
			if (CommonGetFileEx(downloadQueue, &currentItem, &actionBus, &pData) == DOWNLOAD_ERROR)
			{
				printf("Download failed (returned DOWNLOAD_ERROR), waiting responce\n");
				errRet = waitResponce (MPKG_DOWNLOAD_ERROR);
				switch(errRet)
				{
					case MPKG_RETURN_IGNORE:
						printf("Download errors ignored, continue installing\n");
						goto installProcess;
						break;
				
					case MPKG_RETURN_RETRY:
						printf("retrying...\n");
						do_download = true;
						break;
					case MPKG_RETURN_ABORT:
						printf("aborting...\n");
						return -100;
						break;
					default:
						printf("Unknown value, don't know what to do\n");
				}
					
			}
		
		}
		actionBus.setActionState(ACTIONID_DOWNLOAD);
		pData.downloadAction=false;
installProcess:
	
		actionBus.setCurrentAction(ACTIONID_MD5CHECK);
		pData.resetItems("waiting", 0, 1, ITEMSTATE_WAIT);
	
		bool hasErrors=false;
		skip=false;
		currentStatus = "Checking files (comparing MD5):";
		pData.setCurrentAction("Checking md5");
		for (int i=0; i<install_list.size(); i++)
		{
			actionBus.actions.at(actionBus.currentProcessing())._currentProgress=i;
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_MD5CHECK, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			if (actionBus.skipped(ACTIONID_MD5CHECK)) break;

			printf("Checking MD5 for %s\n", install_list.get_package(i)->get_filename().c_str());
			currentStatus = "Checking md5 of downloaded files: " + install_list.get_package(i)->get_name();
	
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			pData.setItemCurrentAction(install_list.get_package(i)->itemID, "Checking md5");
			pData.setItemProgressMaximum(install_list.get_package(i)->itemID, 1);
			pData.setItemProgress(install_list.get_package(i)->itemID, 0);
	
			if (!check_cache(install_list.get_package(i), true))
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, "md5 failed");
				pData.increaseItemProgress(install_list.get_package(i)->itemID);
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FAILED);
	
				errRet = waitResponce(MPKG_DOWNLOAD_ERROR);
				switch(errRet)
				{
					case MPKG_RETURN_IGNORE:
						printf("Wrong checksum ignored, continuing...\n");
						break;
					case MPKG_RETURN_RETRY:
						printf("Re-downloading...\n");
						break;
					case MPKG_RETURN_ABORT:
						printf("Aborting installation\n");
						return -100;
						break;
					default:
						printf("Unknown... aborting\n");
						return -120;
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
		actionBus.setActionState(ACTIONID_MD5CHECK);
	
		actionBus.setCurrentAction(ACTIONID_INSTALL);
		pData.resetItems("waiting", 0, 1, ITEMSTATE_WAIT);
	
		pData.setCurrentAction("Installing packages");
		int installItemID;
		for (int i=0; i<install_list.size(); i++)
		{
			installItemID=install_list.get_package(i)->itemID;
			pData.setItemState(installItemID, ITEMSTATE_WAIT);
			pData.setItemCurrentAction(installItemID, "Waiting");
			pData.resetIdleTime(installItemID);
			pData.setItemProgress(installItemID, 0);
			pData.setItemProgressMaximum(installItemID,8);
		}
		for (int i=0;i<install_list.size();i++)
		{

			actionBus.actions.at(actionBus.currentProcessing())._currentProgress=i;
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			currentStatus = "Installing package " + install_list.get_package(i)->get_name();
			if (install_package(install_list.get_package(i))!=0)
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, "Installation failed");
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FAILED);
			}
			else
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, "Installed");
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}
		}
		currentStatus = "Installation complete.";
		actionBus.setActionState(ACTIONID_INSTALL);
	}
	actionBus.clear();
	return 0;
}

int mpkgDatabase::install_package(PACKAGE* package)
{

	pData.setItemCurrentAction(package->itemID, "installing");
	printf("Installing %s %s\n",package->get_name().c_str(), package->get_fullversion().c_str());
	string statusHeader = "["+IntToStr((int) actionBus.progress())+"/"+IntToStr((int)actionBus.progressMaximum())+"] "+"Installing package "+package->get_name()+": ";
	currentStatus = statusHeader + "initialization";
	// First of all: EXTRACT file list and scripts!!!
	LocalPackage lp(SYS_CACHE+package->get_filename());
	bool no_purge=true;
	FILE_LIST old_config_files;
	int purge_id=get_purge(package->get_name()); // returns package id if this previous package config files are not removed, or 0 if purged.
	debug("purge_id="+IntToStr(purge_id));
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

	currentStatus = statusHeader + "extracting installation scripts";
	pData.increaseItemProgress(package->itemID);
	lp.fill_scripts(package);
	currentStatus = statusHeader + "extracting file list";
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	lp.fill_filelist(package);
	currentStatus = statusHeader + "detecting configuration files";
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	lp.fill_configfiles(package);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	if (fileConflictChecking==CHECKFILES_PREINSTALL) 
	{
		currentStatus = statusHeader + "checking file conflicts";
		pData.increaseItemProgress(package->itemID);

	}

	if (fileConflictChecking == CHECKFILES_PREINSTALL && check_file_conflicts(package)!=0)
	{
		currentStatus = "Error: Unresolved file conflict on package "+package->get_name();
		printf("Unresolved file conflict on package %s, it will be skipped!\n", package->get_name().c_str());
		return -5;
	}
	
	add_scripts_record(package->get_id(), package->get_scripts()); // Add paths to scripts to database
	currentStatus = statusHeader + "installing...";
	pData.increaseItemProgress(package->itemID);


// Filtering file list...
	FILE_LIST package_files;
	if (!no_purge) add_filelist_record(package->get_id(), package->get_files());
	string sys;
	debug("Preparing scripts");
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
		currentStatus = statusHeader + "executing pre-install scripts";
		if (FileExists(package->get_scripts()->get_preinstall()))
		{
			string preinst="cd " + SYS_ROOT + " ; sh "+package->get_scripts()->get_preinstall();
			system(preinst.c_str());
		}
	}

	// Extracting package
	currentStatus = statusHeader + "extracting...";
	pData.increaseItemProgress(package->itemID);


	debug("calling extract");
	string sys_cache=SYS_CACHE;
	string sys_root=SYS_ROOT;
	string create_root="mkdir -p "+sys_root+" 2>/dev/null";
	system(create_root.c_str());
	sys="(cd "+sys_root+"; tar zxf "+sys_cache+package->get_filename()+" --exclude install";
	//If previous version isn't purged, do not overwrite config files
	if (no_purge)
	{
		debug("retrieving old config files");
		get_filelist(purge_id, &old_config_files, true);
		debug("no_purge flag IS SET, config files count = "+IntToStr(package->get_config_files()->size()));
		for (int i=0; i<package->get_config_files()->size(); i++)
		{
			// Writing new config files, skipping old
			for (int k=0; k < old_config_files.size(); k++)
			{
				if (package->get_config_files()->get_file(i)->get_name()==old_config_files.get_file(k)->get_name())
				{
					debug("excluding file "+package->get_config_files()->get_file(i)->get_name());
					sys+=" --exclude "+package->get_config_files()->get_file(i)->get_name();
				}
			}
		}
		for (int i=0; i<package->get_files()->size(); i++)
		{
			for (int k=0; k <= old_config_files.size(); k++)
			{
				if (k==old_config_files.size()) 
				{
					package_files.add(*package->get_files()->get_file(i));
					break;
				}
				if (package->get_files()->get_file(i)->get_name()==old_config_files.get_file(k)->get_name())
				{
					debug("Skipping file "+package->get_files()->get_file(i)->get_name());
					break;
				}
			}
		}
		debug("package_files size: "+IntToStr(package_files.size())+", package->get_files size: "+IntToStr(package->get_files()->size()));
		add_filelist_record(package->get_id(), &package_files);
	}
	sys+=" > /dev/null)";
	if (system(sys.c_str()) == 0) currentStatus = statusHeader + "executing post-install scripts...";
	else {
		currentStatus = "Failed to extract!";
		return -10;
	}
	pData.increaseItemProgress(package->itemID);


	
	// Creating and running POST-INSTALL script
	if (!DO_NOT_RUN_SCRIPTS)
	{
		if (FileExists(package->get_scripts()->get_postinstall()))
		{
			string postinst="cd " + SYS_ROOT + " ; sh "+package->get_scripts()->get_postinstall();
			system(postinst.c_str());
		}
	}

	set_installed(package->get_id(), ST_INSTALLED);
	set_configexist(package->get_id(), ST_CONFIGEXIST);
	set_action(package->get_id(), ST_NONE);
	sqlFlush();
	debug("*********************************************\n*        Package installed sussessfully     *\n*********************************************");
	pData.increaseItemProgress(package->itemID);


	return 0;
}	//End of install_package

int mpkgDatabase::purge_package(PACKAGE* package)
{
	printf("%s is deprecated, use remove_package instead\n", __func__);
	return remove_package(package);
}

int mpkgDatabase::remove_package(PACKAGE* package)
{
	pData.setItemProgressMaximum(package->itemID, package->get_files()->size()+8);
	pData.setItemCurrentAction(package->itemID, "removing");

	printf("Removing package %s %s...\n",package->get_name().c_str(), package->get_fullversion().c_str());

	string statusHeader = "["+IntToStr((int)actionBus.progress())+"/"+IntToStr((int)actionBus.progressMaximum())+"] "+"Removing package "+package->get_name()+": ";
	currentStatus = statusHeader + "initialization";
	
	if (package->action()==ST_REMOVE || package->action()==ST_PURGE)
	{
		// Running pre-remove scripts
		debug("REMOVE PACKAGE::Preparing scripts");
		if(!DO_NOT_RUN_SCRIPTS)
		{
			if (FileExists(package->get_scripts()->get_preremove()))
			{
				currentStatus = statusHeader + "executing pre-remove scripts";
				string prerem="cd " + SYS_ROOT + " ; sh "+package->get_scripts()->get_preremove();
				system(prerem.c_str());
			}
		}
		pData.increaseItemProgress(package->itemID);
		// removing package
		debug("calling remove");
		string sys_cache=SYS_CACHE;
		string sys_root=SYS_ROOT;
		string fname;
		debug("Package has "+IntToStr(package->get_files()->size())+" files");

		// Purge is now implemented here; checking all
		currentStatus = statusHeader + "building file list";
		FILE_LIST *remove_files=package->get_files();
		currentStatus = statusHeader + "removing files...";
		bool removeThis;
		for (int i=0; i<remove_files->size(); i++)
		{
			removeThis = false;
			if (package->action()==ST_PURGE || remove_files->get_file(i)->get_type()==FTYPE_PLAIN) removeThis = true;
			fname=sys_root + remove_files->get_file(i)->get_name(false);
			if (removeThis && fname[fname.length()-1]!='/')
			{
				pData.increaseItemProgress(package->itemID);
				unlink (fname.c_str());
			}
		}

		currentStatus = statusHeader + "removing empty directories...";
	
		// Run 2: clearing empty directories
		vector<string>empty_dirs;
		string edir;
		pData.increaseItemProgress(package->itemID);
		for (int i=0; i<remove_files->size(); i++)
		{
			fname=sys_root + remove_files->get_file(i)->get_name(false);
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
				rmdir(empty_dirs[x].c_str());
			}
			edir.clear();
			empty_dirs.clear();
		}
	
		// Creating and running POST-INSTALL script
		if (!DO_NOT_RUN_SCRIPTS)
		{
			if (FileExists(package->get_scripts()->get_postremove()))
			{
				currentStatus = statusHeader + "executing post-removal scripts";
				string postrem="cd " + SYS_ROOT + " ; sh "+package->get_scripts()->get_postremove();
				system(postrem.c_str());
			}
		}
	}
	// Restoring backups
	FILE_LIST restore = get_conflict_records(package->get_id());
	if (!restore.IsEmpty())
	{
		string cmd;
		string tmpName;
		for (int i=0; i<restore.size(); i++)
		{
			cmd = "mkdir -p ";
		       	cmd += SYS_ROOT + restore.get_file(i)->backup_file.substr(0, restore.get_file(i)->backup_file.find_last_of("/"));
			system(cmd.c_str());
			cmd = "mv ";
		        cmd += restore.get_file(i)->backup_file + " ";
			tmpName = restore.get_file(i)->backup_file.substr(strlen(SYS_BACKUP));
			tmpName = tmpName.substr(tmpName.find("/"));
		        cmd += SYS_ROOT	+ tmpName;
			system(cmd.c_str());
			delete_conflict_record(package->get_id(), restore.get_file(i)->backup_file);
		}
	}

	pData.increaseItemProgress(package->itemID);
	set_installed(package->get_id(), ST_NOTINSTALLED);
	set_action(package->get_id(), ST_NONE);
	currentStatus = statusHeader + "cleaning file list";
	pData.increaseItemProgress(package->itemID);
	cleanFileList(package->get_id());
	pData.increaseItemProgress(package->itemID);
	sqlFlush();
	currentStatus = statusHeader + "remove complete";
	debug("*********************************************\n*        Package removed sussessfully     *\n*********************************************");
	return 0;
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
		sqlSearch.addField("package_id", IntToStr(pkgList->get_package(i)->get_id()));
	}
	db.sql_delete("packages", sqlSearch);
	sqlSearch.clear();
	for (int i=0; i<pkgList->size(); i++)
	{
		sqlSearch.addField("packages_package_id", IntToStr(pkgList->get_package(i)->get_id()));
	}
	db.sql_delete("tags_links", sqlSearch);
	db.sql_delete("dependencies", sqlSearch);
#ifdef ENABLE_INTERNATIONAL
	db.sql_delete("descriptions", sqlSearch);
	db.sql_delete("changelogs", sqlSearch);
	db.sql_delete("ratings", sqlSearch);
#endif
	return 0;
}



int mpkgDatabase::cleanFileList(int package_id)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	if (get_configexist(package_id)) sqlSearch.addField("file_type", IntToStr(FTYPE_PLAIN));
	return db.sql_delete("files", sqlSearch);
}

int mpkgDatabase::update_package_data(int package_id, PACKAGE *package)
{
	PACKAGE old_package;
	if (get_package(package_id, &old_package, false)!=0)
	{
		debug("mpkg.cpp: update_package_data(): get_package error: no package or error while querying database");
		return -1;
	}
	
	SQLRecord sqlUpdate;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));

	debug("mpkg.cpp: update_package_data(): updating direct package data");
	// 1. Updating direct package data
	if (package->get_md5()!=old_package.get_md5())
	{
		debug("mpkg.cpp: update_package_data(): md5 mismatch, updating description fields");
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
	if (package->get_filename()!=old_package.get_filename())
	{
		debug("mpkg.cpp: update_package_data(): filename mismatch, updating");
		sqlUpdate.addField("package_filename", package->get_filename());
	}

	// 3. Updating status. Seems that somewhere here is an error causing double scan is required
	sqlUpdate.addField("package_available", IntToStr(package->available()));


	// 4. Updating locations
	
	// Note about method used for updating locations:
	// If locations are not identical:
	// 	Step 1. Remove all existing package locations from "locations" table. Servers are untouched.
	// 	Step 2. Add new locations.
	// Note: after end of updating procedure for all packages, it will be good to do servers cleanup - delete all servers who has no locations.
	debug("mpkg.cpp: update_package_data(): checking locations");	
	if (*package->get_locations()!=*old_package.get_locations())
	{
		debug("mpkg.cpp: update_package_data(): locations mismatch, cleanup");
		debug("mpkg.cpp: update_package_data(): old has "+IntToStr(old_package.get_locations()->size())+" locations, but new has "+\
				IntToStr(package->get_locations()->size())+" ones");
		SQLRecord loc_sqlDelete;
		loc_sqlDelete.addField("packages_package_id", IntToStr(package_id));
		debug("mpkg.cpp: update_package_data(): deleting old locations relating this package");
		int sql_del=db.sql_delete("locations", loc_sqlDelete);
		if (sql_del!=0)
		{
			debug("mpkg.cpp: update_package_data(): unable to delete old locations: SQL error "+IntToStr(sql_del));
			return -2;
		}
		if (add_locationlist_record(package_id, package->get_locations())!=0)
		{
			debug("mpkg.cpp: update_package_data(): unable to add new locations: error in add_locationlist_record()");
			return -3;
		}
		if (package->get_locations()->IsEmpty())
		{
			sqlUpdate.addField("package_available", IntToStr(ST_NOTAVAILABLE));
		}
	}

	// 5. Updating tags
	if (*package->get_tags()!=*old_package.get_tags())
	{
		SQLRecord taglink_sqlDelete;
		taglink_sqlDelete.addField("packages_package_id", IntToStr(package_id));

		if (db.sql_delete("tags_links", taglink_sqlDelete)!=0) return -4;
		if (add_taglist_record(package_id, package->get_tags())!=0) return -5;
	}

	// 6. Updating dependencies
	if (*package->get_dependencies()!=*old_package.get_dependencies())
	{
		SQLRecord dep_sqlDelete;
		dep_sqlDelete.addField("packages_package_id", IntToStr(package_id));

		if(db.sql_delete("dependencies", dep_sqlDelete)!=0) return -6;
		if (add_dependencylist_record(package_id, package->get_dependencies())!=0) return -7;
	}

	// 7, 8 - update scripts and file list. It is skipped for now, because we don't need this here (at least, for now).
	if (!sqlUpdate.empty())
	{
		if (db.sql_update("packages", sqlUpdate, sqlSearch)!=0)
		{
			debug("mpkg.cpp: update_package_data(): db.sql_update failed");
			return -8;
		}
	}
	else debug("mpkg.cpp: update_package_data(): sqlUpdate empty, nothing to update");
	debug("mpkg.cpp: update_package_data(): successful end");
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
	printf("Processing\n");	
	// Стираем locations и servers
	db.clear_table("locations");
	db.clear_table("servers");

	// Забираем текущий список пакетов
	PACKAGE_LIST *pkgList = new PACKAGE_LIST;
	SQLRecord sqlSearch;
	get_packagelist(sqlSearch, pkgList, false);
	
	printf("Comparing\n");
	// Ищем соответствия
	int pkgNumber;
	for (int i=0; i<newPackages->size(); i++)
	{
		pkgNumber = pkgList->getPackageNumberByMD5(newPackages->get_package(i)->get_md5());
		
		if (pkgNumber!=-1)	// Если соответствие найдено...
		{
			pkgList->get_package(pkgNumber)->set_locations(*newPackages->get_package(i)->get_locations());	// Записываем locations
		}
		else			// Если соответствие НЕ найдено...
		{
			newPackages->get_package(i)->newPackage=true;
			pkgList->add(*newPackages->get_package(i));
		}
	}

	printf("Syncing\n");
	// Вызываем синхронизацию данных.
	// Вообще говоря, ее можно было бы делать прямо здесь, но пусть таки будет универсальность.
	delete newPackages;//->clear();
	syncronize_data(pkgList);
	printf("Done\n");
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
	printf("Cleanup (pause here)\n");
	sleep(1000);

	// Дополнение от 10 мая 2007 года: сносим нафиг все недоступные пакеты, которые не установлены. Нечего им болтаться в базе.
	PACKAGE_LIST allList;
	SQLRecord sqlSearch;
	get_packagelist(sqlSearch, &allList, false);
	PACKAGE_LIST deleteQueue;
	for (int i=0; i<allList.size(); i++)
	{
		if (!allList.get_package(i)->reachable(true))
		{
			deleteQueue.add(*allList.get_package(i));
		}
	}
	if (!deleteQueue.IsEmpty()) delete_packages(&deleteQueue);
	return 0;

}

