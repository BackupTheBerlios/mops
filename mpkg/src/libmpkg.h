/********************************************************************************
 * MOPSLinux packaging system: core API
 * $Id: libmpkg.h,v 1.2 2007/02/09 16:11:12 i27249 Exp $
 * *****************************************************************************/

#ifndef LIBMPKG_H_
#define LIBMPKG_H_

// Includes: some overhead, but enough.
#include "config.h"
#include "local_package.h"
#include "debug.h"
#include "mpkg.h"
#include "repository.h"
#include "actions.h"
#include "converter.h"
#include "mpkgsys.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>


// Main class - use only this one while using the library
class mpkg
{
	public:
		// Constructor & destructor
		mpkg();
		~mpkg();
		bool init_ok;
		// Interface transports - current status messages
		string current_status;

		// Package building
		int build_package();
		int convert_directory(string output_dir);

		// Repository building
		Repository rep;
		
		// Package installation, removing, upgrade
		int install(vector<string> fname);
		int uninstall(vector<string> pkg_name);
		int upgrade (vector<string> pkgname);
		int purge(vector<string> pkg_name);
		
		// Repository data updating
		int update_repository_data();

		// Cache cleaning
		int clean_cache();

		// Package list retrieving
		int get_packagelist(SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo=true);

		// Server data retrieving
		int get_server_tag_list(int server_id, SERVER_TAG_LIST *server_tag_list);
		int get_server(int server_id, SERVER *server);

		// Configuration and settings: retrieving
		vector<string> get_repositorylist();
		//SERVER_LIST get_fullrepositorylist();
		string get_sysroot();
		string get_syscache();
		string get_dburl();
		string get_scriptsdir();
		bool get_runscripts();

		// Configuration and settings: setting
		int set_repositorylist(vector<string> newrepositorylist);
		//int set_fullrepositorylist(SERVER_LIST newrepositorylist);
		int set_sysroot(string newsysroot);
		int set_syscache(string newsyscache);
		int set_dburl(string newdburl);
		int set_scriptsdir(string newscriptsdir);
		int set_runscripts(bool dorun);

		// Finalizing
		int commit();

		mpkgDatabase *db;
		DependencyTracker *DepTracker;

};

#endif //LIBMPKG_H_

