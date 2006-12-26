/********************************************************************************
 *
 * 			Central core for MOPSLinux package system
 *			TODO: Should be reorganized to objects
 *	$Id: core.cpp,v 1.10 2006/12/26 18:57:11 i27249 Exp $
 *
 ********************************************************************************/

#include "core.h"
#include "debug.h"
#include "mpkg.h"



//------------------Library front-end--------------------------------

int mpkgDatabase::check_file_conflicts(PACKAGE *package)
{
	return 0; // Skip checking for some time - I think it doesn't work
	int package_id;
	PACKAGE tmp;
	string fname;
	string fname_skip="install";
	int ilength=fname_skip.length();
	SQLTable sqlTable;
	SQLRecord sqlFields;
	SQLRecord sqlSearch;
	for (int i=0;i<package->get_files()->size();i++)
	{
		//printf("Checking files...\n");
		fname_skip.clear();
		fname=package->get_files()->get_file(i)->get_name(false);
		for (int l=1;l<ilength;i++)
			fname_skip+=fname[l];

		if (fname[fname.length()-1]!='/' && fname_skip!="install")
		{
			sqlTable.clear();
			sqlFields.clear();
			sqlSearch.clear();
			sqlFields.addField("packages_package_id");
			sqlSearch.addField("file_name", package->get_files()->get_file(i)->get_name());
		//	printf("!!!!!!!!!!!!!!!!!!!!\n");
			db.get_sql_vtable(&sqlTable, sqlFields, "files", sqlSearch);
			
			//package_id=get_id("files", "packages_package_id", "file_name", package->get_files()->get_file(i)->get_name(), &array, &col);
			if (!sqlTable.empty())
			{
				for (int k=0;k<sqlTable.getRecordCount() ;k++) // Excluding from check packages with exactly same name - e.g. just other version
				{
					package_id=atoi(sqlTable.getValue(k, "packages_package_id").c_str());
					get_package(package_id, &tmp, false);
					if (tmp.get_name() != package->get_name())
					{
						debug("File conflict found, aborting...");
						return package_id;
					}
				}
			}
		}
		else debug("File "+fname+" is not standard, skipping comparsion");
	}
	debug ("file conflict checking finished.");
	return 0;
}

int mpkgDatabase::check_install_package (PACKAGE *package)
{
	debug("check_install_package start");
	//Searching in DB
	int package_id;
	package_id=get_package_id(package);
	if (package_id<0)
	{
		printf("Cannot install package: database error\n");
		return CHKINSTALL_DBERROR;
	}
	if (package_id==0)
	{
		// New methods! Adding to database as available from given location!
		add_package_record(package);
		return check_install_package(package); // Generally, we can simply return AVAILABLE, but we will re-check (file conflicts, etc)...
	}
	else
	{
		int package_status;
		package_status=get_status(package_id);
		debug("got status " + IntToStr(package_status));
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
			// Very important procedure - if an earlier/newer version of package is installed? (!!!NOT WORKING!!!)
			string other_ver_id;
			int other_ver_status;
			PACKAGE_LIST others;
			SQLRecord sqlSearch;
			sqlSearch.addField("package_name", package->get_name());
			sqlSearch.addField("package_status", IntToStr(PKGSTATUS_INSTALLED)); //TODO: add other installed flags
			get_packagelist(sqlSearch, &others);
			if (!others.IsEmpty())
			{
				debug(IntToStr(others.size())+" OTHER installed VERSIONS FOUND");
				return CHKINSTALL_AVAILABLE;
			}	
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

string mpkgDatabase::_install_package (PACKAGE *package)
{
	//Searching in DB
	int package_id=get_package_id(package);
	if (package_id<0)
	{
		printf("Cannot install package: database error\n");
		return "ERROR";
	}
	if (package_id==0)
	{
		package->set_status(PKGSTATUS_INSTALL);
		return IntToStr(add_package_record(package));
	}
	else
	{
		int package_status=get_status(package_id);
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
		return IntToStr(package_id);
	}
}

int mpkgDatabase::_remove_package (PACKAGE *package)
{
	set_status(package->get_id(), PKGSTATUS_REMOVE);
	return 0;
}

int mpkgDatabase::clean_package_filelist (PACKAGE *package)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package->get_id()));
	return db.sql_delete("files", sqlSearch);
}

