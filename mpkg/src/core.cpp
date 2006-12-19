/********************************************************************************
 *
 * 			Central core for MOPSLinux package system
 *			TODO: Should be reorganized to objects
 *	$Id: core.cpp,v 1.4 2006/12/19 17:29:09 i27249 Exp $
 *
 ********************************************************************************/

#include "core.h"
#include "debug.h"




//------------------Library front-end--------------------------------

RESULT check_file_conflicts(PACKAGE *package)
{
	debug("check_file_conflicts");
	string package_id;
	PACKAGE tmp;
	char **array;
	int col=0;
	string fname;
	string fname_skip="install";
	int ilength=fname_skip.length();
	for (int i=0;i<package->get_files()->size();i++)
	{
		debug("Checking files...");
		fname_skip.clear();
		fname=package->get_files()->get_file(i)->get_name(false);
		for (int l=1;l<ilength;i++)
			fname_skip+=fname[l];

		if (fname[fname.length()-1]!='/' && fname_skip!="install")
		{
			debug("standard file, running comparsion");
			package_id=get_id("files", "packages_package_id", "file_name", package->get_files()->get_file(i)->get_name(), &array, &col);
			if (package_id!="0")
			{
				for (int k=1;k<=col;k++) // Excluding from check packages with exactly same name - e.g. just other version
				{
					package_id=array[k];
					get_package(package_id, &tmp, false);
					if (tmp.get_name() != package->get_name())
					{
						debug("File conflict found, aborting...");
						return atoi(package_id.c_str());
					}
				}
				sqlite3_free_table(array);
			}
		}
		else debug("File "+fname+" is not standard, skipping comparsion");
	}
	debug ("file conflict checking finished.");
	//tmp.destroy();
	return 0;
}

int check_install_package (PACKAGE *package)
{
	//Searching in DB
	string package_id;
	package_id=get_package_id(package);
	if (package_id=="ERROR")
	{
		printf("Cannot install package: database error\n");
		return CHKINSTALL_DBERROR;
	}
	if (package_id=="0")
	{
		// New methods! Adding to database as available from given location!
		add_package_record(package);
		return check_install_package(package); // Generally, we can simply return AVAILABLE, but we will re-check (file conflicts, etc)...
	}
	else
	{
		int package_status;
		package_status=atoi(get_status(package_id).c_str());
		if (package_status==PKGSTATUS_INSTALL)
		{
			return CHKINSTALL_INSTALL;
		}
		if (package_status==PKGSTATUS_INSTALLED)
		{
			return CHKINSTALL_INSTALLED;
		}
		if (package_status==PKGSTATUS_REMOVE)
		{
			return CHKINSTALL_REMOVE;
		}
		if (package_status==PKGSTATUS_AVAILABLE)
		{
			// Very important procedure - if an earlier/newer version of package is installed?
			string other_ver_id;
			int other_ver_status;
			PACKAGE_LIST others;
			get_packagelist("select * from packages where package_name='"+package->get_name()+"';", &others);
			if (!others.IsEmpty())
				return CHKINSTALL_AVAILABLE; 

			if (check_file_conflicts(package)==0) return CHKINSTALL_AVAILABLE;
			else return CHKINSTALL_FILECONFLICT;
		}
		if (package_status==PKGSTATUS_UNAVAILABLE)
		{
			if (package->get_locations()->get_location(0)->get_local()) return CHKINSTALL_AVAILABLE; // Local install - we can install!
			return CHKINSTALL_UNAVAILABLE;
		}
		if (package_status==PKGSTATUS_REMOVE_PURGE)
		{
			set_status(package_id, PKGSTATUS_INSTALLED);
			return CHKINSTALL_REMOVE_PURGE;
		}
		if (package_status==PKGSTATUS_PURGE)
		{
			return CHKINSTALL_PURGE;
		}
		// If nothing detected - so, status is NULL - it cannot be...
		debug("Check_install_package: no status detected - ERROR!!!!!!!!!");
		return -1;
	}
}

