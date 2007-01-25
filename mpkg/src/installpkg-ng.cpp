/**
 *	MOPSLinux packaging system    
 *	    installpkg-ng
 *	New generation of installpkg :-)
 *	This tool ONLY can install concrete local file, but in real it can do more :-) 
 *	
 *	$Id: installpkg-ng.cpp,v 1.22 2007/01/25 09:51:44 i27249 Exp $
 */

#include "config.h"
#include "local_package.h"
#include "debug.h"
#include "mpkg.h"
#include "repository.h"
#include "actions.h"
#include "converter.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>


const char* program_name;
extern char* optarg;
extern int optind, opterr, optopt;
string output_dir;
//static LoggerPtr rootLogger;

int setup_action(char* act);
int check_action(char* act);
void print_usage(FILE* stream, int exit_code);
int install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker);
int uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker, int do_purge);
int update_repository_data(mpkgDatabase *db, DependencyTracker *DepTracker);
int list(mpkgDatabase *db, DependencyTracker *DepTracker, vector<string> search);
int _clean(const char *filename, const struct stat *file_status, int filetype);
int clean_cache();
void ShowBanner();
int convert_directory();
int _conv_dir(const char *filename, const struct stat *file_status, int filetype);
int upgrade (string pkgname, mpkgDatabase *db, DependencyTracker *DepTracker);

void ShowBanner()
{
	char *version="0.1 alpha 4";
	char *copyright="\(c) 2006-2007 RPUNet (http://www.rpunet.ru)";
	printf("MOPSLinux packaging system v.%s\n%s\n--\n", version, copyright);
}