// Adds file list linked to package_id (usually for package adding)
int mpkgDatabase::add_filelist_record(int package_id, FILE_LIST *filelist)
{
	SQLTable sqlTable;
	SQLRecord sqlValues;
	for (int i=0;i<filelist->size();i++)
	{
		sqlValues.clear();
		sqlValues.addField("file_name", filelist->get_file(i)->get_name());
		sqlValues.addField("packages_package_id", IntToStr(package_id));
		sqlTable.addRecord(sqlValues);
	}
	if (!sqlTable.empty())
	{
		return db.sql_insert("files", sqlTable);
	}
	else return 0;
}

// Adds location list linked to package_id
int mpkgDatabase::add_locationlist_record(int package_id, LOCATION_LIST *locationlist)
{
	debug("ADDING LOCATION LIST");
	int serv_id;
	SQLTable sqlLocations;
	SQLRecord sqlLocation;
	for (int i=0;i<locationlist->size();i++)
	{
		debug("Adding location #"+IntToStr(i));
		if (!locationlist->get_location(i)->get_server()->IsEmpty())
		{
			debug("Adding SERVER");
			serv_id=add_server_record(locationlist->get_location(i)->get_server());
			sqlLocation.clear();
			sqlLocation.addField("servers_server_id", IntToStr(serv_id));
			sqlLocation.addField("packages_package_id", IntToStr(package_id));
			sqlLocation.addField("location_path", locationlist->get_location(i)->get_path());
			sqlLocations.addRecord(sqlLocation);
		}
		else debug("Location incomplete, cannot add");
	}
	int ret=1;
	if (sqlLocations.empty()) debug ("No valid locations found");
	else ret=db.sql_insert("locations", sqlLocations);
	return ret;
}

int mpkgDatabase::add_server_record(SERVER *server)
{
	//string sql_query;
	int server_id;
	//sql_query.clear();
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("server_id");
	SQLRecord sqlSearch;
	sqlSearch.addField("server_url", server->get_url());
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "servers", sqlSearch);
	//server_id=get_id("servers","server_id","server_url",server->get_url());
	if(sqlTable.empty())
	{
		sqlSearch.addField("server_priority", server->get_priority());
		
		db.sql_insert("servers", sqlSearch);
		server_id=get_last_id("servers","server_id");
		if (!server->get_tags()->IsEmpty() && server_id!=0) add_server_taglist_record(server_id, server->get_tags());
	}
	return server_id;
}

int mpkgDatabase::add_server_taglist_record (int server_id, SERVER_TAG_LIST *server_tag_list)
{
	int tag_id;
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("server_tag_id");
	SQLRecord sqlSearch;
	SQLRecord sqlInsert;
	for (int i=0; i<server_tag_list->size();i++)
	{
		sqlTable.clear();
		sqlSearch.clear();
		sqlSearch.addField("server_tag_name", server_tag_list->get_server_tag(i)->get_name());
		if (db.get_sql_vtable(&sqlTable, sqlFields, "server_tags", sqlSearch)!=0)
		{
			return -1;
		}
		if (sqlTable.empty())
		{
			sqlInsert.clear();
			sqlInsert.addField("server_tag_name", server_tag_list->get_server_tag(i)->get_name());
			if (db.sql_insert("server_tags", sqlInsert)!=0)
				return -2;
			if (db.get_sql_vtable(&sqlTable, sqlFields, "server_tags", sqlSearch)!=0)
				return -3;
		}
		tag_id=atoi(sqlTable.getValue(0, "server_tag_id").c_str());
		if (add_server_tag_link(server_id, tag_id)!=0)
			return -4;
	}
	return 0;
}

// Creates a link between package_id and tag_id
int mpkgDatabase::add_server_tag_link(int server_id, int tag_id)
{
	// Now we have a tag_id, and can write links between tags and package. Inserting data into server_tags_links table
	SQLRecord sqlInsert;
	sqlInsert.addField("servers_server_id", IntToStr(server_id));
	sqlInsert.addField("tags_tag_id",IntToStr(tag_id));
	return db.sql_insert("server_tag_links", sqlInsert);
}