string install_package (PACKAGE *package)
{
	//Searching in DB
	string package_id=get_package_id(package);
	if (package_id=="ERROR")
	{
		printf("Cannot install package: database error\n");
		return "ERROR";
	}
	if (package_id=="0")
	{
		package->set_status(PKGSTATUS_INSTALL);
		return add_package_record(package);
	}
	else
	{
		int package_status=atoi(get_status(package_id).c_str());
		if (package_status==PKGSTATUS_INSTALL)
		{
			printf ("Already marked for install\n");
			return "ALREADY_MARKED";
		}
		if (package_status==PKGSTATUS_INSTALLED)
		{
			printf("Package is already installed\n");
			return "ALREADY_INSTALLED";
		}
		if (package_status==PKGSTATUS_REMOVE)
		{
			printf("Package is already installed and marked to remove\n");
			return "MARKED_TO_REMOVE";
		}
		
		//Check file conflicts
		if (check_file_conflicts(package)==0)
		{
			set_status(package_id, PKGSTATUS_INSTALL);
		}
		else return "FILE_CONFLICT";
		return package_id;
	}
}

RESULT remove_package (PACKAGE *package)
{
	set_status(IntToStr(package->get_id()), PKGSTATUS_REMOVE);
	return 0;
}

RESULT clean_package_filelist (PACKAGE *package)
{
	sql_exec("delete from files where packages_package_id="+IntToStr(package->get_id()));
}

PACKAGE_LIST resolve_dependencies (PACKAGE *package)
{
//TODO
	PACKAGE_LIST pkglist;
	if (package->get_status()==PKGSTATUS_INSTALL)
	{
	}
	return pkglist;
}	

// Adds fingle file linked to package_id
RESULT add_file_record(string package_id, FILES *file)
{
	string sql_query;
	sql_query="insert into files values(NULL, '"+\
		   file->get_name()+T+\
		   file->get_size()+T+\
		   package_id+"');";
	RESULT ret=sql_exec(sql_query);
	return ret;
}

// Adds file list linked to package_id (usually for package adding)
RESULT add_filelist_record(string package_id, FILE_LIST *filelist)
{
	string sql_query;
	sql_query.clear();
	for (int i=0;i<filelist->size();i++)
	{
		sql_query+="insert into files values(NULL, '"+\
			   filelist->get_file(i)->get_name()+T+\
			   filelist->get_file(i)->get_size()+T+\
			   package_id+"'); ";
	}
	if (!sql_query.empty())
		sql_exec(sql_query);

	
	RESULT ret=0;//=sql_exec(sql_query);
	return ret;
}

// Adds location list linked to package_id
RESULT add_locationlist_record(string package_id, LOCATION_LIST *locationlist)
{
	debug("ADDING LOCATION LIST");
	string sql_query;
	sql_query.clear();
	string serv_id;
	for (int i=0;i<locationlist->size();i++)
	{
		debug("Adding location #"+IntToStr(i));
		if (!locationlist->get_location(i)->get_server()->IsEmpty())
		{
			debug("Adding SERVER");
			serv_id=IntToStr(add_server_record(locationlist->get_location(i)->get_server()));
			sql_query+="insert into locations values (NULL, '"+\
			    package_id+T+\
			    serv_id+T+\
			    locationlist->get_location(i)->get_path()+"');";
		}
		else debug("Location incomplete, cannot add");

	}
	RESULT ret=1;
	if (sql_query.empty()) debug ("No valid locations found");
	else ret=sql_exec(sql_query);
	return ret;
}

RESULT add_server_record(SERVER *server)
{
	string sql_query;
	string server_id;
	sql_query.clear();
	RESULT ret;
	server_id=get_id("servers","server_id","server_url",server->get_url());
	if(server_id=="0")
	{
		sql_query="insert into servers values(NULL, '"+\
			   server->get_url()+T+\
			   server->get_priority()+"');";
		ret=sql_exec(sql_query);
		server_id=get_last_id("servers","server_id");
		debug("--------------------------------------------------------------->>>SERVER ID="+server_id+"<------------------------------------------------");
		if (!server->get_tags()->IsEmpty() && server_id!="0") add_server_taglist_record(server_id, server->get_tags());
	}
	return atoi(server_id.c_str());
}

RESULT add_server_taglist_record (string server_id, SERVER_TAG_LIST *server_tag_list)
{
	string sql_query;
	string tag_id;
	for (int i=0; i<server_tag_list->size();i++)
	{
		tag_id=get_id("server_tags","server_tag_id","server_tag_name",server_tag_list->get_server_tag(i)->get_name());
		// If tag doesn't exist in database - adding it. We make it by one query per tag, because tag count is wery small
		if (tag_id=="0")
		{
			sql_query="insert into server_tags values (NULL, '"+\
				   server_tag_list->get_server_tag(i)->get_name()+"');";
			sql_exec(sql_query);
			sql_query.clear();
			tag_id=get_id("server_tags","server_tag_id","server_tag_name",server_tag_list->get_server_tag(i)->get_name());
		}
		add_server_tag_link(server_id, tag_id);
	}
	return 0;
}

