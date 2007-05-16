/*********************************************************************
 * MOPSLinux packaging system: library interface
 * $Id: libmpkg.cpp,v 1.27 2007/05/16 01:15:58 i27249 Exp $
 * ******************************************************************/

#include "libmpkg.h"

mpkg::mpkg(bool _loadDatabase)
{
#ifdef APPLE_DEFINED
	//printf("Running MacOS X, welcome!\n");
#endif
#ifdef LINUX_DEFINED
	//printf("Running Linux, welcome!\n");
#endif
	debug("creating core");
	currentStatus="Loading database...";
	loadGlobalConfig();
	if (_loadDatabase)
	{
		db = new mpkgDatabase();
		DepTracker = new DependencyTracker(db);
	}
	init_ok=true;
	currentStatus = "Database loaded (mpkg constructor)";
}

mpkg::~mpkg()
{
	if (DepTracker!=NULL) delete DepTracker;
	if (db!=NULL) delete db;
	//printf("closing database...\n");
	delete_tmp_files();
}

string mpkg::current_status()
{
	return currentStatus;
}

int mpkg::clean_queue()
{
	return mpkgSys::clean_queue(db);
}

int mpkg::unqueue(int package_id)
{
	return mpkgSys::unqueue(package_id, db);
}
// Package building
int mpkg::build_package()
{
	return mpkgSys::build_package();
}

int mpkg::convert_directory(string output_dir)
{
	return mpkgSys::convert_directory(output_dir);
}
	
// Packages installation
int mpkg::install(vector<string> fname)
{
	//printf("function install\n");
	int ret=0;
	for (unsigned int i = 0; i < fname.size(); i++)
	{
		currentStatus = "Building queue: "+IntToStr(i) + "/" +IntToStr(fname.size()) +" ["+fname[i]+"]";
		if (mpkgSys::requestInstall(fname[i], db, DepTracker)!=0) ret--;
	}
	//currentStatus = "Installation complete";
	return ret;
}

int mpkg::install(string fname)
{
	currentStatus = "Building queue: "+fname;
	return mpkgSys::requestInstall(fname, db, DepTracker);
}

int mpkg::install(PACKAGE *pkg)
{
	return mpkgSys::requestInstall(pkg, db, DepTracker);
}

int mpkg::install(int package_id)
{
	return mpkgSys::requestInstall(package_id, db, DepTracker);
}

int mpkg::install(PACKAGE_LIST *pkgList)
{
	int ret=0;
	for (unsigned int i=0; i<pkgList->size(); i++)
	{
		if (mpkgSys::requestInstall(pkgList->get_package(i), db, DepTracker)!=0) ret--;
	}
	return ret;
}
// Packages removing
int mpkg::uninstall(vector<string> pkg_name)
{
	int ret=0;
	for (unsigned int i = 0; i < pkg_name.size(); i++)
	{
		currentStatus = "Building queue: "+IntToStr(i) + "/" +IntToStr(pkg_name.size()) +" ["+pkg_name[i]+"]";
		if (mpkgSys::requestUninstall(pkg_name[i], db, DepTracker)!=0) ret--;
	}
	return ret;
}

// Packages purging
int mpkg::purge(vector<string> pkg_name)
{
	int ret=0;
	for (unsigned int i = 0; i < pkg_name.size(); i++)
	{
		//printf("Filling purge queue...\n");
		currentStatus = "Building queue: "+IntToStr(i) + "/" +IntToStr(pkg_name.size()) +" ["+pkg_name[i]+"]";
		if (mpkgSys::requestUninstall(pkg_name[i], db, DepTracker, true)!=0) ret--;
	}
	return ret;
}
// Repository data updating
int mpkg::update_repository_data()
{
	if (mpkgSys::update_repository_data(db, DepTracker) == 0 && db->sqlFlush() == 0)
	{
		currentStatus = "Repository data updated";
		return 0;
	}
	else 
	{
		currentStatus = "Repository data update failed";
		return -1;
	}
}

// Cache cleaning
int mpkg::clean_cache()
{
	currentStatus = "Cleaning cache...";
	int ret = mpkgSys::clean_cache();
	if (ret == 0) currentStatus = "Cache cleanup complete";
	else currentStatus = "Error cleaning cache!";
	return ret;
}

// Package list retrieving
int mpkg::get_packagelist(SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo, bool ultraFast)
{
	currentStatus = "Retrieving package list...";
	int ret = db->get_packagelist(sqlSearch, packagelist, GetExtraInfo, ultraFast);
	if (ret == 0) currentStatus = "Retriveal complete";
	else currentStatus = "Failed retrieving package list!";
	return ret;
}

// Configuration and settings: retrieving
vector<string> mpkg::get_repositorylist()
{
	return mpkgconfig::get_repositorylist();
}
vector<string> mpkg::get_disabled_repositorylist()
{
	return mpkgconfig::get_disabled_repositorylist();
}

string mpkg::get_sysroot()
{
	return mpkgconfig::get_sysroot();
}
string mpkg::get_syscache()
{
	return mpkgconfig::get_syscache();
}
string mpkg::get_dburl()
{
	return mpkgconfig::get_dburl();
}

string mpkg::get_cdromdevice()
{
	return mpkgconfig::get_cdromdevice();
}

string mpkg::get_cdrommountpoint()
{
	return mpkgconfig::get_cdrommountpoint();
}

string mpkg::get_scriptsdir()
{
	return mpkgconfig::get_scriptsdir();
}
bool mpkg::get_runscripts()
{
	return mpkgconfig::get_runscripts();
}

unsigned int mpkg::get_checkFiles()
{
	return mpkgconfig::get_checkFiles();
}

int mpkg::set_checkFiles(unsigned int value)
{
	return mpkgconfig::set_checkFiles(value);
}


// Configuration and settings: setting
int mpkg::set_repositorylist(vector<string> newrepositorylist, vector<string> drList)
{
	return mpkgconfig::set_repositorylist(newrepositorylist, drList);
}
int mpkg::set_sysroot(string newsysroot)
{
	return mpkgconfig::set_sysroot(newsysroot);
}

int mpkg::set_syscache(string newsyscache)
{
	return mpkgconfig::set_syscache(newsyscache);
}
int mpkg::set_dburl(string newdburl)
{
	return mpkgconfig::set_dburl(newdburl);
}

int mpkg::set_cdromdevice(string cdromDevice)
{
	return mpkgconfig::set_cdromdevice(cdromDevice);
}

int mpkg::set_cdrommountpoint(string cdromMountPoint)
{
	return mpkgconfig::set_cdrommountpoint(cdromMountPoint);
}


int mpkg::set_scriptsdir(string newscriptsdir)
{
	return mpkgconfig::set_scriptsdir(newscriptsdir);
}
int mpkg::set_runscripts(bool dorun)
{
	return mpkgconfig::set_runscripts(dorun);
}

// Finalizing
int mpkg::commit()
{
	currentStatus = "Checking dependencies...";
	int errorCount = DepTracker->renderData();
	if (errorCount==0)
	{
		DepTracker->commitToDb();
		currentStatus = "Committing changes...";
		db->commit_actions();
		currentStatus = "Complete.";
	}
	else
	{
		//mpkgErrorReturn errRet = waitResponce(MPKG_DEPENDENCY_ERROR);
		printf("Error in dependencies: %d failures \n", errorCount);
		currentStatus = "Failed - depencency errors";
	}
	return 0;
}