//------------------------------------------------------------------------------------------	
// Adds dependency list linked to package_id
int mpkgDatabase::add_dependencylist_record(int package_id, DEPENDENCY_LIST *deplist)
{
	string dep_condition;
	string dep_type;
	SQLTable sqlTable;
	SQLRecord sqlInsert;
	for (int i=0;i<deplist->size();i++)
	{
		dep_condition=deplist->get_dependency(i)->get_condition();
		dep_type=deplist->get_dependency(i)->get_type();
		sqlInsert.clear();
		sqlInsert.addField("packages_package_id", IntToStr(package_id));
		sqlInsert.addField("dependency_condition", dep_condition);
		sqlInsert.addField("dependency_type", dep_type);
		sqlInsert.addField("dependency_package_name", deplist->get_dependency(i)->get_package_name());
		sqlInsert.addField("dependency_package_version", deplist->get_dependency(i)->get_package_version());
		sqlTable.addRecord(sqlInsert);
	}
	return db.sql_insert("dependencies", sqlTable);
}

// Adds tag list linked to package_id. It checks existance of tag in tags table, and creates if not. Next, it calls add_tag_link() to link package and tags
int mpkgDatabase::add_taglist_record (int package_id, TAG_LIST *taglist)
{
	int tag_id;
	SQLRecord sqlInsert;
	SQLTable sqlTable;
	SQLRecord sqlSearch;
	SQLRecord sqlFields;
	sqlFields.addField("tags_id");
	for (int i=0; i<taglist->size();i++)
	{
		sqlSearch.clear();
		sqlSearch.addField("tags_name", taglist->get_tag(i)->get_name());
		if (db.get_sql_vtable(&sqlTable, sqlFields, "tags", sqlSearch)!=0)
			return -1;


		// If tag doesn't exist in database - adding it. We make it by one query per tag, because tag count is wery small
		if (sqlTable.empty())
		{
			sqlInsert.clear();
			sqlInsert.addField("tags_name", taglist->get_tag(i)->get_name());
			if (db.sql_insert("tags", sqlInsert)!=0)
				return -2;
			if (db.get_sql_vtable(&sqlTable, sqlFields, "tags", sqlSearch)!=0)
				return -3;
			tag_id=atoi(sqlTable.getValue(0, "tags_id").c_str());
		}
		if (add_tag_link(package_id, tag_id)!=0)
			return -4;
	}
	return 0;
}

// Creates a link between package_id and tag_id
int mpkgDatabase::add_tag_link(int package_id, int tag_id)
{
	SQLRecord sqlInsert;
	sqlInsert.addField("packages_package_id", IntToStr(package_id));
	sqlInsert.addField("tags_tag_id", IntToStr(tag_id));
	return db.sql_insert("tags_links", sqlInsert);
}



// Adds package - full structure (including files, locations, deps, and tags), returning package_id
int mpkgDatabase::add_package_record (PACKAGE *package)
{
	int pkg_status;
	pkg_status=package->get_status();
	// INSERT INTO PACKAGES
	SQLRecord sqlInsert;
	
	
	sqlInsert.addField("package_name", package->get_name());
	sqlInsert.addField("package_version", package->get_version());
	sqlInsert.addField("package_arch", package->get_arch());
	sqlInsert.addField("package_build", package->get_build());
	sqlInsert.addField("package_compressed_size", package->get_compressed_size());
	sqlInsert.addField("package_installed_size", package->get_installed_size());
	sqlInsert.addField("package_short_description", package->get_short_description());
	sqlInsert.addField("package_description", package->get_description());
	sqlInsert.addField("package_changelog", package->get_changelog());
	sqlInsert.addField("package_packager", package->get_packager());
	sqlInsert.addField("package_packager_email", package->get_packager_email());
	sqlInsert.addField("package_status", IntToStr(package->get_status()));
	sqlInsert.addField("package_md5", package->get_md5());
	sqlInsert.addField("package_filename", package->get_filename());
	db.sql_insert("packages", sqlInsert);
	
	// Retrieving package ID
	int package_id=get_last_id("packages", "package_id");
	if (package_id==0) exit(-100);
	
	// INSERT INTO FILES
	if (!package->get_files()->IsEmpty()) add_filelist_record(package_id, package->get_files());
	
	// INSERT INTO LOCATIONS
	if (!package->get_locations()->IsEmpty()) add_locationlist_record(package_id, package->get_locations());

	//INSERT INTO DEPENDENCIES
	if (!package->get_dependencies()->IsEmpty()) add_dependencylist_record(package_id, package->get_dependencies());

	// INSERT INTO TAGS
	if (!package->get_tags()->IsEmpty()) add_taglist_record(package_id, package->get_tags());
	
	// INSERT INTO SCRIPTS
	add_scripts_record(package_id, package->get_scripts());
	return package_id;
}	