// Creates a link between package_id and tag_id
RESULT add_server_tag_link(string server_id, string tag_id)
{
	// Now we have a tag_id, and can write links between tags and package. Inserting data into tags_links table
	string sql_query;
	sql_query.clear();
	sql_query="insert into server_tags_links values (NULL, '"+\
		   server_id+T+\
		   tag_id+"');";
	sql_exec(sql_query);
	return 0;
}

//------------------------------------------------------------------------------------------	
// Adds dependency list linked to package_id
RESULT add_dependencylist_record(string package_id, DEPENDENCY_LIST *deplist)
{
	string sql_query;
	sql_query.clear();
	string dep_condition;
	string dep_type;
	for (int i=0;i<deplist->size();i++)
	{
		dep_condition=deplist->get_dependency(i)->get_condition();
		dep_type=deplist->get_dependency(i)->get_type();
		sql_query+="insert into dependencies values(NULL, '"+\
			    package_id+T+\
			    dep_condition+T+\
			    dep_type+T+\
			    deplist->get_dependency(i)->get_package_name()+T+\
			    deplist->get_dependency(i)->get_package_version()+"');";
	}
	RESULT ret=sql_exec(sql_query);
//	printf("add_dependencylist_record()\n");
	return ret;
}

// Adds tag list linked to package_id. It checks existance of tag in tags table, and creates if not. Next, it calls add_tag_link() to link package and tags
RESULT add_taglist_record (string package_id, TAG_LIST *taglist)
{
	string sql_query;
	string tag_id;
	for (int i=0; i<taglist->size();i++)
	{
		tag_id=get_id("tags","tags_id","tags_name",taglist->get_tag(i)->get_name());
		// If tag doesn't exist in database - adding it. We make it by one query per tag, because tag count is wery small
		if (tag_id=="0")
		{
			sql_query="insert into tags values (NULL, '"+\
				   taglist->get_tag(i)->get_name()+"');";
			sql_exec(sql_query);
			sql_query.clear();
			tag_id=get_id("tags","tags_id","tags_name",taglist->get_tag(i)->get_name());
		}
		add_tag_link(package_id, tag_id);
	}
	return 0;
}

// Creates a link between package_id and tag_id
RESULT add_tag_link(string package_id, string tag_id)
{
	string sql_query;
	sql_query.clear();
	sql_query="insert into tags_links values (NULL, '"+\
		   package_id+T+\
		   tag_id+"');";
	sql_exec(sql_query);
	return 0;
}



// Adds package - full structure (including files, locations, deps, and tags), returning package_id
string add_package_record (PACKAGE *package)
{
	string sql_query;
	string pkg_status;
	pkg_status=IntToStr(package->get_status());
	debug("package status: "+pkg_status);
	// INSERT INTO PACKAGES
	sql_query="insert into packages values(NULL,'"+\
		   package->get_name()+T+\
		   package->get_version()+T+\
		   package->get_arch()+T+\
		   package->get_build()+T+\
		   package->get_compressed_size()+T+\
		   package->get_installed_size()+T+\
		   package->get_short_description()+T+\
		   package->get_description()+T+\
		   package->get_changelog()+T+\
		   package->get_packager()+T+\
		   package->get_packager_email()+T+\
		   pkg_status+T+\
		   package->get_md5()+T+\
		   package->get_filename()+"');";
	debug("EXECUTING ADD PACKAGE!!!");
	
	sql_exec(sql_query);
	sql_query.clear();
	
	// Retrieving package ID
	string package_id;
	package_id=get_last_id("packages", "package_id");
	
	// INSERT INTO FILES
//	printf("inserting into files...\n");
	if (!package->get_files()->IsEmpty()) add_filelist_record(package_id, package->get_files());
//	else printf("no file list found\n");
	
	// INSERT INTO LOCATIONS
	if (!package->get_locations()->IsEmpty()) add_locationlist_record(package_id, package->get_locations());
//	else printf("no locations found\n");

	//INSERT INTO DEPENDENCIES
	if (!package->get_dependencies()->IsEmpty()) add_dependencylist_record(package_id, package->get_dependencies());
//	else printf("no dependencies found\n");

	// INSERT INTO TAGS
	if (!package->get_tags()->IsEmpty()) add_taglist_record(package_id, package->get_tags());
//	else printf("no tags found\n");
	
//	printf("add_package_record()\n");
	return package_id;
}	


