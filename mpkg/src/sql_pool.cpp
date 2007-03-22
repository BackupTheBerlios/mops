
/*************************************************************************************
 * 	SQL pool for MOPSLinux packaging system
 * 	Currently supports SQLite only. Planning support for other database servers
 * 	in future (including networked)
 *	$Id: sql_pool.cpp,v 1.19 2007/03/22 16:40:10 i27249 Exp $
 ************************************************************************************/

#include "sql_pool.h"
#include "debug.h"

bool SQLiteDB::CheckDatabaseIntegrity()
{
	// TODO: check if all required field exists.
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
			sql_exec("select * from tags_links;")!=0 || \
			sql_exec("select * from descriptions;")!=0 || \
			sql_exec("select * from changelogs;")!=0 || \
			sql_exec("select * from ratings;")!=0 \
			)
	{
		return false;
	}

	else 
	{
		return true;
	}
}


RESULT SQLiteDB::get_sql_table (string *sql_query, char ***table, int *rows, int *cols)
{
#ifdef _SQL_DEBUG_
	debug("get_sql_table:");
	printf("%s\n", sql_query->c_str());
#endif
	char *errmsg=0;
	int query_return;
	//printf("CONVERSION\n");
	const char *qqq = sql_query->c_str();
	//printf("CALL to GET_TABLE\n");
	query_return=sqlite3_get_table(db, qqq,/* sql_query->c_str(),*/ table, rows, cols, &errmsg);
	//printf("END\n");
	
	if (query_return!=SQLITE_OK) // Means error
	{
		perror("SQLite INTERNAL ERROR");
		printf("sql_pool.cpp: get_sql_table(): SQL error while querying database: %s\n", errmsg);
		printf("The query was: %s\n", sql_query->c_str());
		sqlError=query_return;
		sqlErrMsg=errmsg;
		free(errmsg);
		return query_return;
	}
	
	// If all ok, free memory
	free(errmsg);
	sqlError=0;
	sqlErrMsg.clear();
	return 0;
}	

int SQLiteDB::sqlBegin()
{
	string begin = "begin transaction;";
	return sql_exec(begin);
}

int SQLiteDB::sqlCommit()
{
	string commit = "commit transaction;";
	return sql_exec(commit);
}

int SQLiteDB::sqlFlush()
{
	if (sqlCommit() == 0 && sqlBegin() == 0) return 0;
	else
	{
		printf("Error flushing to DB!\n");
		return -1;
	}
}

int SQLiteDB::init()
{
	int ret;
	ret = sqlite3_open(db_filename.c_str(), &db);
	if (ret!=SQLITE_OK)
	{
		printf("Error opening database, cannot continue\n");
		return 1;
	}
	sqlBegin();
	return ret;

}
RESULT SQLiteDB::sql_exec (string sql_query)
{
#ifdef _SQL_DEBUG_
	debug("sql_exec: "+sql_query);
#endif
	char *sql_errmsg=0;
	int query_return;
	
	query_return=sqlite3_exec(db,sql_query.c_str(),NULL, NULL, &sql_errmsg);
	
	if (query_return!=SQLITE_OK) // Means error
	{
		printf("sql_pool.cpp: sql_exec(): SQL error while querying database: %s\n", sql_errmsg);
		printf("The query was: %s\n", sql_query.c_str());
		sqlError=query_return;
		sqlErrMsg=sql_errmsg;
		free(sql_errmsg);
		return query_return;
	}

	sqlError=0;
	sqlErrMsg.clear();
	return 0;
}