//--------------------Mid-level functions-------------------------

int mpkgDatabase::get_package(int package_id, PACKAGE *package, bool GetExtraInfo)
{
	//printf("Request to get package with ID=%d\n", package_id);
	SQLTable sqlTable;
	SQLRecord sqlFields;
	SQLRecord sqlSearch;

	sqlSearch.addField("package_id", IntToStr(package_id));
	
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "packages", sqlSearch);
	if (sql_ret!=0)
	{
		return sql_ret;
	}
	if (sqlTable.empty())
		return -1;
	if (sqlTable.getRecordCount()>1)
		return -2;

	package->set_id(package_id);
	package->set_name(sqlTable.getValue(0,"package_name"));
	package->set_version(sqlTable.getValue(0,"package_version"));
	package->set_arch(sqlTable.getValue(0,"package_arch"));
	package->set_build(sqlTable.getValue(0,"package_build"));
	package->set_compressed_size(sqlTable.getValue(0,"package_compressed_size"));
	package->set_installed_size(sqlTable.getValue(0,"package_installed_size"));
	package->set_short_description(sqlTable.getValue(0,"package_short_description"));
	package->set_description(sqlTable.getValue(0,"package_description"));
	package->set_changelog(sqlTable.getValue(0,"package_changelog"));
	package->set_packager(sqlTable.getValue(0,"package_packager"));
	package->set_packager_email(sqlTable.getValue(0,"package_packager_email"));
	package->set_status(atoi(sqlTable.getValue(0,"package_status").c_str()));
	package->set_md5(sqlTable.getValue(0,"package_md5"));
	package->set_filename(sqlTable.getValue(0,"package_filename"));
	if (GetExtraInfo)
	{
		//printf("[get_package] Getting extra info, package_id=%d\n",package->get_id());
		get_filelist(package_id, package->get_files());
		get_locationlist(package_id, package->get_locations());
		get_dependencylist(package_id, package->get_dependencies());
		get_taglist(package_id, package->get_tags());
		get_scripts(package_id, package->get_scripts());
	}
	return 0;
}




int mpkgDatabase::get_packagelist (SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	PACKAGE package;
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "packages", sqlSearch);
	if (sql_ret!=0)
	{
		return sql_ret;
	}

	for (int i=0; i<sqlTable.getRecordCount(); i++)
	{
		package.clear();
		//printf("ID[str]: %s, ID[int]: %d\n", sqlTable.getValue(i, "package_id").c_str(), atoi(sqlTable.getValue(i, "package_id").c_str()));
		package.set_id(atoi(sqlTable.getValue(i, "package_id").c_str()));
		package.set_name(sqlTable.getValue(i, "package_name"));
		package.set_version(sqlTable.getValue(i, "package_version"));
		package.set_arch(sqlTable.getValue(i, "package_arch"));
		package.set_build(sqlTable.getValue(i, "package_build"));
		package.set_compressed_size(sqlTable.getValue(i, "package_compressed_size"));
		package.set_installed_size(sqlTable.getValue(i, "package_installed_size"));
		package.set_short_description(sqlTable.getValue(i, "package_short_description"));
		package.set_description(sqlTable.getValue(i, "package_description"));
		package.set_changelog(sqlTable.getValue(i, "package_changelog"));
		package.set_packager(sqlTable.getValue(i, "package_packager"));
		package.set_packager_email(sqlTable.getValue(i, "package_packager_email"));
		package.set_status(atoi(sqlTable.getValue(i, "package_status").c_str()));
		package.set_md5(sqlTable.getValue(i, "package_md5"));
		package.set_filename(sqlTable.getValue(i, "package_filename"));
		if (GetExtraInfo)
		{
			//printf("Getting extra info, package_id=%d\n",package.get_id());
			get_filelist(package.get_id(), package.get_files());
			get_locationlist(package.get_id(), package.get_locations());
			get_dependencylist(package.get_id(), package.get_dependencies());
			get_taglist(package.get_id(), package.get_tags());
			get_scripts(package.get_id(), package.get_scripts());
		}
		packagelist->add(package);
	}
	return 0;

}

