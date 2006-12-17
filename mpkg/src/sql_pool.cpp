
/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 ************************************************************************************/

/** Very important function! Run it before any usage of database. If it fails (and returns false), it means that database contains serious errors (or just empty, 
 * or even doesn't exist), and it is impossible to continue
 */
#include "sql_pool.h"
#include "debug.h"

bool CheckDatabaseIntegrity()
{
	if (\
			sql_exec("select * from dependencies;")!=0 || \
			sql_exec("select * from files;")!=0 || \
			sql_exec("select * from locations;")!=0 || \
			sql_exec("select * from packages;")!=0 || \
			sql_exec("select * from server_tags;")!=0 || \
			sql_exec("select * from server_tags_links;")!=0 || \
			sql_exec("select * from servers;")!=0 || \
			sql_exec("select * from tags;")!=0 || \
			sql_exec("select * from tags_links;")!=0 \
			)
	{
		printf("Database integrity error!");
		return false;
	}

	else 
	{
		printf("Database integrity OK\n");
		return true;
	}
}


RESULT get_sql_table (string *sql_query, char ***table, int *rows, int *cols)
{
	debug("get_sql_table:");
	debug(*sql_query);
	sqlite3 *db;
	char *errmsg=0;
	int sql_return;
	int query_return;
	int _rows;
	int _cols;
	sql_return=sqlite3_open("/var/log/packages.db",&db);
	if (sql_return) { sqlite3_close(db); printf("SQL Error: cannot open database /var/log/packages.db\n"); return 1; } //ERROR
	query_return=sqlite3_get_table(db, sql_query->c_str(), table, rows, cols, &errmsg);
	if (query_return!=SQLITE_OK)
	{
		printf("SQL error while querying database: %s\n", errmsg);
		sqlite3_close(db);
		free(errmsg);
		return query_return;
	}
	free(errmsg);
	sqlite3_close(db);
	return 0;
}	

RESULT sql_exec (string sql_query)
{
	debug("sql_exec:");
	string transaction="begin transaction; "+sql_query+" commit transaction;";
	sql_query=transaction;
	debug(sql_query);
	sqlite3 *db;
	char *sql_errmsg=0;
	int sql_return;
	int query_return;
	sql_return=sqlite3_open("/var/log/packages.db",&db);
	if (sql_return) { sqlite3_close(db); return 1; }// ERROR
	query_return=sqlite3_exec(db,sql_query.c_str(),NULL, NULL, &sql_errmsg);
	if (query_return!=SQLITE_OK)
	{
		printf("SQL error while querying database: %s\n", sql_errmsg);
		free(sql_errmsg);
		sqlite3_close(db);
		return query_return;
	}
	sqlite3_close(db);
	return 0;
}
