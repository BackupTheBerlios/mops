/**
	MOPSLinux packaging system    
	    installpkg-ng
	New generation of installpkg :-)
	This tool ONLY can install concrete local file, but in real it can do more :-) 
	
	$Id: installpkg-ng.cpp,v 1.12 2006/12/22 10:33:59 adiakin Exp $
				    **/


#include "local_package.h"
#include "debug.h"
#include "mpkg.h"
#include "repository.h"
#include "actions.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>


/*
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/ndc.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
*/


const char* program_name;
extern char* optarg;
extern int optind, opterr, optopt;

//static LoggerPtr rootLogger;

int setup_action(char* act);
int check_action(char* act);
void print_usage(FILE* stream, int exit_code);
int install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker);
int uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker);



int main (int argc, char **argv)
{
setlocale(LC_ALL, "");
bindtextdomain( "installpkg-ng", "/usr/share/locale");
textdomain("installpkg-ng");

/*	
	try {

		BasicConfigurator::configure();
 		rootLogger = Logger::getRootLogger();

		NDC::push(_T("main"));

		rootLogger->debug(_T("starting app..."));
	} catch (Exception&) {
#ifdef DEBUG
			printf("no logger avaliable\n");
#endif
	}
*/

	/**
	 * do we need to be verbose?
	 */
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
		
		action = setup_action( argv[optind++] );
	}


	if ( action == ACT_NONE )
			print_usage(stderr, 1);
	


	mpkgDatabase db;
	DependencyTracker DepTracker;


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
			uninstall(pname, &db, &DepTracker);
		}
	}

	if ( action == ACT_SEARCH ) {
		
	}

	if ( action == ACT_UPGRADE  ) {
	
	}

	if (  action == ACT_LIST ) {
	
	}

	if ( action == ACT_UPDATE ) {
		
	}

	if ( action == ACT_CLEAN ) {
	
	}

	
	printf(_("Next packages will INSTALL:\n"));
	for (int i=0;i<DepTracker.get_install_list()->size();i++)
	{
		printf("%s \t(status=%s)\n", DepTracker.get_install_list()->get_package(i)->get_name(false).c_str(), \
				DepTracker.get_install_list()->get_package(i)->get_vstatus().c_str());
		//DepTracker.PrintFailure(DepTracker.get_install_list()->get_package(i));
	}
	printf(_("\nNext packages will REMOVE:\n"));
	for (int i=0;i<DepTracker.get_remove_list()->size();i++)
	{
		printf("%s:\n", DepTracker.get_remove_list()->get_package(i)->get_name(false).c_str());
		DepTracker.PrintFailure(DepTracker.get_remove_list()->get_package(i));

	}
	printf(_("Next packages is BROKEN:\n"));
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
	fprintf(stream,_("\tupdate     update packages info\n"));
	fprintf(stream,_("\tlist       list installed packages\n"));
	fprintf(stream,_("\tsearch     search package\n"));
	fprintf(stream,_("\tclean      remove all packages from cache\n"));
	exit(exit_code);
}


int install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker)
{


	printf(_("Preparing to install package %s\n"), fname.c_str());
	// Part 0. Check if file exists.
	
	struct stat st;
	if (lstat(fname.c_str(), &st) != 0) {
		if ( errno == ENOENT ) {
			fprintf(stderr, _("file %s not found\n"), fname.c_str());
			exit(1);
		}
	}
	
	if ( !S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode) ) {
		fprintf(stderr, _("unknown file type\n"));
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

int check_action(char* act)
{
	std::string _act(act);	
	int res = 0;

#ifdef DEBUG
	printf("DEBUG: action = %s\n", act);
#endif

	if ( _act != "install"
		|| _act != "remove"
	  	|| _act != "update"
	  	|| _act != "upgrade"
	  	|| _act != "list"
	  	|| _act != "search"
	  	|| _act != "clean" ) {
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
	printf("DEBUG: action = %s\n", act);
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

	return ACT_NONE;
}


