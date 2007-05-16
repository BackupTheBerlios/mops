/***********************************************************************************
 * 	$Id: mpkg.h,v 1.25 2007/05/16 06:43:20 i27249 Exp $
 * 	MOPSLinux Package System
 * ********************************************************************************/

#ifndef MPKG_H_
#define MPKG_H_

#include "constants.h"
#include "debug.h"
#include "config.h"
#include "sql_pool.h"
#include <ftw.h>
class mpkgDatabase
{
	public:
		// Functions to get data
		int get_package(int package_id, PACKAGE *package, bool GetExtraInfo=true);
		int get_packagelist(SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo=true, bool ultraFast=false);
#ifdef ENABLE_INTERNATIONAL
		int get_descriptionlist(int package_id, DESCRIPTION_LIST *desclist, string language="");
#endif
		int get_filelist (int package_id, FILE_LIST *filelist, bool config_only=false);
		int get_dependencylist(int package_id, DEPENDENCY_LIST *deplist);
		int get_taglist(int package_id, TAG_LIST *taglist);
		int get_server_tag_list(int server_id, SERVER_TAG_LIST *server_tag_list);
		int get_server(int server_id, SERVER *server);
		int get_locationlist(int package_id, LOCATION_LIST *location_list);
		int get_last_id(string table_name, string id_field="");
		int get_package_id(PACKAGE *package);
		int get_installed(int package_id);
		int get_available(int package_id);
		int get_action(int package_id);
		int get_configexist(int package_id);

		int get_scripts(int package_id, SCRIPTS *scripts);
		PACKAGE_LIST get_other_versions(string package_name);
		PACKAGE get_max_version(PACKAGE_LIST pkgList, DEPENDENCY *dep);
		bool checkVersion(string version1, int condition, string version2);
		int sqlFlush();
		
		// Checking functions
		int check_file_conflicts (PACKAGE *package);
		int backupFile(string filename, int overwritten_package_id, int conflicted_package_id);
		void mpkgDatabase::clean_backup_directory();
		int check_install_package(PACKAGE *package); 
		int add_conflict_record(int conflicted_id, int overwritten_id, string file_name);
		int delete_conflict_record(int conflicted_id, string file_name);
		FILE_LIST get_conflict_records(int conflicted_id);



		// Action functions
		string _install_package(PACKAGE *package);
		int _remove_package(PACKAGE *package);
		int add_filelist_record(int package_id, FILE_LIST *filelist);
#ifdef ENABLE_INTERNATIONAL
		int add_descriptionlist_record(int package_id, DESCRIPTION_LIST *desclist);
#endif
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
		int delete_packages(PACKAGE_LIST *pkgList);
		int cleanFileList(int package_id);
		int clean_package_filelist (PACKAGE *package);

	private:
		SQLProxy db;
	public:
		bool check_cache(PACKAGE *package, bool clear_wrong=true);
		int commit_actions(); // Commits ALL actions planned in database
		//int set_available(int package_id, int status);
		int set_action(int package_id, int status);
		int set_configexist(int package_id, int status);
		int set_installed(int package_id, int status);

		int emerge_to_db(PACKAGE *package);	// Adds new package to database, filtering data
		int install_package(PACKAGE *package); // PHYSICALLY install package (extract, execute scripts)
		int remove_package(PACKAGE *package); // PHYSICALLY remove package (delete files, execute remove scripts)
		int purge_package(PACKAGE *package); // removes config files
		int uninstall(vector<string> pkgnames); // Wrapper, uninstalls all packages with given names.
		int updateRepositoryData(PACKAGE_LIST *newPackages);
		int syncronize_data(PACKAGE_LIST *pkgList);
		PACKAGE get_installed_package(string pkg_name); // Returns installed package

		SQLProxy *getSqlDb();
		mpkgDatabase();
		~mpkgDatabase();
};



#endif //MPKG_H_

#include "local_package.h"
#include "core.h"
#include "dependencies.h"
