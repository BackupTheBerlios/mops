/********************************************************************************
 *					core_head.h
 * 			Central core for MOPSLinux package system
 *					Headers
 ********************************************************************************/
#ifndef CORE_H_
#define CORE_H_

#include "dataunits.h"
#include "constants.h"
#include "sql_pool.h"

//string T="', '"; // Wery helpful element for SQL queries

#define T "', '"   // Wery helpful element for SQL queries

//------------------Library front-end--------------------------------
RESULT modify_package(PACKAGE *package);
PACKAGE_LIST resolve_dependencies (PACKAGE *package);
RESULT check_file_conflicts (PACKAGE *package);
RESULT add_file_record(string package_id, FILES *file);
RESULT add_filelist_record(string package_id, FILE_LIST *filelist);
RESULT add_locationlist_record(string package_id, LOCATION_LIST *locationlist);
RESULT add_dependencylist_record(string package_id, DEPENDENCY_LIST *deplist);
RESULT add_taglist_record (string package_id, TAG_LIST *taglist);
RESULT add_tag_link(string package_id, string tag_id);
string add_package_record (PACKAGE *package);
RESULT add_server_record (SERVER *server);
RESULT add_server_taglist_record(string server_id, SERVER_TAG_LIST *server_tag_list);
RESULT add_server_tag_link(string server_id, string tag_id);

//--------------------Mid-level functions-------------------------
PACKAGE_LIST get_packagelist (string query);
RESULT get_filelist(string package_id, FILE_LIST *filelist);
RESULT get_locationlist(string package_id, LOCATION_LIST *location_list);
RESULT get_dependencylist(string package_id, DEPENDENCY_LIST *deplist);
RESULT get_package(string package_id, PACKAGE *package, bool GetExtraInfo);
RESULT get_packagelist(string query, PACKAGE_LIST *packagelist);
RESULT get_serverlist(string server_id, SERVER_LIST *server);
RESULT get_taglist(string package_id, TAG_LIST *taglist);
string get_last_id(string table_name, string id_field);
string get_id(string table_name, string id_field, string search_field, string search_value, char ***array=NULL, int *col=NULL);
void set_status(string package_id, int status);
string get_status(string package_id);
string get_package_id(PACKAGE *package);
string get_version(string package_id);
int check_install_package(PACKAGE *package);
//----------------------------------SQL low-level routines----------------------------

#endif //CORE_H_

