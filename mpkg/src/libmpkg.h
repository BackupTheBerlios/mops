/********************************************************************************
 * MOPSLinux packaging system: core API
 * $Id: libmpkg.h,v 1.3 2007/02/12 01:35:36 i27249 Exp $
 *
 * Use this header to access the library. No other headers are needed :)
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

		// Error checking

		bool init_ok;
		/* Boolean value:
		 * TRUE if all initialized OK (database is opened and passed integrity check, etc)
		 * FALSE if something goes wrong
		 @@*/

		// Interface transports - current status messages
		string current_status;
		/* Contains current status. Used to make user output more informative.
		 * @@*/

		// Package building
		int build_package();
		/* build_package:
		 * Builds package in current directory
		 * It gets data from install/data.xml, and runs makepkg based on package information.
		 * Returns 0 if all ok, -1 if fails.
		 * @@*/

		int convert_directory(string output_dir);
		/* convert_directory(string output_dir):
		 * Converts current directory (including subdirs) from old Slackware format to new.
		 * It collects data from package and generates install/data.xml, and repackages the
		 * package with extended data. Output will be placed in path defined by output_dir variable.
		 * Returns 0 if all ok, <0 (e.g. -1) if something failed.
		 * @@*/

		// Repository building
		Repository rep;
		/* rep:
		 * Object containing repository processing functions.
		 * Try to not use it directly, in future, should be moved to private.
		 * @@*/
		
		// Package installation, removing, upgrade
		int install(vector<string> fname);
		/* install(vector<string> fname):
		 * Prepares packages with names defined in fname vector to install.
		 * It checks availability, chooses newest version, builds dependency tree, and
		 * marks all required packages to install/remove/upgrade.
		 * Note: this function does NOT actually install the package, it just places it on the queue.
		 * Returns 0 if successful, <0 if fails.
		 * @@*/

		int uninstall(vector<string> pkg_name);
		/* uninstall(vector<string> pkg_name):
		 * Prepares packages with names defined by pkg_name vector to uninstall.
		 * It builds dependency tree, and marks all required packages to remove.
		 * Returns 0 if successful, <0 if fails.
		 * @@*/

		int upgrade (vector<string> pkgname);
		/* upgrade (vector<string> pkgname):
		 * Upgrades packages defined by pkgname vector.
		 * It builds dependency tree, and marks all required packages to remove and install.
		 * Returns 0 if successful, <0 if fails.
		 * @@*/

		int purge(vector<string> pkg_name);
		/* purge (vector<string> pkg_name):
		 * Purges package. Similar to uninstall, but mark to remove package completely (including config files).
		 * If package is already uninstalled, it marks configuration files to be removed.
		 * Returns 0 if OK, <0 if fails.
		 * @@*/
		
		// Repository data updating
		int update_repository_data();
		/* update_repository_data():
		 * Updates a list of available packages from the repositories defined in configuration file.
		 * Returns 0 of OK, <0 if something fails.
		 * @@*/

		// Cache cleaning
		int clean_cache();
		/* clean_cache():
		 * Removes all packages from download cache.
		 * Returns 0 if OK, <0 if fails.
		 * @@*/

		// Package list retrieving
		int get_packagelist(SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo=true);
		/* get_packagelist(SQLRecord, PACKAGE_LIST *, bool):
		 * Fills the packagelist by packages who meets sqlSearch requirements.
		 * To use this, you should define sqlSearch first. See documentation about an SQLRecord object.
		 * If GetExtraInfo is true (default value), the function retrieves a complete info about packages,
		 * otherwise only basic (name, ver, arch, build, sizes, descriptions and changelog) will be retrieved
		 * (which is little faster). For now on, I don't know any computer who was so slow so it requires this speedup.
		 * Returns 0 if all ok, <0 if fails.
		 * @@*/

		// Server data retrieving
		int get_server_tag_list(int server_id, SERVER_TAG_LIST *server_tag_list);
		int get_server(int server_id, SERVER *server);

		// Configuration and settings: retrieving
		vector<string> get_repositorylist();
		/* get_repositorylist():
		 * returns a vector with URLs of currently enabled repositories.
		 * @@*/
		//SERVER_LIST get_fullrepositorylist();
		string get_sysroot(); // Returns system root path (in normal sys, should be '/')

		string get_syscache(); // Returns package cache path
		string get_dburl(); // Returns database URL
		string get_scriptsdir(); // Returns package scripts dir path
		bool get_runscripts(); // True if configured to run scripts, false otherwise.

		// Configuration and settings: setting
		int set_repositorylist(vector<string> newrepositorylist); // Sets new repository list to config
		//int set_fullrepositorylist(SERVER_LIST newrepositorylist);
		int set_sysroot(string newsysroot); // Sets new system root to config
		int set_syscache(string newsyscache); // Sets new system cache to config
		int set_dburl(string newdburl); // Sets new database to use
		int set_scriptsdir(string newscriptsdir); // Sets a scripts directory
		int set_runscripts(bool dorun); // Enables or disables a script running

		// Finalizing
		int commit(); 
		/* Committing changes:
		 * this function ACTUALLY perform all queued actions: install, upgrade, remove, etc.
		 * @@*/

		// Objects to work with. Normally should be private, so try to avoid using it directly.
		mpkgDatabase *db;
		DependencyTracker *DepTracker;

};

#endif //LIBMPKG_H_

