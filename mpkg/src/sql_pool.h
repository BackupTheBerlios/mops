/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 *	$Id: sql_pool.h,v 1.5 2006/12/29 12:57:00 i27249 Exp $
 ************************************************************************************/



#ifndef SQL_POOL_H_
#define SQL_POOL_H_
#include <sqlite3.h>
#include "core.h"
#include "config.h"
//#include "mpkg.h"
class SQLiteDB
{
	private:
		string db_filename;
		int sqlError;
		string sqlErrMsg;
		string lastSQLQuery;
		RESULT sql_exec (string sql_query);
		bool CheckDatabaseIntegrity(); // Checks database integrity
		RESULT get_sql_table (string *sql_query, char ***table, int *rows, int *cols); // fills table using 
	public:
		vector<string> getFieldNames(string table_name);
		int getLastError();
		string getLastErrMsg();
		RESULT get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search);
		int sql_insert(string table_name, SQLRecord values);
		int sql_insert(string table_name, SQLTable values);
		int sql_update(string table_name, SQLRecord fields, SQLRecord search);
		int sql_delete(string table_name, SQLRecord search);
		
		SQLiteDB(string filename=DB_FILENAME);
		~SQLiteDB();
};

class SQLProxy
{
	private:
		SQLiteDB sqliteDB;
	public:
		int getLastError();
		string getLastErrMsg();
		RESULT get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search);
		int sql_insert(string table_name, SQLRecord values);
		int sql_insert(string table_name, SQLTable values);
		int sql_update(string table_name, SQLRecord fields, SQLRecord search);
		int sql_delete(string table_name, SQLRecord search);


		SQLProxy();
		~SQLProxy();
};

#endif
