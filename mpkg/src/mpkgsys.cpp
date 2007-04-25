/*********************************************************
 * MOPSLinux packaging system: general functions
 * $Id: mpkgsys.cpp,v 1.14 2007/04/25 23:47:44 i27249 Exp $
 * ******************************************************/

#include "mpkgsys.h"

string output_dir;

// Cleans system cache
int mpkgSys::clean_cache()
{
	ftw(SYS_CACHE.c_str(), _clean, 100);
	return 0;
}

int mpkgSys::clean_queue(mpkgDatabase *db)
{
	PACKAGE_LIST toInstall;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", IntToStr(ST_INSTALL));
	sqlSearch.addField("package_action", IntToStr(ST_REMOVE));
	sqlSearch.addField("package_action", IntToStr(ST_PURGE));
	db->get_packagelist(sqlSearch, &toInstall);
	for (int i=0; i<toInstall.size();i++)
	{
		db->set_action(toInstall.get_package(i)->get_id(), ST_NONE);
	}
	return 0;
}

int mpkgSys::unqueue(int package_id, mpkgDatabase *db)
{
	db->set_action(package_id, ST_NONE);
	return 0;
}


int mpkgSys::build_package()
{
    printf("building package...\n");
    if (FileNotEmpty("install/data.xml"))
    {
	    PackageConfig p("install/data.xml");
	    if (!p.parseOk) return -100;
	    string pkgname;
	    string sysline;
	    pkgname=p.getName()+"-"+p.getVersion()+"-"+p.getArch()+"-"+p.getBuild();
	    printf("Creating package %s\n", pkgname.c_str());
#ifdef APPLE_DEFINED
	    sysline = "tar czf "+pkgname+".tgz *";
#else
	    sysline="makepkg -l y -c n "+pkgname+".tgz";
#endif
	    system(sysline.c_str());
    }
    return 0;
}

int mpkgSys::update_repository_data(mpkgDatabase *db, DependencyTracker *DepTracker)
{
	Repository rep;
	PACKAGE_LIST availablePackages;
	PACKAGE_LIST tmpPackages;
	if (REPOSITORY_LIST.empty())
	{
		printf("No repositories defined, add at least one to proceed\n");
		return -1;
	}

	printf(_("Updating package data from %d repositories...\n"), REPOSITORY_LIST.size());
	int total_packages=0;
	for (unsigned int i=0; i<REPOSITORY_LIST.size(); i++)
	{
		tmpPackages.clear();
		rep.get_index(REPOSITORY_LIST[i], &tmpPackages);
		if (!tmpPackages.IsEmpty())
		{
			total_packages+=tmpPackages.size();
			printf("Received %d packages from %d rep\n", tmpPackages.size(), i);
			for (int s=0; s<tmpPackages.size(); s++)
			{
				tmpPackages.get_package(s)->set_available();
				tmpPackages.get_package(s)->get_locations()->get_location(0)->get_server()->set_url(REPOSITORY_LIST[i]);
				tmpPackages.get_package(s)->get_locations()->get_location(0)->get_server()->set_priority(IntToStr(i+1));
			}
			availablePackages.add_list(&tmpPackages);
		}
	}
	printf("Total %d packages received, filtering...\n", total_packages);
	int ret=db->updateRepositoryData(&availablePackages);
	printf("Update complete.\n");
	return ret;
}