int mpkgDatabase::get_filelist(int package_id, FILE_LIST *filelist)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("file_name");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	FILES file;

	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "files", sqlSearch);
	if (sql_ret!=0) 
	{
		return 1;
	}

	for (int row=0;row<sqlTable.getRecordCount();row++)
	{
		file.set_name(sqlTable.getValue(row, "file_name"));
		filelist->add(file);
	}
	return 0;
}

int mpkgDatabase::get_dependencylist(int package_id, DEPENDENCY_LIST *deplist)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("dependency_condition");
	sqlFields.addField("dependency_type");
	sqlFields.addField("dependency_package_name");
	sqlFields.addField("dependency_package_version");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	DEPENDENCY dependency;

	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "dependencies", sqlSearch);
	if (sql_ret!=0)
	{
		return 1;
	}

	for (int row=0; row<sqlTable.getRecordCount(); row++)
	{
		dependency.set_condition(sqlTable.getValue(row, "dependency_condition"));
		dependency.set_type(sqlTable.getValue(row, "dependency_type"));
		dependency.set_package_name(sqlTable.getValue(row, "dependency_package_name"));
		dependency.set_package_version(sqlTable.getValue(row, "dependency_package_version"));
		deplist->add(dependency);
	}
	return 0;
}

int mpkgDatabase::get_taglist(int package_id, TAG_LIST *taglist)
{
	// Step 1. Read link table, and create a list of tag's ids.
	SQLTable id_sqlTable;
	SQLRecord id_sqlFields;
	id_sqlFields.addField("tags_tag_id");
	SQLRecord id_sqlSearch;
	id_sqlSearch.addField("packages_package_id", IntToStr(package_id));
	int id_sql_ret=db.get_sql_vtable(&id_sqlTable, id_sqlFields, "tags_links", id_sqlSearch);
	if (id_sql_ret!=0)
	{
		return 1;
	}

	TAG tag;
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("tags_name");
	SQLRecord sqlSearch;

	if (!id_sqlTable.empty())
	{
		sqlSearch.setSearchMode(SEARCH_OR);
		for (int i=0; i<=id_sqlTable.getRecordCount(); i++)
		{
			sqlSearch.addField("tags_id", sqlTable.getValue(i, "tags_tag_id"));
		}
	}
	
	// Step 2. Read the tags with readed ids
	
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "tags", sqlSearch);
	if (sql_ret!=0)
	{
		return 2;
	}

	if (!sqlTable.empty())
	{
		for (int i=0; i<sqlTable.getRecordCount(); i++)
		{
			tag.set_name(sqlTable.getValue(i, "tags_name"));
			taglist->add(tag);
		}
	}
	return 0;
}

int mpkgDatabase::get_server_tag_list(int server_id, SERVER_TAG_LIST *server_tag_list)
{
	// Step 1. Read link table, and create a list of server_tags id's.
	SQLTable id_sqlTable;
	SQLRecord id_sqlFields;
	id_sqlFields.addField("server_tags_server_tag_id");
	SQLRecord id_sqlSearch;
	id_sqlSearch.addField("servers_server_id", IntToStr(server_id));

	SERVER_TAG server_tag;
	if (db.get_sql_vtable(&id_sqlTable, id_sqlFields, "server_tags_links", id_sqlSearch)!=0)
	{
		return 1;
	}
	
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("server_tag_name");
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	
	if (!id_sqlTable.empty())
	{
		for (int i=0; i<id_sqlTable.getRecordCount(); i++)
		{
			sqlSearch.addField("server_tag_id", id_sqlTable.getValue(i, "server_tags_server_tag_id"));
		}
	}

	// Step 2. Read the tags with readed id's
	if (db.get_sql_vtable(&sqlTable, sqlFields, "server_tags", sqlSearch)!=0)
	{
		return 2;
	}

	if (!sqlTable.empty())
	{
		for (int i=0; i<sqlTable.getRecordCount(); i++)
		{
			server_tag.set_name(sqlTable.getValue(i, "server_tag_name"));
			server_tag_list->add(server_tag);
		}
	}
	return 0;
}


