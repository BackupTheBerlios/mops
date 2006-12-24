
/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 *	$Id: sql_pool.cpp,v 1.6 2006/12/24 14:13:22 i27249 Exp $
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
			sql_exec("select * from scripts;")!=0 || \
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
#ifdef _SQL_DEBUG_
	debug("get_sql_table:");
	debug(*sql_query);
#endif
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
	string transaction="begin transaction; "+sql_query+" commit transaction;";
	sql_query=transaction;
#ifdef _SQL_DEBUG_
	debug("sql_exec: "+sql_query);
#endif
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

RESULT get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search)
{
	string sql_query;
	string sql_action;
	string sql_fields;
	string sql_from;
	string sql_where;

	char **table;
	int cols;
	int rows;
	// Do what?
	sql_action="select";

	// Get what?
	if (fields.empty()) // If fields is empty, just get all fields
	{
		sql_fields+="*";
	}

	for (int i=0; i<fields.size();i++) // Otherwise, get special fields
	{
		sql_query+=fields.getFieldName(i);
		if (i!=fields.size()-1) sql_query+=", ";
	}

	sql_from = "from "+table_name;

	if (!search.empty())
	{
		sql_where="where ";
		for (int i=0;i<search.size();i++)
		{
			sql_where+=search.getFieldName(i) + "='"+search.getValueI(i)+"'";
			if (i!=search.size()-1) sql_where+=" and ";
		}
	}

	sql_query=sql_action+" "+sql_fields+" "+sql_from+" "+sql_where+";";

	printf("[vtable] %s\n", sql_query.c_str());

	int sql_ret=get_sql_table(&sql_query, &table, &rows, &cols);
	if (sql_ret==0)
	{
		output->clear(); // Ensure that output is clean and empty
		SQLRecord row;
		string _fieldname;
		for (int i=0;i<cols;i++)
		{
			_fieldname=table[i];
			row.addField(_fieldname);
		}
		fields=row;
		
		// value == cols*row_num + shift;
		
		int field_num=0;
		for (int current_row=1; current_row<=rows; current_row++)
		{
			field_num=0;
			row=fields;
			for (int value_pos=cols*current_row; value_pos<cols*(current_row+1); value_pos++)
			{
				row.setValue((string) table[field_num], (string) table[value_pos]);
			}
			output->addRecord(row);
		}
		sqlite3_free_table(table);
		return 0;
	}
	else return sql_ret;
}