//--------------------Mid-level functions-------------------------

RESULT get_package(string package_id, PACKAGE *package, bool GetExtraInfo)
{
	char **table;
	int rows;
	int cols;
	string query="select * from packages where package_id='"+package_id+"';";
//	printf("DEBUG: %s\n", query.c_str());
	get_sql_table(&query,&table, &rows, &cols);
//	printf("DEBUG: rows=%d, cols=%d\n",rows, cols);
	int id;
	if (rows==0)
	{
		sqlite3_free_table(table);
		return 1;
	}
	id=cols;
	package->set_id(atoi(package_id.c_str()));
	package->set_name(table[id+1]);
	package->set_version(table[id+2]);
	package->set_arch(table[id+3]);
	package->set_build(table[id+4]);
	package->set_compressed_size(table[id+5]);
	package->set_installed_size(table[id+6]);
	package->set_short_description(table[id+7]);
	package->set_description(table[id+8]);
	package->set_changelog(table[id+9]);
	package->set_packager(table[id+10]);
	package->set_packager_email(table[id+11]);
	package->set_status(atoi(table[id+12]));
	package->set_md5(table[id+13]);
	package->set_filename(table[id+14]);
	if (GetExtraInfo)
	{
		debug("EXTRACTING EXTRA INFO");
		get_filelist(package_id, package->get_files());
		get_locationlist(package_id, package->get_locations());
		get_dependencylist(package_id, package->get_dependencies());
		get_taglist(package_id, package->get_tags());
	}
	else debug ("SKIPPING EXTRA INFO");
	sqlite3_free_table(table);
	return 0;
}




RESULT get_packagelist (string query, PACKAGE_LIST *packagelist)
{
	char **table;
	int rows;
	int cols;
	get_sql_table(&query,&table, &rows, &cols);
	PACKAGE package;
	int id;
	string package_id;
	if (rows==0)
	{
		sqlite3_free_table(table);
		return 1;
	}
	for (int row=1;row<=rows;row++)
	{
		id=row*cols;
		package_id=table[id];
		package.set_id(atoi(package_id.c_str()));
		package.set_name(table[id+1]);
		package.set_version(table[id+2]);
		package.set_arch(table[id+3]);
		package.set_build(table[id+4]);
		package.set_compressed_size(table[id+5]);
		package.set_installed_size(table[id+6]);
		package.set_short_description(table[id+7]);
		package.set_description(table[id+8]);
		package.set_changelog(table[id+9]);
		package.set_packager(table[id+10]);
		package.set_packager_email(table[id+11]);
		package.set_status(atoi(table[id+12]));
		package.set_md5(table[id+13]);
		package.set_filename(table[id+14]);

		get_filelist(package_id, package.get_files());		//TODO: change all get_[*]list functions to returning value instead of direct filling
		get_locationlist(package_id, package.get_locations());
		get_dependencylist(package_id, package.get_dependencies());
		get_taglist(package_id, package.get_tags());
		debug("get_packagelist: got package with "+IntToStr(package.get_locations()->size())+" locations...");
		packagelist->add(package);
	}
	sqlite3_free_table(table);
	return 0;
}


RESULT get_filelist(string package_id, FILE_LIST *filelist)
{
	string sql_query="select * from files where packages_package_id="+package_id+";";
	char **table;
	int rows;
	int cols;
	get_sql_table(&sql_query, &table, &rows, &cols);
	FILES file;
	int id;
	if (rows==0) 
	{
		sqlite3_free_table(table);
		return 1;
	}
	for (int row=1;row<=rows;row++)
	{
		id=row*cols;
		file.set_name(table[id+1]);
		file.set_size(table[id+2]);
		filelist->add(file);
	}
	sqlite3_free_table(table);
	return 0;
}