int mpkgSys::install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker, bool do_upgrade)
{
	
	// Step 0. Checking source

	//printf(_("Querying the database, please wait...\n"));
	// Step 1. Checking if it is just a name of a package
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", fname);
	PACKAGE_LIST candidates;
	PACKAGE tmp_pkg;
	db->get_packagelist(sqlSearch, &candidates);
	debug("candidates size = "+IntToStr(candidates.size()));
	bool alreadyInstalled=false;
	if (candidates.size()>0)
	{
		for (int i=0; i<candidates.size(); i++)
		{
			debug("Candidate #"+IntToStr(i)+": "+candidates.get_package(i)->get_name()+" with ID "+IntToStr(candidates.get_package(i)->get_id())+" and status "+ \
					candidates.get_package(i)->get_vstatus());
			if (!do_upgrade && candidates.get_package(i)->installed())
			{
				alreadyInstalled=true;
				printf(_("Package is already installed (ver. %s). For upgrade, choose upgrade option\n"), candidates.get_package(i)->get_version().c_str());
				break;
			}
			if (candidates.get_package(i)->available())
			{
				debug("Status passed to installation");
				if (tmp_pkg.IsEmpty() || tmp_pkg.get_version()<candidates.get_package(i)->get_version())
				{
					debug("tmp stored successfully");
					tmp_pkg=*candidates.get_package(i);
				}
			}
		}
		if (alreadyInstalled)
		{
			printf("already installed\n");
			return 0;
		}
		DepTracker->merge(&tmp_pkg);
		return 0;
	}
	else printf("no candidates");
	return -1;
	/*	// Local installation disabled, because it need to do complete re-design... Have to think about..
	printf("LOCAL INSTALL ATTEMPT DETECTED\n");
	
	// If reached this point, the package isn't in the database. Trying to install from local file.
	// Part 1. Extracts all information from file and fill the package structure, and insert into dep tracker
	// NOTE: ALL PACKAGE FORMATTING TOOLS ARE CALLED HERE! Add any new formats here.
	
	int pkgType = CheckFileType(fname);
	
	if (pkgType == PKGTYPE_UNKNOWN)
	{
		printf("Unknown package - skipping\n");
		return -1;
	}
	else
	{
		LocalPackage lp(fname, pkgType);
		if (lp.injectFile()==0)
		{	
			lp.data.set_available();
			db->emerge_to_db(&lp.data);
			DepTracker->merge(&lp.data);
		}
	}
	return 0;	*/
}

int mpkgSys::uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker, int do_purge, bool do_upgrade)
{
	if (do_purge==0) printf(_("You are going to uninstall package %s\n"), pkg_name.c_str());
	if (do_purge==1) printf(_("You are going to purge package %s\n"), pkg_name.c_str());
	
	PACKAGE package=db->get_installed_package(pkg_name);
	if (package.IsEmpty())
	{
		debug("Not installed package");
		int id;
		if (do_purge==1) id=db->get_purge(pkg_name);
		else id=0;
		if (id==0)
		{
			if (do_purge) printf(_("Package %s is already purged\n"), pkg_name.c_str());
			else printf(_("Package %s is not installed\n"), pkg_name.c_str());
			return 0;
		}
		if (id<0)
		{
			printf("Internal error while calling get_purge(): error code = %d\n", id);
			return id;
		}
		printf("setting status to purge for ID %d\n", id);
		db->set_action(id, ST_PURGE);
		return 0;
	}
	DepTracker->unmerge(&package, do_purge, do_upgrade);
	return 0;
}

int mpkgSys::upgrade (string pkgname, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	printf("Upgrade: Calling uninstall\n");
	uninstall(pkgname, db, DepTracker, 0, true);
	printf("Upgrade: Calling install\n");
	install(pkgname, db, DepTracker, true);
	printf("Upgrade: normalizing\n");
	DepTracker->normalize();
	printf("Upgrade: committing deptracker\n");
	DepTracker->commitToDb();
	printf("Upgrade: committing actions\n");
	db->commit_actions();
	return 0;
}

int mpkgSys::_clean(const char *filename, const struct stat *file_status, int filetype)
{
	switch(filetype)
	{
		case FTW_F:
			unlink(filename);
			break;
		case FTW_D:
			rmdir(filename);
			break;
		default:
			unlink(filename);
	}
	return 0;
}

int mpkgSys::_conv_dir(const char *filename, const struct stat *file_status, int filetype)
{
	string _package=filename;
       	string ext;
	for (unsigned int i=_package.length()-4;i<_package.length();i++)
	{
		ext+=_package[i];
	}

	if (filetype==FTW_F && ext==".tgz")
	{
		convert_package(_package, output_dir);
	}
	return 0;

}


int mpkgSys::convert_directory(string out_dir)
{
	output_dir=out_dir;
	ftw("./", _conv_dir, 100);
	return 0;
}


