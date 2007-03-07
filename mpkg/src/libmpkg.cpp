/*********************************************************************
 * MOPSLinux packaging system: library interface
 * $Id: libmpkg.cpp,v 1.5 2007/03/07 07:02:36 i27249 Exp $
 * ******************************************************************/

#include "libmpkg.h"

mpkg::mpkg()
{
#ifdef APPLE_DEFINED
	printf("Running MacOS X, welcome!\n");
#endif
#ifdef LINUX_DEFINED
	printf("Running Linux, welcome!\n");
#endif
	debug("creating core");
	current_status="Loading database...";
	loadGlobalConfig();
	db = new mpkgDatabase();
	DepTracker = new DependencyTracker(db);
	init_ok=true;
}

mpkg::~mpkg()
{
	delete DepTracker;
	delete db;
	delete_tmp_files();
}

int mpkg::clean_queue()
{
	return mpkgSys::clean_queue(db);
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
	int ret=0;
	for (unsigned int i = 0; i < fname.size(); i++)
	{
		ret+=mpkgSys::install(fname[i], db, DepTracker);
	}
	return ret;
}

int mpkg::install(string fname)
{
	vector<string> fname2;
	fname2.push_back(fname);
	return install(fname2);
}

// Packages removing
int mpkg::uninstall(vector<string> pkg_name)
{
	int ret=0;
	for (unsigned int i = 0; i < pkg_name.size(); i++)
	{
		ret+=mpkgSys::uninstall(pkg_name[i], db, DepTracker, 0);
	}
	return ret;
}

// Packages purging
int mpkg::purge(vector<string> pkg_name)
{
	int ret=0;
	for (unsigned int i = 0; i < pkg_name.size(); i++)
	{
		ret+=mpkgSys::uninstall(pkg_name[i], db, DepTracker, 1);
	}
	return ret;
}

// Packages upgrading
int mpkg::upgrade (vector<string> pkgname)
{
	int ret=0;
	for (unsigned int i = 0; i < pkgname.size(); i++)
	{
		ret+=mpkgSys::upgrade(pkgname[i], db, DepTracker);
	}
	return ret;
}	
		
// Repository data updating
int mpkg::update_repository_data()
{
	// FIXME: running twice due to unresolved bug in updating mechanism.
	mpkgSys::update_repository_data(db, DepTracker);
	return mpkgSys::update_repository_data(db, DepTracker);
}

// Cache cleaning
int mpkg::clean_cache()
{
	return mpkgSys::clean_cache();
}

// Package list retrieving
int mpkg::get_packagelist(SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo)
{
	return db->get_packagelist(sqlSearch, packagelist, GetExtraInfo);
}

// Configuration and settings: retrieving
vector<string> mpkg::get_repositorylist()
{
	return mpkgconfig::get_repositorylist();
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
string mpkg::get_scriptsdir()
{
	return mpkgconfig::get_scriptsdir();
}
bool mpkg::get_runscripts()
{
	return mpkgconfig::get_runscripts();
}

// Configuration and settings: setting
int mpkg::set_repositorylist(vector<string> newrepositorylist)
{
	return mpkgconfig::set_repositorylist(newrepositorylist);
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
	printf("committing...\n");
	DepTracker->commitToDb();
	printf("commit = %d\n", db->commit_actions());
	return 0;
}