int main (int argc, char **argv)
{
	ShowBanner();
	loadGlobalConfig();
	setlocale(LC_ALL, "C");
	bindtextdomain( "installpkg-ng", "/usr/share/locale");
	textdomain("installpkg-ng");


	int verbose = 0;

	/**
	 * remove everithing?
	 */
	int do_purge = 0;

	/**
	 * selected action
	 * by default - ACT_NONE
	 */
	int action = ACT_NONE;

	uid_t uid, euid;

	uid = getuid();
	euid = geteuid();

#ifdef DEBUG
	printf("DEBUG: uid = %i, euid = %i\n", uid, euid);
#endif

	if ( uid != 0 ) {
		fprintf(stderr, _("You must login as root to run this program\n"));
		exit(1);
	}
	

	int ich;
	const char* short_opt = "hvp";
    const struct option long_options[] =  {
		{ "help",	0,	NULL, 'h'},
		{ "verbose", 0, NULL, 'v'},
		{ "purge", 0, NULL, 'p'},
		{ NULL, 0, NULL, 0}
	};

	program_name = argv[0];

	do {
		ich = getopt_long(argc, argv, short_opt,
						long_options, NULL);
		

		switch (ich) {
			case 'h':
					print_usage(stdout, 0);

			case 'v':
					verbose = 1;
					break;

			case 'p':
					do_purge = 1;
					break;
					
			case '?':
					print_usage(stderr, 1);

			case -1:
					break;

			default:
					abort();
		}
	
	}  while ( ich != -1 );


#ifdef DEBUG
	printf("DEBUG: last optind = %i\n", optind);
	printf("DEBUG: argc = %i\n", argc);
#endif

	if ( optind < argc ) {
		if ( check_action( argv[optind++] ) == -1 )
				print_usage(stderr, 1);
		
		action = setup_action( argv[optind-1] );
	}


	if ( action == ACT_NONE )
			print_usage(stderr, 1);
	


	mpkgDatabase db;
	DependencyTracker DepTracker(&db);


	string fname;
	string pname;

	if (action == ACT_INSTALL)
	{
		for (int i = optind; i < argc; i++)
		{
			fname=argv[i];
			install(fname, &db, &DepTracker);
		}
	}

	if (action == ACT_REMOVE)
	{
		for (int i = optind; i < argc; i++)
		{
			pname=argv[i];
			uninstall(pname, &db, &DepTracker, do_purge);
		}
	}

	if ( action == ACT_SEARCH ) {
		vector<string> list_search;
		for (int i = optind; i < argc; i++)
		{
			list_search.push_back((string) argv[i]);
		}
		list(&db, &DepTracker, list_search);
		delete_tmp_files();
		return 0;
	}
	if (action == ACT_UPGRADE ) {
		for (int i = optind; i < argc; i++)
		{
			pname=argv[i];
			upgrade(pname, &db, &DepTracker);
		}
		//printf("Upgrade not implemented yet\n");
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_CONVERT  ) {
		for (int i = optind; i < argc; i++)
		{
			convert_package((string) argv[i], "/root/development/converted/");
		}
		delete_tmp_files();
		return 0;
	
	}

	if ( action == ACT_CONVERT_DIR ) {
		if (optind < argc )
		{
			output_dir=(string) argv[optind];
			convert_directory();
		}
		else 
		{
			printf("Please define output directory\n");
		}
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_LIST ) {
		vector<string> list_empty;
		list(&db, &DepTracker, list_empty);
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_UPDATE ) {
		update_repository_data(&db, &DepTracker);
		delete_tmp_files();
		return 0;
		
	}

	if ( action == ACT_CLEAN ) {
		clean_cache();	
//		printf("Cache cleanup isn't implemented yet\n");
		delete_tmp_files();
		return 0;
	
	}

	if ( action == ACT_INDEX ) {
		Repository rep;
		if (optind < argc)
		{
			rep.build_index(argv[optind]);
		}
		else
		{
			printf("Please define server URL\n");
		}
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_PURGE ) {
		for (int i = optind; i < argc; i++)
		{
			do_purge=1;
			pname=argv[i];
			uninstall(pname, &db, &DepTracker, do_purge);
		}
	}

	// ACTION SUMMARY - NEED TO FIX!
	if (!DepTracker.get_install_list()->IsEmpty())
	{
		printf(_("Next packages will be installed:\n"));
		for (int i=0;i<DepTracker.get_install_list()->size();i++)
		{
			printf("%s\n", DepTracker.get_install_list()->get_package(i)->get_name(false).c_str());
		}
	}

	if (!DepTracker.get_remove_list()->IsEmpty())
	{
		printf(_("\nNext packages will be removed:\n"));
		for (int i=0;i<DepTracker.get_remove_list()->size();i++)
		{
			printf("%s\n", DepTracker.get_remove_list()->get_package(i)->get_name(false).c_str());
		}
	}

	if (!DepTracker.get_failure_list()->IsEmpty())
	{

		printf(_("Next packages is failed to install:\n"));
		for (int i=0;i<DepTracker.get_failure_list()->size();i++)
		{
			printf("%s: ", DepTracker.get_failure_list()->get_package(i)->get_name(false).c_str());
			DepTracker.PrintFailure(DepTracker.get_failure_list()->get_package(i));
		}
	}
	printf("\n");

	DepTracker.commitToDb();
	db.commit_actions();
	delete_tmp_files();
	return 0;
}


void print_usage(FILE* stream, int exit_code)
{
	fprintf(stream, _("Usage: %s [options] action package [package ...]\n"), program_name);
	fprintf(stream,_("Options:\n"));
	fprintf(stream,_("\t-h    --help       show this help\n"));
	fprintf(stream,_("\t-v    --verbose    be verbose\n"));
	fprintf(stream,_("\t-p    --purge      purge package\n\n"));
	fprintf(stream,_("Actions:\n"));
	fprintf(stream,_("\tinstall    install packages\n"));
	fprintf(stream,_("\tupgrade    upgrade selected package or full system if no package selected\n"));
	fprintf(stream,_("\tremove     remove selected package\n"));
	fprintf(stream,_("\tpurge      purge selected package\n"));
	fprintf(stream,_("\tupdate     update packages info\n"));
	fprintf(stream,_("\tlist       list installed packages\n"));
	fprintf(stream,_("\tsearch     search package\n"));
	fprintf(stream,_("\tclean      remove all packages from cache\n"));
	fprintf(stream,_("Repository maintaining functions:\n"));
	fprintf(stream,_("\tindex [server_url]     create a repository index file \"packages.xml\"\n"));
	fprintf(stream,_("\tconvert [package]      convert specified packages from Slackware to MPKG format\n"));
	fprintf(stream,_("\tconvert_dir [outp_dir] convert whole directory (including sub-dirs) to MPKG format\n"));


	exit(exit_code);
}

