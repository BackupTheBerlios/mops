#ifndef SQL_POOL_H_
#define SQL_POOL_H_
#include <sqlite3.h>
#include "core.h"
/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 ************************************************************************************/

bool CheckDatabaseIntegrity();
RESULT get_sql_table (string *sql_query, char ***table, int *rows, int *cols);
RESULT sql_exec (string sql_query);
#endif