RESULT get_dependencylist(string package_id, DEPENDENCY_LIST *deplist)
{
	string sql_query="select * from dependencies where packages_package_id="+package_id+";";
	char **table;
	int rows;
	int cols;
	get_sql_table(&sql_query, &table, &rows, &cols);
	//DEPENDENCY_LIST deplist;
	DEPENDENCY dependency;
	int id;
	if (rows==0)
	{
		sqlite3_free_table(table);
		return 1;
	}
	for (int row=1; row<=rows;row++)
	{
		id=row*cols;
		dependency.set_condition(table[id+2]);
		dependency.set_type(table[id+3]);
		dependency.set_package_name(table[id+4]);
		dependency.set_package_version(table[id+5]);
		deplist->add(dependency);
	}
	sqlite3_free_table(table);
	return 0;
}

RESULT get_taglist(string package_id, TAG_LIST *taglist)
{
	// Step 1. Read link table, and create a list of tag's ids.
	string id_sql_query="select tags_tag_id from tags_links where packages_package_id="+package_id+";";
	TAG tag;
	//TAG_LIST taglist;
	char **id_table;
	int id_rows;
	int id_cols;
	get_sql_table(&id_sql_query, &id_table, &id_rows, &id_cols);
	string sql_query="select tags_name from tags where tags_id='";
	if (id_rows!=0)
	{
		for (int i=1;i<=id_rows;i++)
		{
			sql_query+=id_table[i];
			if (i!=id_rows) sql_query+="' or tags_id='";
			else sql_query+="';";
		}
		sqlite3_free_table(id_table);
	}
	else
	{
		sqlite3_free_table(id_table);
		return 1;
	}
	
	// Step 2. Read the tags with readed ids
	char **table;
	int rows;
	int cols;
	get_sql_table(&sql_query, &table, &rows, &cols);
	if (rows!=0)
	{
		for (int i=1; i<=rows;i++)
		{
			tag.set_name(table[i]);
			taglist->add(tag);
		}
	}
	sqlite3_free_table(table);
	return 0;
}

RESULT get_server_tag_list(string server_id, SERVER_TAG_LIST *server_tag_list)
{
	// Step 1. Read link table, and create a list of server_tags id's.
	string id_sql_query="select server_tags_server_tag_id from server_tags_links where servers_server_id="+server_id+";";
	SERVER_TAG server_tag;
	//SERVER_TAG_LIST server_tag_list;
	char **id_table;
	int id_rows;
	int id_cols;
	get_sql_table(&id_sql_query, &id_table, &id_rows, &id_cols);
	string sql_query="select server_tag_name from server_tags where server_tag_id=";
	if (id_rows!=0)
	{
		for (int i=1;i<=id_rows;i++)
		{
			sql_query+=id_table[i];
			if (i!=id_rows) sql_query+=" of server_tag_id=";
			else sql_query+=";";
		}
		sqlite3_free_table(id_table);
	}
	else 
	{
		sqlite3_free_table(id_table);

		return 1; //in case of empty result - nothing to return, we may don't query the database
	}

	// Step 2. Read the tags with readed id's
	
	char **table;
	int rows;
	int cols;
	get_sql_table(&sql_query, &table, &rows, &cols);
	if (rows!=0)
	{
		for (int i=1;i<=rows;i++)
		{
			server_tag.set_name(table[i]);
			server_tag_list->add(server_tag);
		}
	}
	sqlite3_free_table(table);
	return 0;
}


RESULT get_server(string server_id, SERVER *server)
{
	debug("core: get_server");
	get_server_tag_list(server_id, server->get_tags());

	char **table;
	int rows;
	int cols;
	string sql_query="select * from servers where server_id="+server_id+";";
	get_sql_table(&sql_query, &table, &rows, &cols);
	if (rows==1)
	{
		server->set_id(atoi(table[3]));
		server->set_url(table[4]);
		server->set_priority(table[5]);
	}
	else
	{
		debug("No server with this ID found. Returning an \"empty\" server - setting server ID to 0");
		server->set_id(0);
		sqlite3_free_table(table);
		return 1; //ERROR: either server not found in DB, or there are multiple servers with same id
	}

	// Going next - filling server tags. TODO

	sqlite3_free_table(table);
	return 0;
}
	

