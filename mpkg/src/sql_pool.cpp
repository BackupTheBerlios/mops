
/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 *	$Id: sql_pool.cpp,v 1.8 2006/12/29 12:57:00 i27249 Exp $
 ************************************************************************************/

#include "sql_pool.h"
#include "debug.h"

bool SQLiteDB::CheckDatabaseIntegrity()
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


RESULT SQLiteDB::get_sql_table (string *sql_query, char ***table, int *rows, int *cols)
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
	sql_return=sqlite3_open(db_filename.c_str(),&db);
	
	if (sql_return) // Means error
       	{
		sqlite3_close(db);
	       	printf("SQL Error: cannot open database /var/log/packages.db\n");
		sqlError=sql_return;
		sqlErrMsg="Cannot open database file "+db_filename;
	       	return 1;
       	}
	lastSQLQuery=*sql_query;
	query_return=sqlite3_get_table(db, sql_query->c_str(), table, rows, cols, &errmsg);
	
	if (query_return!=SQLITE_OK) // Means error
	{
		printf("SQL error while querying database: %s\n", errmsg);
		sqlError=query_return;
		sqlErrMsg=errmsg;
		sqlite3_close(db);
		free(errmsg);
		return query_return;
	}
	
	// If all ok, free memory
	free(errmsg);
	sqlError=0;
	sqlErrMsg.clear();
	sqlite3_close(db);
	return 0;
}	

RESULT SQLiteDB::sql_exec (string sql_query)
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
	sql_return=sqlite3_open(db_filename.c_str(),&db);
	
	if (sql_return) // Means error
	{
		sqlError=sql_return;
		sqlErrMsg="Error opening database file "+db_filename;
	       	sqlite3_close(db);
	       	return 1;
       	}

	lastSQLQuery=sql_query;

	query_return=sqlite3_exec(db,sql_query.c_str(),NULL, NULL, &sql_errmsg);
	
	if (query_return!=SQLITE_OK) // Means error
	{
		printf("SQL error while querying database: %s\n", sql_errmsg);
		sqlError=query_return;
		sqlErrMsg=sql_errmsg;
		free(sql_errmsg);
		sqlite3_close(db);
		return query_return;
	}

	sqlite3_close(db);
	sqlError=0;
	sqlErrMsg.clear();
	return 0;
}

RESULT SQLiteDB::get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search)
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
		//sql_fields+="*";
		vector<string> fieldnames=getFieldNames(table_name); // Get field names to fill
		for (int i=0;i<fieldnames.size();i++)
			fields.addField(fieldnames[i]);
	}

	for (int i=0; i<fields.size();i++) // Otherwise, get special fields
	{
		sql_fields+=fields.getFieldName(i);
		if (i!=fields.size()-1) sql_fields+=", ";
	}

	sql_from = "from "+table_name;

	if (!search.empty())
	{
		sql_where="where ";
		for (int i=0;i<search.size();i++)
		{
			sql_where+=search.getFieldName(i) + "='"+search.getValueI(i)+"'";
			if (i!=search.size()-1 && search.getSearchMode()==SEARCH_AND) sql_where+=" and ";
			if (i!=search.size()-1 && search.getSearchMode()==SEARCH_OR) sql_where+=" or ";
		}
	}

	sql_query=sql_action+" "+sql_fields+" "+sql_from+" "+sql_where+";";

//	printf("[vtable] %s\n", sql_query.c_str());
	
	lastSQLQuery=sql_query;

	int sql_ret=get_sql_table(&sql_query, &table, &rows, &cols);
	if (sql_ret==0)
	{
		output->clear(); // Ensure that output is clean and empty
		SQLRecord row; 	// One row of data
		//string _fieldname;
		
		// value == cols*row_num + shift;
		// TODO: ERROR IS SOMETHERE HERE....	
		int field_num=0;
		for (int current_row=1; current_row<=rows; current_row++)
		{
			field_num=0;
			row=fields;
			for (int value_pos=cols*current_row; value_pos<cols*(current_row+1); value_pos++)
			{
				row.setValue(fields.getFieldName(field_num), (string) table[value_pos]);
				field_num++;
			}
			/*for (int z=0;z<row.size();z++)
			{
				//printf("%s: %s\n\n", row.getFieldName(z).c_str(), row.getValueI(z).c_str());
			}*/
			output->addRecord(row);
		}
		sqlite3_free_table(table);
		return 0;
	}
	else return sql_ret;
}

int SQLiteDB::getLastError()
{
	return sqlError;
}

