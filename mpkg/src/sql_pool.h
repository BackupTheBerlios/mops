/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 *	$Id: sql_pool.h,v 1.3 2006/12/24 12:47:21 i27249 Exp $
 ************************************************************************************/



#ifndef SQL_POOL_H_
#define SQL_POOL_H_
#include <sqlite3.h>
#include "core.h"

bool CheckDatabaseIntegrity();
RESULT get_sql_table (string *sql_query, char ***table, int *rows, int *cols); // fills table using 
RESULT get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name); // writes output to output, get required fields and search values from fields.
RESULT sql_exec (string sql_query);
#endif
