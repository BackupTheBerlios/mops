/***********************************************************************************
 * 	$Id: mpkg.h,v 1.28 2007/05/18 07:35:33 i27249 Exp $
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
		int get_packagelist(SQLRecord *sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo=true, bool ultraFast=false);
#ifdef ENABLE_INTERNATIONAL
		int get_descriptionlist(int package_id, vector<DESCRIPTION> *desclist, string language="");
#endif
		int get_filelist (int package_id, vector<FILES> *filelist, bool config_only=false);
		int get_dependencylist(int package_id, vector<DEPENDENCY> *deplist);
		int get_taglist(int package_id, vector<string> *taglist);
//		int get_server_tag_list(int server_id, SERVER_TAG_LIST *server_tag_list);
//		int get_server(int server_id, SERVER *server);
		int get_locationlist(int package_id, vector<LOCATION> *location_list);
//		int get_last_id(/*string table_name, string id_field=""*/);
		int get_package_id(PACKAGE *package);
		int get_installed(int package_id);
		int get_available(int package_id);
		int get_action(int package_id);
		int get_configexist(int package_id);

//		int get_scripts(int package_id, SCRIPTS *scripts);
		PACKAGE_LIST get_other_versions(string *package_name);
		PACKAGE* get_max_version(PACKAGE_LIST* pkgList, DEPENDENCY *dep);
		bool checkVersion(string *version1, int condition, string *version2);
		int sqlFlush();
		
		// Checking functions
		int check_file_conflicts (PACKAGE *package);
		int backupFile(string *filename, int overwritten_package_id, int conflicted_package_id);
		void clean_backup_directory();
		int add_conflict_record(int conflicted_id, int overwritten_id, string *file_name);
		int delete_conflict_record(int conflicted_id, string *file_name);
		void get_conflict_records(int conflicted_id, vector<FILES> *ret);



		// Action functions
		int add_filelist_record(int package_id, vector<FILES> *filelist);
#ifdef ENABLE_INTERNATIONAL
		int add_descriptionlist_record(int package_id, vector<DESCRIPTION> *desclist);
#endif
		int add_locationlist_record(int package_id, vector<LOCATION> *locationlist);
		int add_dependencylist_record(int package_id, vector<DEPENDENCY> *deplist);
		int add_taglist_record(int package_id, vector<string> *taglist);
		int add_tag_link(int package_id, int tag_id);
		int add_package_record(PACKAGE *package);
		int update_package_data(int package_id, PACKAGE *package);

		// Purge functions
		int get_purge(string* package_name);
	
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
		int uninstall(vector<string>* pkgnames); // Wrapper, uninstalls all packages with given names.
		int updateRepositoryData(PACKAGE_LIST *newPackages);
		int syncronize_data(PACKAGE_LIST *pkgList);
		PACKAGE get_installed_package(string* pkg_name); // Returns installed package

		SQLProxy *getSqlDb();
		mpkgDatabase();
		~mpkgDatabase();
};



#endif //MPKG_H_

#include "local_package.h"
#include "core.h"
#include "dependencies.h"