int mpkgDatabase::get_server(int server_id, SERVER *server)
{

	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("server_url");
	sqlFields.addField("server_priority");
	SQLRecord sqlSearch;
	sqlSearch.addField("server_id", IntToStr(server_id));
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "servers", sqlSearch);
	
	if (sql_ret!=0)
	{
		return 1;
	}
	
	if (sqlTable.getRecordCount()==1)
	{
		server->set_id(server_id);
		server->set_url(sqlTable.getValue(0, "server_url"));
		server->set_priority(sqlTable.getValue(0, "server_priority"));
		get_server_tag_list(server_id, server->get_tags());
	}
	else
	{
		server->set_id(0);
		return -1;
	}
	
	return 0;
}
	

int mpkgDatabase::get_locationlist(int package_id, LOCATION_LIST *location_list)
{
	LOCATION location;
	SERVER server;
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("location_id");
	sqlFields.addField("servers_server_id");
	sqlFields.addField("location_path");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "locations", sqlSearch);
	
	if (sql_ret!=0)
	{
		return 1;
	}

	for (int i=0; i<sqlTable.getRecordCount(); i++)
	{
		location.set_id(atoi(sqlTable.getValue(i, "location_id").c_str()));
		get_server(atoi(sqlTable.getValue(i, "servers_server_id").c_str()), &server);
		location.set_server(server);
		location.set_path(sqlTable.getValue(i, "location_path"));
		location_list->add(location);
	}
	//printf("Ended getting locations, got %d locations\n", location_list->size());
	return 0;
}

int mpkgDatabase::get_last_id(string table_name, string field)
{
	if (field.empty()) field=table_name+"_id";
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField(field);
	SQLRecord sqlSearch;
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, table_name, sqlSearch);
	if (sql_ret!=0)
	{
		return -1;
	}

	if(!sqlTable.empty())
	{
		//printf("get_last_id: %s\n", sqlTable.getValue(sqlTable.getRecordCount()-1, field).c_str());
		return atoi(sqlTable.getValue(sqlTable.getRecordCount()-1, field).c_str());
	}
	else
	{
		return 0;
	}
}

/*int mpkgDatabase::get_id(string table_name, string id_field, string search_field, string search_value, vector<string> *id_array)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	SQLRecord sqlSearch;
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
*/

int mpkgDatabase::get_package_id(PACKAGE *package)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_id");
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", package->get_name());
	sqlSearch.addField("package_version", package->get_version());
	sqlSearch.addField("package_arch", package->get_arch());
	sqlSearch.addField("package_build", package->get_build());
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "packages", sqlSearch);
	if (sql_ret!=0)
	{
		return -2;
	}

	if (sqlTable.empty())
	{
		return 0;
	}
	if (sqlTable.getRecordCount()==1)
	{
		//printf("get_package_id: %s\n", sqlTable.getValue(0, "package_id").c_str());
		return atoi(sqlTable.getValue(0, "package_id").c_str());
	}
	if (sqlTable.getRecordCount()>1)
	{
		//printf("get_package_id: multiple id found\n");
		return -1;
	}

	return -1;
}


int mpkgDatabase::set_status(int package_id, int status)
{
       	SQLRecord sqlUpdate;
	sqlUpdate.addField("package_status", IntToStr(status));
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));
	return db.sql_update("packages", sqlUpdate, sqlSearch);
}


int mpkgDatabase::get_status(int package_id)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_status");
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "packages", sqlSearch);
	if (sql_ret!=0)
	{
		return -2;
	}

	if (sqlTable.empty())
	{
		return -1;
	}
	if (sqlTable.getRecordCount()==1)
	{
		return atoi(sqlTable.getValue(0, "package_status").c_str());
	}
	if (sqlTable.getRecordCount()>1)
		return -3;
	
	return -100;
}