int update_repository_data(mpkgDatabase *db, DependencyTracker *DepTracker)
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
	
	for (unsigned int i=0; i<REPOSITORY_LIST.size(); i++)
	{
		//printf("cycle i=%d\n", i);
		tmpPackages = rep.get_index(REPOSITORY_LIST[i]);
		if (i+1==REPOSITORY_LIST.size())
		{
			printf("Download completed. Processing data, please wait...\n");
		}
		if (tmpPackages.IsEmpty())
		{
			printf("No packages found at %s: repository error, or connection error\n", REPOSITORY_LIST[i].c_str());
			break;
		}
		else
		{
			//printf(_("Repository has %d packages, infiltrating...\n"), tmpPackages.size());
			for (int s=0; s<tmpPackages.size(); s++)
			{
				tmpPackages.get_package(s)->set_status(PKGSTATUS_AVAILABLE);
				debug("installpkg-ng.cpp: update_repository_data(): set status to PKGSTATUS_AVAILABLE ("+\
						IntToStr(tmpPackages.get_package(s)->get_status())+")");

				tmpPackages.get_package(s)->get_locations()->get_location(0)->get_server()->set_url(REPOSITORY_LIST[i]);
				tmpPackages.get_package(s)->get_locations()->get_location(0)->get_server()->set_priority(IntToStr(i+1));


				debug("installpkg-ng.cpp: update_repository_data(): set server url to "+\
						tmpPackages.get_package(s)->get_locations()->get_location(0)->get_server()->get_url());

			}
			//printf("Running add_list....");
			availablePackages.add_list(&tmpPackages);
			//printf("done.\n");
			//printf(_("After infiltrating, we have got %d NEW packages"), availablePackages.size());

		}
	}

#ifdef DEBUG
	for (int i=0; i<availablePackages.size(); i++)
	{
		printf("installpkg-ng.cpp (update_repository_data): package %d has %d locations\n", i, availablePackages.get_package(i)->get_locations()->size());
	}
	printf("installpkg-ng.cpp: (update_repository_data): Sending %d packages to db->updateRepositoryData()\n", availablePackages.size());
#endif
	// Go merging to DB
	printf(_("Importing new data...\n"));
	int ret=db->updateRepositoryData(&availablePackages);
	printf("Import complete.\n");
	return ret;
	
}

int install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker)
{


	//printf(_("Querying the database, please wait...\n"));
	// Step 1. Checking if it is just a name of a package
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", fname);
	PACKAGE_LIST candidates;
	PACKAGE tmp_pkg;
	db->get_packagelist(sqlSearch, &candidates);
	bool alreadyInstalled=false;
	if (candidates.size()>0)
	{
		for (int i=0; i<candidates.size(); i++)
		{
			if (candidates.get_package(i)->get_status()==PKGSTATUS_INSTALLED)
			{
				alreadyInstalled=true;
				printf(_("Package is already installed (ver. %s). For upgrade, choose upgrade option\n"), candidates.get_package(i)->get_version().c_str());
				break;
			}
			if (candidates.get_package(i)->get_status()==PKGSTATUS_AVAILABLE)
			{
				if (tmp_pkg.get_version()<candidates.get_package(i)->get_version() || tmp_pkg.IsEmpty())
				{
					tmp_pkg=*candidates.get_package(i);
				}
			}
		}
		if (alreadyInstalled)
		{
			return 0;
		}
		DepTracker->merge(&tmp_pkg);
		return 0;
	}
	
	// If reached this point, the package isn't in the database. Install from local file.
	// Part 0. Check if file exists.
	if (!CheckFileType(fname))
	{
		printf("%s: unsupported package type\n", fname.c_str());
		return -1;
	}
	struct stat st;
	if (lstat(fname.c_str(), &st) != 0) {
		if ( errno == ENOENT ) {
			fprintf(stderr, _("file %s not found\n"), fname.c_str());
			exit(1);
		}
	}
	
	if ( !S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode) ) {
		fprintf(stderr, _("unknown file type\n"));
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


int uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker, int do_purge)
{
	if (do_purge==0) printf(_("You are going to uninstall package %s\n"), pkg_name.c_str());
	if (do_purge==1) printf(_("You are going to purge package %s\n"), pkg_name.c_str());
	
	PACKAGE package=db->get_installed_package(pkg_name);
	if (package.IsEmpty())
	{
		int id=db->get_purge(pkg_name);
		if (id==0)
		{
			printf(_("Package %s is not installed and purged\n"), pkg_name.c_str());
			return 0;
		}
		if (id<0)
		{
			printf("Internal error while calling get_purge(): error code = %d\n", id);
			return id;
		}
		db->set_status(id, PKGSTATUS_PURGE);
		return 0;
	}
	DepTracker->unmerge(&package, do_purge);
	return 0;
}

