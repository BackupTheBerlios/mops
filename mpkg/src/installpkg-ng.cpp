/**
	MOPSLinux packaging system    
	    installpkg-ng
	New generation of installpkg :-)
	This tool ONLY can install concrete local file, but in real it can do more :-) 
	
	$Id: installpkg-ng.cpp,v 1.8 2006/12/20 19:05:00 i27249 Exp $
				    **/


#include "local_package.h"
#include "debug.h"
#include "mpkg.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


int install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker)
{


	printf("Preparing to install package %s\n", fname.c_str());
	// Part 0. Check if file exists.
	debug("opening file...");
	FILE* _f=fopen(fname.c_str(),"r");
	if (_f)
	{
		debug("File opened successfully, so it exists");
		fclose(_f);
		debug("File closed.");
	}
	else
	{
		printf("Error: file %s not found. Cannot install package %s\n", fname.c_str());
		return 1;
	}
	// Part 1. Extracts all information from file and fill the package structure, and insert into dep tracker
	LocalPackage lp(fname);
	lp.injectFile(); 
	lp.data.set_status(PKGSTATUS_AVAILABLE);
	db->emerge_to_db(&lp.data);
	DepTracker->merge(&lp.data);
	return 0;
	
}

int uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	printf("Preparing to UNINSTALL package %s\n", pkg_name.c_str());
	PACKAGE package=db->get_installed_package(pkg_name);
	DepTracker->unmerge(&package);
	return 0;
}

int main (int argc, char **argv)
{

	uid_t uid, euid;

	uid = getuid();
	euid = geteuid();

#ifdef DEBUG
	printf("uid = %i, euid = %i\n", uid, euid);
#endif

	if ( uid != 0 ) {
		fprintf(stderr, "You must login as root to run this program\n");
		exit(1);
	}
	

	if (argc<3)
	{
		printf("MOPSLinux Packaging System\ninstallpkg-ng v.0.1 alpha \nusage: %s install [package_file] [package_file]... \nor:   %s remove [package_name] [package_name]...\n", argv[0], argv[0]);
		return 1;
	}
	mpkgDatabase db;
	DependencyTracker DepTracker;
	string action=argv[1]; // We will depend on installpkg or removepkg to decide what to do

	if (action!="install" && action!="remove")
	{
		printf("MOPSLinux Packaging System\ninstallpkg-ng v.0.1 alpha \nusage: %s install [package_file] [package_file]... \nor:    %s remove [package_name] [package_name]...\n", argv[0], argv[0]);
		return 1;
	}

	string fname;
	string pname;
	if (action=="install")
	{
		for (int i=2;i<argc;i++)
		{
			fname=argv[i];
			install(fname, &db, &DepTracker);
		}
	}
	if (action=="remove")
	{
		for (int i=2; i<argc;i++)
		{
			pname=argv[i];
			uninstall(pname, &db, &DepTracker);
		}
	}
	
	printf("Next packages will INSTALL:\n");
	for (int i=0;i<DepTracker.get_install_list()->size();i++)
	{
		printf("%s \t(status=%s)\n", DepTracker.get_install_list()->get_package(i)->get_name(false).c_str(), \
				DepTracker.get_install_list()->get_package(i)->get_vstatus().c_str());
		//DepTracker.PrintFailure(DepTracker.get_install_list()->get_package(i));
	}
	printf("\n");	
	printf("Next packages will REMOVE:\n");
	for (int i=0;i<DepTracker.get_remove_list()->size();i++)
	{
		printf("%s:\n", DepTracker.get_remove_list()->get_package(i)->get_name(false).c_str());
		DepTracker.PrintFailure(DepTracker.get_remove_list()->get_package(i));

	}
	printf("\n");
	printf("Next packages is BROKEN:\n");
	for (int i=0;i<DepTracker.get_failure_list()->size();i++)
	{
		printf("%s:\n", DepTracker.get_failure_list()->get_package(i)->get_name(false).c_str());
		DepTracker.PrintFailure(DepTracker.get_failure_list()->get_package(i));

	}
	printf("\n");

	DepTracker.commitToDb();
	db.commit_actions();
	delete_tmp_files();
	return 0;
}
