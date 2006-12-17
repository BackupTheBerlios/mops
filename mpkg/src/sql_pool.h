/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 *	$Id: sql_pool.h,v 1.2 2006/12/17 19:34:57 i27249 Exp $
 ************************************************************************************/



#ifndef SQL_POOL_H_
#define SQL_POOL_H_
#include <sqlite3.h>
#include "core.h"

bool CheckDatabaseIntegrity();
RESULT get_sql_table (string *sql_query, char ***table, int *rows, int *cols);
RESULT sql_exec (string sql_query);
#endif
