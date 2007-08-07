/*********************************************************************
 * MOPSLinux packaging system: library interface
 * $Id: libmpkg.cpp,v 1.42 2007/08/07 17:26:46 i27249 Exp $
 * ******************************************************************/

#include "libmpkg.h"

mpkg::mpkg(bool _loadDatabase)
{
	if (!consoleMode && !dialogMode) initErrorManager(EMODE_QT);
	if (consoleMode && dialogMode) initErrorManager(EMODE_DIALOG);
	if (consoleMode && !dialogMode) initErrorManager(EMODE_CONSOLE);

	mDebug("creating core");
	currentStatus=_("Loading database...");
	loadGlobalConfig();
	db=NULL;
	DepTracker=NULL;
	if (_loadDatabase)
	{
		mDebug("Loading database");
		db = new mpkgDatabase();
		DepTracker = new DependencyTracker(db);
	}
	init_ok=true;
	currentStatus = _("Database loaded");
}

mpkg::~mpkg()
{
	printf("Desctructing mpkg\n");
	if (DepTracker!=NULL) delete DepTracker;
	if (db!=NULL) delete db;
	delete_tmp_files();
	mpkgSys::clean_cache(true);
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

void mpkg::get_available_tags(vector<string>* output)
{
	db->get_available_tags(output);
}
	
// Packages installation
int mpkg::install(vector<string> fname)
{
	int ret=0;
	for (unsigned int i = 0; i < fname.size(); i++)
	{
		currentStatus = _("Building queue: ")+IntToStr(i) + "/" +IntToStr(fname.size()) +" ["+fname[i]+"]";
		if (mpkgSys::requestInstall(fname[i], db, DepTracker)!=0) ret--;
	}
	//currentStatus = "Installation complete";
	return ret;
}
int mpkg::installGroups(vector<string> groupName)
{
	for (unsigned int i=0; i<groupName.size(); i++)
	{
		mpkgSys::requestInstallGroup(groupName[i], db, DepTracker);
	}
	return 0;
}
int mpkg::install(string fname)
{
	currentStatus = _("Building queue: ")+fname;
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
	for (int i=0; i<pkgList->size(); i++)
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
		//printf("[%d] REMOVING %s\n", i, pkg_name[i].c_str());
		currentStatus = _("Building queue: ")+IntToStr(i) + "/" +IntToStr(pkg_name.size()) +" ["+pkg_name[i]+"]";
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
		currentStatus = _("Building queue: ")+IntToStr(i) + "/" +IntToStr(pkg_name.size()) +" ["+pkg_name[i]+"]";
		if (mpkgSys::requestUninstall(pkg_name[i], db, DepTracker, true)!=0) ret--;
	}
	return ret;
}
// Repository data updating
int mpkg::update_repository_data()
{
	if (mpkgSys::update_repository_data(db) == 0 && db->sqlFlush() == 0)
	{
		currentStatus = _("Repository data updated");
		return 0;
	}
	else 
	{
		currentStatus = _("Repository data update failed");
		return -1;
	}
}

// Cache cleaning
int mpkg::clean_cache(bool clean_symlinks)
{
	currentStatus = _("Cleaning cache...");
	int ret = mpkgSys::clean_cache(clean_symlinks);
	if (ret == 0) currentStatus = _("Cache cleanup complete");
	else currentStatus = _("Error cleaning cache!");
	return ret;
}

// Package list retrieving
int mpkg::get_packagelist(SQLRecord *sqlSearch, PACKAGE_LIST *packagelist)
{
	currentStatus = _("Retrieving package list...");
	int ret = db->get_packagelist(sqlSearch, packagelist);
	if (ret == 0) currentStatus = _("Retriveal complete");
	else currentStatus = _("Failed retrieving package list!");
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
	mDebug("committing");
	say(_("Checking dependencies\n"));
	currentStatus = _("Checking dependencies...");
	int errorCount = DepTracker->renderData();
	if (errorCount==0)
	{
		say(_("Building queue\n"));
		mDebug("Tracking deps");
		DepTracker->commitToDb();
		say(_("Committing...\n"));
		currentStatus = _("Committing changes...");
		unsigned int ret = db->commit_actions();
		if (ret==0) say(_("Complete successfully\n"));
		else mError(_("Commit failed"));
		currentStatus = _("Complete.");
		return ret;
	}
	else
	{
		//mpkgErrorReturn errRet = waitResponce(MPKG_DEPENDENCY_ERROR);
		mError(_("Error in dependencies: ") + IntToStr(errorCount) + _(" failures"));
		currentStatus = _("Failed - depencency errors");
		return MPKGERROR_UNRESOLVEDDEPS;
	}
}