RESULT get_locationlist(string package_id, LOCATION_LIST *location_list)
{
	LOCATION location;
	SERVER server;
	string server_id;
	char **table;
	int rows;
	int cols;
	debug("Package ID="+package_id);
	string sql_query="select * from locations where packages_package_id='"+package_id+"';";
	get_sql_table(&sql_query, &table, &rows, &cols);
	debug("Locations found: "+IntToStr(rows)+"/"+IntToStr(cols));
	if (rows!=0)
	{
		string a=table[0];
		string b=table[1];
		string c;
		string d;
		debug("\n\n");
		for (int i=0; i<8;i++)
			printf("%s|",table[i]);
		printf("\n\n");
		debug("EXTRACTING SERVER, "+a+" | "+b);
		for (int i=4;i<(rows+1)*cols; i=i+cols)
		{
			a=table[i];
			b=table[i+1];
			c=table[i+2];
			d=table[i+3];
			debug("Location ID: "+a+", Package ID: "+b+", Server ID: "+c+", Path: "+d);
			location.set_id(atoi(a.c_str()));
			debug("Location ID set.");
			get_server(c, &server);
			debug("Got server");
			location.set_server(server);
			debug("Set server");
			location.set_path(d);
			debug("Set path");
			location_list->add(location);
			debug("LOCATION ADDED");
		}
	}
	else
	{
		sqlite3_free_table(table);
		return 1;
	}
	debug("[GET_LOCATIONLIST::] Returning "+IntToStr(location_list->size())+" locations");
	sqlite3_free_table(table);
	return 0;
}


string get_last_id(string table_name, string id_field)
{
	char **table;
	int rows;
	int cols;
	string sql_query="select package_id from packages;";
	if(get_sql_table(&sql_query, &table, &rows, &cols)==0)
	{
		string last_id;
		last_id=table[rows];
		sqlite3_free_table(table);
		return last_id;
	}
	else
	{
		sqlite3_free_table(table);
		return "0";
	}
}

string get_id(string table_name, string id_field, string search_field, string search_value, char ***array, int *col)
{
	char **table;
	int rows;
	int cols;
	string sql_query="select "+id_field+" from "+table_name+" where "+search_field+"='"+search_value+"';";
	get_sql_table(&sql_query, &table, &rows, &cols);
	if (rows==0) 
	{
		sqlite3_free_table(table);
		return "0";
	}
	if (rows>1)
	{
		array=&table; // if multiple id found... 
	}
	col=&rows;
	string id=table[rows]; // It always returns the last found ID
	sqlite3_free_table(table);
	return id;
}

string get_package_id(PACKAGE &package)
{
	char **table;
	int rows;
	int cols;
	string ret;
	string sql_query="select * from packages where package_name='"+package.get_name()+\
			  "' and package_version='"+package.get_version()+\
			  "' and package_arch='"+package.get_arch()+\
			  "' and package_build='"+package.get_build()+"';";
	get_sql_table(&sql_query, &table, &rows, &cols);
	if (rows==0)
	{
		sqlite3_free_table(table);
		return "0";
	}
	if (rows==1)
	{
		ret = table[cols];
		sqlite3_free_table(table);
		return ret;
	}
	if (rows>1)
	{
		//multiple packages with this data, getting extra search
		printf("Error in database data, multiple identical packages detected\n");
		sqlite3_free_table(table);
		return "ERROR";
	}
}


void set_status(string package_id, int status)
{
	string sql_query;
	string stat;
	stat=IntToStr(status);
       	sql_query = "update packages set package_status='"+stat+"' where package_id="+package_id+";";
	sql_exec(sql_query);
}


string get_status(string package_id)
{
	char **table;
	int rows;
	int cols;
	string sql_query="select package_status from packages where package_id="+package_id+";";
	get_sql_table(&sql_query, &table, &rows, &cols);
	if (rows==0)
	{
		sqlite3_free_table(table);
		return "";
	}
	string status;
	status=table[rows];
	sqlite3_free_table(table);
	return status;
}

// Retrieves package ID using md5 checksum. 
string get_package_id(PACKAGE *package)
{
	char **table;
	int rows;
	int cols;
	string sql_query="select package_id from packages where package_md5='"+package->get_md5()+"';";
	get_sql_table(&sql_query, &table, &rows, &cols);
	if (rows==0)
	{
		sqlite3_free_table(table);
		return "0"; //This means not found
	}
	string package_id;
	package_id=table[rows];
	sqlite3_free_table(table);
	return package_id;
}

string get_version(string package_id)
{
	string version;
	char **table;
	int rows;
	int cols;
	string sql_query="select package_version from packages where package_id="+package_id+";";
	get_sql_table(&sql_query, &table, &rows, &cols);
	version=table[rows];
	sqlite3_free_table(table);
	return version;
}



