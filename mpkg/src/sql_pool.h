/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 *	$Id: sql_pool.h,v 1.16 2007/05/18 07:35:33 i27249 Exp $
 ************************************************************************************/



#ifndef SQL_POOL_H_
#define SQL_POOL_H_
#include <sqlite3.h>
#include "debug.h"
#include "core.h"
#include "config.h"
#include "string_operations.h"
#include "file_routines.h"
//#include "mpkg.h"
class SQLiteDB
{
	private:
		string db_filename;
		int sqlError;
		bool initOk;
		string sqlErrMsg;
		string lastSQLQuery;
		int sql_exec (string sql_query);
		int initDatabaseStructure();
		bool CheckDatabaseIntegrity(); // Checks database integrity
		int get_sql_table (string *sql_query, char ***table, int *rows, int *cols); // fills table
		sqlite3 *db; //Database is open all the time during work (added: aix27249, for optimization reasons)
	public:
		vector<string> getFieldNames(string table_name);
		int getLastError();
		string getLastErrMsg();
		int get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search);
		int init();
		int sqlBegin();
		int sqlCommit();
		int sqlFlush();
		long long int getLastID();

		int clear_table(string table_name);
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
		long long int getLastID();
		int getLastError();
		string getLastErrMsg();
		int clear_table(string table_name);
		int get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search);
		int sql_insert(string table_name, SQLRecord values);
		int sql_insert(string table_name, SQLTable values);
		int sql_update(string table_name, SQLRecord fields, SQLRecord search);
		int sql_delete(string table_name, SQLRecord search);
		int sqlCommit();
		int sqlBegin();
		int sqlFlush();


		SQLProxy();
		~SQLProxy();
};

#endif