string SQLiteDB::getLastErrMsg()
{
	return sqlErrMsg;
}


vector<string> SQLiteDB::getFieldNames(string table_name)
{
	vector<string> fieldNames;
	if (table_name=="packages")
	{
		fieldNames.push_back("package_id");
		fieldNames.push_back("package_name");
		fieldNames.push_back("package_version");
		fieldNames.push_back("package_arch");// TEXT NOT NULL,
		fieldNames.push_back("package_build");// TEXT NULL,
		fieldNames.push_back("package_compressed_size");// TEXT NOT NULL,
		fieldNames.push_back("package_installed_size");// TEXT NOT NULL,
		fieldNames.push_back("package_short_description");// TEXT NULL,
		fieldNames.push_back("package_description");// TEXT NULL, 
		fieldNames.push_back("package_changelog");// TEXT NULL,
		fieldNames.push_back("package_packager");// TEXT NULL,
		fieldNames.push_back("package_packager_email");// TEXT NULL,
		fieldNames.push_back("package_status");// INTEGER NOT NULL DEFAULT '0',
		fieldNames.push_back("package_md5");// TEXT NOT NULL,
		fieldNames.push_back("package_filename");// NOT NULL
	}
	
	if (table_name=="scripts")
	{
		fieldNames.push_back("script_id");// INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("packages_package_id");//  INTEGER NOT NULL,
		fieldNames.push_back("preinstall");//  TEXT NOT NULL DEFAULT '#!/bin/sh',
		fieldNames.push_back("postinstall");//  TEXT NOT NULL DEFAULT '#!/bin/sh',
		fieldNames.push_back("preremove");//  TEXT NOT NULL DEFAULT '#!/bin/sh',
		fieldNames.push_back("postremove");//  TEXT NOT NULL DEFAULT '#!/bin/sh'
	}
	if(table_name=="files")
	{
		fieldNames.push_back("file_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("file_name");//  TEXT NOT NULL,
		fieldNames.push_back("file_size");//  TEXT NOT NULL,
		fieldNames.push_back("packages_package_id");//  INTEGER NOT NULL
	}
 	if(table_name=="locations")
	{
		fieldNames.push_back("location_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("packages_package_id");//  INTEGER NOT NULL,
		fieldNames.push_back("servers_server_id");//  INTEGER NOT NULL,
		fieldNames.push_back("location_path");//  TEXT NOT NULL
	}
	if(table_name=="servers")
	{
		fieldNames.push_back("server_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("server_url");//  TEXT NOT NULL,
		fieldNames.push_back("server_priority");//  INTEGER NOT NULL DEFAULT '1' 
	}

 	if(table_name=="tags")
	{
		fieldNames.push_back("tags_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("tags_name");//  TEXT NOT NULL
	}

 	if(table_name=="server_tags")
	{
		fieldNames.push_back("server_tag_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("server_tag_name");//  TEXT NOT NULL
	}

 	if(table_name=="server_tags_links")
	{
		fieldNames.push_back("server_tags_link_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("servers_server_id");//  INTEGER NOT NULL,
		fieldNames.push_back("server_tags_server_tag_id");//  INTEGER NOT NULL
	}

 	if(table_name=="tags_links")
	{
		fieldNames.push_back("tags_link_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("packages_package_id");//  INTEGER NOT NULL,
		fieldNames.push_back("tags_tag_id");//  INTEGER NOT NULL
	}

 	if(table_name=="dependencies")
	{
		fieldNames.push_back("dependency_id");//  INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,
		fieldNames.push_back("packages_package_id");//  INTEGER NOT NULL,
		fieldNames.push_back("dependency_condition");//  INTEGER NOT NULL DEFAULT '1',
		fieldNames.push_back("dependency_type");//  INTEGER NOT NULL DEFAULT '1',
		fieldNames.push_back("dependency_package_name");//  TEXT NOT NULL,
		fieldNames.push_back("dependency_package_version");//  TEXT NULL
	}
	return fieldNames;

	
	/*

	//printf("Searching fields in %s table", table_name.c_str());
	char **table;
	int cols;
	int rows;
	vector<string> fieldNames;
	string sql_query="select * from "+table_name+";";
	printf("%s\n", sql_query.c_str());
	int sql_ret=get_sql_table(&sql_query, &table, &rows, &cols);
	if (sql_ret!=0)
	{
		printf("error querying table names\n");
		sqlite3_free_table(table);
		exit(1);
		return fieldNames;
	}
	for (int i=0; i<=cols; i++)
	{
	//	printf("FECHING FIELDS...%d found", cols);
		fieldNames.resize(i+1);
		fieldNames[i]=(string) table[i];
		printf("[getFieldNames] got %s, wrote %s\n", table[i], fieldNames[i].c_str());
	}
	sqlite3_free_table(table);
	return fieldNames;*/
}

int SQLiteDB::sql_insert(string table_name, SQLRecord values)
{
	
	vector<string> fieldNames=getFieldNames(table_name);
	string sql_query="insert into "+table_name+" values( NULL, ";
	for (int i=1; i<fieldNames.size();i++)
	{
		sql_query+="'"+values.getValue(fieldNames[i])+"'";
		if (i!=fieldNames.size()-1) sql_query+=", ";
	}
	sql_query+=");";
//	printf("[sql_insert] %s\n", sql_query.c_str());
	return sql_exec(sql_query);
}

int SQLiteDB::sql_insert(string table_name, SQLTable values)
{
	vector<string> fieldNames=getFieldNames(table_name);
	string sql_query;
	//printf("fields: %i\n", fieldNames.size());
	for (int k=0; k<values.getRecordCount(); k++)
	{
		sql_query+="insert into "+table_name+" values(NULL, ";
		for (int i=1; i<fieldNames.size();i++)
		{
			sql_query+="'"+values.getValue(k, fieldNames[i])+"'";
			if (i!=fieldNames.size()-1) sql_query+=", ";
		}
		sql_query+=");";
	}
//	printf("[sql_insert] %s\n", sql_query.c_str());

	return sql_exec(sql_query);
}


int SQLiteDB::sql_update(string table_name, SQLRecord fields, SQLRecord search)
{
	string sql_query="update "+table_name+" set ";
	for (int i=0;i<fields.size(); i++)
	{
		sql_query+=fields.getFieldName(i)+"='"+fields.getValue(fields.getFieldName(i))+"'";
		if (i!=fields.size()-1) sql_query+=", ";
	}
	if (!search.empty())
	{
		sql_query+=" where ";
		for (int i=0; i<search.size(); i++)
		{
			sql_query+=search.getFieldName(i)+"='"+search.getValue(search.getFieldName(i))+"'";
		       	if (i!=search.size()-1 && search.getSearchMode()==SEARCH_AND) sql_query+=" and ";
			if (i!=search.size()-1 && search.getSearchMode()==SEARCH_OR) sql_query+=" or ";
		}
	}
	sql_query+=";";
	return sql_exec(sql_query);
}

int SQLiteDB::sql_delete(string table_name, SQLRecord search)
{
	string sql_query="delete from "+table_name;
	if (!search.empty())
	{
		sql_query+=" where ";
		for (int i=0; i<search.size(); i++)
		{
			sql_query+=search.getFieldName(i)+"='"+search.getValue(search.getFieldName(i))+"'";
		       	if (i!=search.size()-1 && search.getSearchMode()==SEARCH_AND) sql_query+=" and ";
			if (i!=search.size()-1 && search.getSearchMode()==SEARCH_OR) sql_query+=" or ";
		}
	}
	sql_query+=";";
	return sql_exec(sql_query);
}

SQLiteDB::SQLiteDB(string filename)
{
#ifdef DEBUG
	printf("Database filename: %s\n", filename.c_str());
#endif
	db_filename=filename;
	sqlError=0;
	CheckDatabaseIntegrity();
}

SQLiteDB::~SQLiteDB(){}

int SQLProxy::getLastError()
{
	return sqliteDB.getLastError();
}

string SQLProxy::getLastErrMsg()
{
	return sqliteDB.getLastErrMsg();
}

RESULT SQLProxy::get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search)
{
	return sqliteDB.get_sql_vtable(output, fields, table_name, search);
}

/*RESULT SQLProxy::sql_exec (string sql_query);
{
	return sql_exec(sql_query);
}*/

int SQLProxy::sql_insert(string table_name, SQLRecord values)
{
	return sqliteDB.sql_insert(table_name, values);
}

int SQLProxy::sql_insert(string table_name, SQLTable values)
{
	return sqliteDB.sql_insert(table_name, values);
}

int SQLProxy::sql_update(string table_name, SQLRecord fields, SQLRecord search)
{
	return sqliteDB.sql_update(table_name, fields, search);
}

int SQLProxy::sql_delete(string table_name, SQLRecord search)
{
	sqliteDB.sql_delete(table_name, search);
}

SQLProxy::SQLProxy(){}
SQLProxy::~SQLProxy(){}