bool mpkg::checkPackageIntegrity(string pkgName)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", &pkgName);
	sqlSearch.addField("package_installed", ST_INSTALLED);
	PACKAGE_LIST table;
	get_packagelist(&sqlSearch, &table);
	if (table.size()==0)
	{
		mError(_("No package \"") + pkgName + _("\" is installed"));
		return true;
	}
	if (table.size()!=1)
	{
		mError(_("Received ") + IntToStr(table.size()) + _(" packages, ambiguity!"));
		return false;
	}
	return checkPackageIntegrity(table.get_package(0));
}

bool mpkg::checkPackageIntegrity(PACKAGE *package)
{

	db->get_filelist(package->get_id(), package->get_files());
	// Function description: 
	// 	1. Check files for exist
	// 	2. Check files for integrity (md5 verify)
	// 	3. Access rights check (hm...)
	bool integrity_ok = true;
	bool broken_sym = false;
	for (unsigned int i=0; i<package->get_files()->size(); i++)
	{
		if (!FileExists(SYS_ROOT + *package->get_files()->at(i).get_name(), &broken_sym))
		{
			if (integrity_ok) 
				mError(_("Package ") + (string) CL_YELLOW + *package->get_name() + (string) CL_WHITE + _(" has broken files or symlinks:"));
			integrity_ok = false;
			if (!broken_sym) 
				say(_("%s%s%s: /%s (file doesn't exist)\n"), CL_YELLOW, package->get_name()->c_str(),CL_WHITE, package->get_files()->at(i).get_name()->c_str());
			else
				say(_("%s%s%s: /%s (broken symlink)\n"), CL_YELLOW, package->get_name()->c_str(),CL_WHITE, package->get_files()->at(i).get_name()->c_str());


		}
	}
	return integrity_ok;
}

bool mpkg::repair(PACKAGE *package)
{
	if (!package->available())
	{
		mError(_("Cannot repair ") + *package->get_name() + _(": package is unavailable"));
		return false;
	}
	db->set_action(package->get_id(), ST_REPAIR);
	return true;
}
			
void mpkg::exportBase(string output_dir)
{
	printf("Exporting data to %s directory\n",output_dir.c_str());
	PACKAGE_LIST allPackages;
	SQLRecord sqlSearch;
//	sqlSearch.addField("package_installed", 1);
	get_packagelist(&sqlSearch, &allPackages);
	PACKAGE *p;
	mstring data;
	printf("Received %d packages\n",allPackages.size());
	for (int i=0; i<allPackages.size(); i++)
	{
		data.clear();
		p = allPackages.get_package(i);
		data = "PACKAGE NAME:\t" + *p->get_name() +"-"+*p->get_version()+"-"+*p->get_arch()+"-"+*p->get_build() +\
			"\nCOMPRESSED PACKAGE SIZE:\t"+*p->get_compressed_size()+ \
			"\nUNCOMPRESSED PACKAGE SIZE:\t"+*p->get_installed_size()+\
			"\nPACKAGE LOCATION:\t/var/log/mount/"+*p->get_filename()+\
			"\nPACKAGE DESCRIPTION:\n"+adjustStringWide(*p->get_short_description()+"\n"+*p->get_description(),70,*p->get_name())+\
			"\nFILE LIST:\n";
		for (unsigned int f=0; f<p->get_files()->size(); f++)
		{
			data+=*p->get_files()->at(i).get_name();
		}
		data+="\n";
		WriteFile(output_dir+"/"+*p->get_name()+"-"+*p->get_version()+"-"+*p->get_arch()+"-"+*p->get_build(), data.s_str());
	}
}