int upgrade (string pkgname, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	// Alpha realization: bad algorithm (поплывет по зависимостям)
	uninstall(pkgname, db, DepTracker, false);
	DepTracker->commitToDb();
	db->commit_actions();

	install(pkgname, db, DepTracker);
	DepTracker->commitToDb();
	db->commit_actions();
	return 0;
}

int list(mpkgDatabase *db, DependencyTracker *DepTracker, vector<string> search)
{
	PACKAGE_LIST pkglist;
	SQLRecord sqlSearch;
	if (!search.empty())
	{
		sqlSearch.setEqMode(EQ_LIKE);
		for (unsigned int i=0; i<search.size(); i++)
		{
			sqlSearch.addField("package_name", search[i]);
		}
	}
	db->get_packagelist(sqlSearch, &pkglist, false);
	if (pkglist.IsEmpty())
	{
		printf(_("Package database empty\n"));
		return 0;
	}

	for (int i=0; i<pkglist.size(); i++)
	{
		printf("[%s]\t%s-%s-%s-%s\t(%s)\n", \
				pkglist.get_package(i)->get_vstatus().c_str(), \
				pkglist.get_package(i)->get_name().c_str(), \
				pkglist.get_package(i)->get_version().c_str(), \
				pkglist.get_package(i)->get_arch().c_str(), \
				pkglist.get_package(i)->get_build().c_str(), \
				pkglist.get_package(i)->get_short_description().c_str());
	}
	return 0;
}

int _clean(const char *filename, const struct stat *file_status, int filetype)
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
//	printf("."); //Optional =)
	return 0;
}

int clean_cache()
{
	printf("Cleaning cache...");
	ftw(SYS_CACHE.c_str(), _clean, 100);
	printf("done\n");
	return 0;
}

int _conv_dir(const char *filename, const struct stat *file_status, int filetype)
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


int convert_directory()
{
	ftw("./", _conv_dir, 100);
	return 0;
}

int check_action(char* act)
{
	std::string _act(act);	
	int res = 0;

#ifdef DEBUG
	printf("DEBUG: action = %s\n", act);
#endif

	if ( _act != "install"
		&& _act != "remove"
	  	&& _act != "update"
	  	&& _act != "upgrade"
	  	&& _act != "list"
	  	&& _act != "search"
		&& _act != "index"
		&& _act != "purge"
		&& _act != "convert"
		&& _act != "convert-dir"
	  	&& _act != "clean" ) {
		res = -1;
	}

#ifdef DEBUG
	printf("DEBUG: res = %i\n", res);
#endif

	return res;
}

int setup_action(char* act)
{
	std::string _act(act);

#ifdef DEBUG
	printf("[setup_action] DEBUG: action = %s\n", act);
#endif

	if ( _act == "install" )
			return ACT_INSTALL;

	if ( _act == "remove" )
			return ACT_REMOVE;

	if ( _act == "search" )
		   return ACT_SEARCH;

	if ( _act == "list")
		return 	ACT_LIST;

	if ( _act == "update" )
		return ACT_UPDATE;

	if ( _act == "upgrade" )
		return ACT_UPGRADE;

	if ( _act == "clean" )
		return ACT_CLEAN;

	if (_act == "index" )
		return ACT_INDEX;

	if (_act == "purge" )
		return ACT_PURGE;

	if (_act == "convert")
		return ACT_CONVERT;

	if (_act == "convert-dir")
		return ACT_CONVERT_DIR;

	return ACT_NONE;
}