RESULT SQLiteDB::get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search)
{
	string sql_query;
	string sql_action;
	mstring sql_fields;
	string sql_from;
	mstring sql_where;

	char **table;
	int cols;
	int rows;
	// Do what?
	sql_action="select";

	// Get what?
	if (fields.empty()) // If fields is empty, just get all fields
	{
		vector<string> fieldnames=getFieldNames(table_name); // Get field names to fill
		for (unsigned int i=0;i<fieldnames.size();i++)
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
		if (search.getSearchMode()==SEARCH_IN)
		{
			sql_where += search.getFieldName(0) + " in (";
			for (int i=0; i<search.size(); i++)
			{
				sql_where += "'"+search.getValueI(i)+"'";
				if (i!=search.size()-1) sql_where+=", ";
			}
			sql_where += ")";
		}
		else
		{
			for (int i=0;i<search.size();i++)
			{
				if (search.getEqMode()!=EQ_LIKE) sql_where+=search.getFieldName(i) + "='"+search.getValueI(i)+"'";
				if (search.getEqMode()==EQ_LIKE) sql_where+=search.getFieldName(i) + " like '%"+search.getValueI(i)+"%'";
				if (i!=search.size()-1 && search.getSearchMode()==SEARCH_AND) sql_where+=" and ";
				if (i!=search.size()-1 && search.getSearchMode()==SEARCH_OR) sql_where+=" or ";
			}
		}
	}

	sql_query=sql_action+" "+sql_fields.s_str()+" "+sql_from+" "+sql_where.s_str()+";";

	
	lastSQLQuery=sql_query;

	int sql_ret=get_sql_table(&sql_query, &table, &rows, &cols);
	if (sql_ret==0)
	{
		output->clear(); // Ensure that output is clean and empty
		SQLRecord row; 	// One row of data
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
		fieldNames.push_back("file_type");//  INTEGER NOT NULL,
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
	if(table_name=="configfiles")
	{
		fieldNames.push_back("configfile_id");
		fieldNames.push_back("packages_package_id");
		fieldNames.push_back("packages_package_name");
		fieldNames.push_back("configfile_name");
	}
	if(table_name=="configfiles_links")
	{
		fieldNames.push_back("configfiles_link_id");
		fieldNames.push_back("configfiles_configfile_id");
		fieldNames.push_back("packages_package_id");
	}
	
	if (table_name=="descriptions") {
		fieldNames.push_back("description_id");
		fieldNames.push_back("packages_package_id");
		fieldNames.push_back("description_language");
		fieldNames.push_back("description_text");
		fieldNames.push_back("short_description_text");
	}
	
	if (table_name=="changelogs") {
		fieldNames.push_back("changelog_id");
		fieldNames.push_back("packages_package_id");
		fieldNames.push_back("changelog_language");
		fieldNames.push_back("changelog_text");
	}
	
	if (table_name=="ratings") {
		fieldNames.push_back("rating_id");
		fieldNames.push_back("rating_value");
		fieldNames.push_back("packages_package_name");
	}
	return fieldNames;
}

int SQLiteDB::sql_insert(string table_name, SQLRecord values)
{
	
	vector<string> fieldNames=getFieldNames(table_name);
	string sql_query="insert into "+table_name+" values( NULL, ";
	for (unsigned int i=1; i<fieldNames.size();i++)
	{
		sql_query+="'"+values.getValue(fieldNames[i])+"'";
		if (i!=fieldNames.size()-1) sql_query+=", ";
	}
	sql_query+=");";
	return sql_exec(sql_query);
}

int SQLiteDB::sql_insert(string table_name, SQLTable values)
{
	vector<string> fieldNames=getFieldNames(table_name);
	string sql_query;
	for (int k=0; k<values.getRecordCount(); k++)
	{
		sql_query+="insert into "+table_name+" values(NULL, ";
		for (unsigned int i=1; i<fieldNames.size();i++)
		{
			sql_query+="'"+values.getValue(k, fieldNames[i])+"'";
			if (i!=fieldNames.size()-1) sql_query+=", ";
		}
		sql_query+=");";
	}
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
	if (fields.empty()) 
	{
		debug("Fields are empty, cannot update SQL data");
		return -1;
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
	int sql_return;
	sql_return=init();
	
	if (sql_return==1) // Means error
	{
		sqlError=sql_return;
		sqlErrMsg="Error opening database file "+db_filename+", aborting.";
		printf("%s\n", sqlErrMsg.c_str());
	       	sqlite3_close(db);
	       	abort();
       	}
	if (!CheckDatabaseIntegrity())
	{
		printf("Integrity check failed, aborting\n");
		sqlite3_close(db);
		abort();
	}
}

SQLiteDB::~SQLiteDB(){
	printf("closing database and committing\n");
	sqlCommit();
	sqlite3_close(db);
}

int SQLProxy::sqlCommit()
{
	return sqliteDB.sqlCommit();
}

int SQLProxy::sqlBegin()
{
	return sqliteDB.sqlBegin();
}

int SQLProxy::sqlFlush()
{
	return sqliteDB.sqlFlush();
}
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
	return sqliteDB.sql_delete(table_name, search);
}

SQLProxy::SQLProxy(){}
SQLProxy::~SQLProxy(){}
