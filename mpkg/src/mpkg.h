/***********************************************************************************
 * 	$Id: mpkg.h,v 1.15 2007/03/11 03:22:27 i27249 Exp $
 * 	MOPSLinux Package System
 * ********************************************************************************/

#ifndef MPKG_H_
#define MPKG_H_

#include "constants.h"
#include "debug.h"
#include "config.h"
#include "sql_pool.h"
class mpkgDatabase
{
	public:
		// Functions to get data
		int get_package(int package_id, PACKAGE *package, bool GetExtraInfo=true);
		int get_packagelist(SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo=true);
		int get_descriptionlist(int package_id, DESCRIPTION_LIST *desclist, string language="");
		int get_filelist (int package_id, FILE_LIST *filelist, bool config_only=false);
		int get_dependencylist(int package_id, DEPENDENCY_LIST *deplist);
		int get_taglist(int package_id, TAG_LIST *taglist);
		int get_server_tag_list(int server_id, SERVER_TAG_LIST *server_tag_list);
		int get_server(int server_id, SERVER *server);
		int get_locationlist(int package_id, LOCATION_LIST *location_list);
		int get_last_id(string table_name, string id_field="");
		int get_package_id(PACKAGE *package);
		int get_status(int package_id);
		int get_scripts(int package_id, SCRIPTS *scripts);
		int sqlFlush();
		
		// Checking functions
		int check_file_conflicts (PACKAGE *package);
		int check_install_package(PACKAGE *package); 

		// Action functions
		string _install_package(PACKAGE *package);
		int _remove_package(PACKAGE *package);
		int add_filelist_record(int package_id, FILE_LIST *filelist);
		int add_descriptionlist_record(int package_id, DESCRIPTION_LIST *desclist);
		int add_locationlist_record(int package_id, LOCATION_LIST *locationlist);
		int add_server_record(SERVER *server);
		int add_server_taglist_record(int server_id, SERVER_TAG_LIST *server_tag_list);
		int add_server_tag_link(int server_id, int tag_id);
		int add_dependencylist_record(int package_id, DEPENDENCY_LIST *deplist);
		int add_taglist_record(int package_id, TAG_LIST *taglist);
		int add_tag_link(int package_id, int tag_id);
		int add_package_record(PACKAGE *package);
		int add_scripts_record(int package_id, SCRIPTS *scripts);
		int update_package_data(int package_id, PACKAGE *package);

		// Purge functions
		int get_purge(string package_name);
	
		// Cleanup functions
		int cleanFileList(int package_id);
		int clean_package_filelist (PACKAGE *package);
	private:
		SQLProxy db;
	public:
		int commit_actions(); // Commits ALL actions planned in database
		int set_status(int package_id, int status);
		int emerge_to_db(PACKAGE *package);	// Adds new package to database, filtering data
		int fetch_package(PACKAGE *package); // Downloads package to local cache, returns 0 if ok, 1 if failed. Also, checks if package is already downloaded.
		int install_package(PACKAGE *package); // PHYSICALLY install package (extract, execute scripts)
		int remove_package(PACKAGE *package); // PHYSICALLY remove package (delete files, execute remove scripts)
		int purge_package(PACKAGE *package); // removes config files
		int uninstall(vector<string> pkgnames); // Wrapper, uninstalls all packages with given names.
		int updateRepositoryData(PACKAGE_LIST *newPackages);
		PACKAGE get_installed_package(string pkg_name); // Returns installed package

		string get_file_md5(string filename);
		SQLProxy *getSqlDb();
		mpkgDatabase();
		~mpkgDatabase();
};



#endif //MPKG_H_

#include "local_package.h"
#include "core.h"
#include "dependencies.h"