int mpkgDatabase::add_scripts_record(int package_id, SCRIPTS *scripts)
{
	SQLRecord sqlInsert;
	sqlInsert.addField("packages_package_id", IntToStr(package_id));
	sqlInsert.addField("preinstall", scripts->get_preinstall());
	sqlInsert.addField("postinstall", scripts->get_postinstall());
	sqlInsert.addField("preremove", scripts->get_preremove());
	sqlInsert.addField("postremove", scripts->get_postremove());
	return db.sql_insert("scripts", sqlInsert);
}

int mpkgDatabase::get_scripts(int package_id, SCRIPTS *scripts)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("script_id");
	sqlFields.addField("preinstall");
	sqlFields.addField("postinstall");
	sqlFields.addField("preremove");
	sqlFields.addField("postremove");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	if (db.get_sql_vtable(&sqlTable, sqlFields, "scripts", sqlSearch)!=0)
		return 1;

	if (sqlTable.getRecordCount()==1)
	{
		scripts->set_vid(sqlTable.getValue(0, "script_id"));
		scripts->set_preinstall(sqlTable.getValue(0, "preinstall"));
		scripts->set_postinstall(sqlTable.getValue(0, "postinstall"));
		scripts->set_preremove(sqlTable.getValue(0, "preremove"));
		scripts->set_postremove(sqlTable.getValue(0, "postremove"));
		return 0;
	}
	else
	{
		return -1;
	}
}

SQLProxy* mpkgDatabase::getSqlDb()
{
	return &db;
}
// class SQLRecord
vector<string> SQLRecord::getRecordValues()
{
	vector<string> output;
	for (int i=0;i<field.size();i++)
	{
		output.resize(i+1);
		output[i]=field[i].value;
	}
	return output;

}
int SQLRecord::size()
{
	return field.size();
}

bool SQLRecord::empty()
{
	if (field.empty())
	{
		return true;
	}
	else
	{
		return false;
	}
}
string SQLRecord::getFieldName(int num)
{
	if (num<field.size() && num >=0)
	{
		return field[num].fieldname;
	}
	else return "__OVERFLOW__";
}

string SQLRecord::getValue(string fieldname)
{
	for (int i=0;i<field.size();i++)
	{
		if (field[i].fieldname==fieldname) return field[i].value;
	}
	//printf("field %s not found\n", fieldname.c_str());
	return ""; // Means error
}

bool SQLRecord::setValue(string fieldname, string value)
{
	for (int i=0; i<field.size(); i++)
	{
		if (field[i].fieldname==fieldname)
		{
			field[i].value=value;
			return true;
		}
	}
	return false;
}

int SQLRecord::addField(string fieldname, string value)
{
	// Check if such field already exists
	for (int i=0; i<field.size(); i++)
	{
		if (field[i].fieldname==fieldname)
		{
			if (!value.empty()) field[i].value=value;
			return i;
		}
	}
	// Else, add field and store value
	int pos;
	pos=field.size();
	field.resize(pos+1);
	field[pos].fieldname=fieldname;
	field[pos].value=value;
	return pos;
}

string SQLRecord::getValueI(int num)
{
	if (num<field.size() && num>=0)
	{
		return field[num].value;
	}
	else return "__OVERFLOW__";
}



void SQLRecord::clear()
{
	field.clear();
}
void SQLRecord::setSearchMode(int mode)
{
	search_type=mode;
}

int SQLRecord::getSearchMode()
{
	return search_type;
}

SQLRecord::SQLRecord()
{
	search_type=SEARCH_AND;
}
SQLRecord::~SQLRecord(){}

// TODO: class SQLTable

int SQLTable::getRecordCount()
{
	return table.size();
}

bool SQLTable::empty()
{
	if (table.size()==0) return true;
	else return false;
}

void SQLTable::clear()
{
	table.clear();
}

string SQLTable::getValue(int num, string fieldname)
{
	if (num<table.size())
	{
		return table[num].getValue(fieldname);
	}
	else return "__OVERFLOW__";
}

SQLRecord SQLTable::getRecord(int num)
{
	if (num<table.size() && num>=0) return table[num];
}

void SQLTable::addRecord(SQLRecord record)
{
	int pos=table.size();
	table.resize(pos+1);
	table[pos]=record;
}


SQLTable::SQLTable(){}
SQLTable::~SQLTable(){}



