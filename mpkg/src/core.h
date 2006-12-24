/********************************************************************************
 *					core.h
 * 			Central core for MOPSLinux package system
 *					Headers
 *	$Id: core.h,v 1.5 2006/12/24 14:13:22 i27249 Exp $
 ********************************************************************************/
#ifndef CORE_H_
#define CORE_H_

#include "dataunits.h"
#include "constants.h"

//string T="', '"; // Wery helpful element for SQL queries

#define T "', '"   // Wery helpful element for SQL queries

typedef struct
{
	string fieldname;
	string value;
} SQLField;

class SQLRecord
{
	private:
		vector<SQLField> field;

	public:
		int size();
		bool empty();
		void clear();
		vector<string> getRecordValues();
		string getFieldName(int num);
		string getValue(string fieldname);
		string getValueI(int num);
		int addField(string filename, string value="");
		bool setValue(string fieldname, string value);

		SQLRecord();
		~SQLRecord();
};

class SQLTable
{
	private:
		vector<SQLRecord> table;
	public:
		int getRecordCount(); 	// returns record count
		bool empty();		// returns TRUE if table is empty (record count = 0), otherwise returns false
		void clear();		// clears table
		string getValue (int num, string fieldname);	// returns value of field called fieldname in num record
		SQLRecord getRecord(int num);

		void addRecord(SQLRecord record);
		SQLTable();
		~SQLTable();
		// TODO!!!
};



#include "sql_pool.h"


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
int add_scripts_record(string package_id, SCRIPTS *scripts);
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
int get_scripts(string package_id, SCRIPTS *scripts);
//----------------------------------SQL low-level routines----------------------------

#endif //CORE_H_

