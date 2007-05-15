/********************************************************************************
 *
 * 			Central core for MOPSLinux package system
 *			TODO: Should be reorganized to objects
 *	$Id: core.cpp,v 1.43 2007/05/15 13:36:26 i27249 Exp $
 *
 ********************************************************************************/

#include "core.h"
#include "debug.h"
#include "mpkg.h"



//------------------Library front-end--------------------------------
#define ENABLE_CONFLICT_CHECK
bool mpkgDatabase::checkVersion(string version1, int condition, string version2)
{
	debug("checkVersion "+version1 + " vs " + version2);
	switch (condition)
	{
		case VER_MORE:
			if (strverscmp(version1.c_str(),version2.c_str())>0) return true;
			else return false;
			break;
		case VER_LESS:
			if (strverscmp(version1.c_str(),version2.c_str())<0) return true;
			else return false;
			break;
		case VER_EQUAL:
			if (strverscmp(version1.c_str(),version2.c_str())==0) return true;
			else return false;
			break;
		case VER_NOTEQUAL:
			if (strverscmp(version1.c_str(),version2.c_str())!=0) return true;
			else return false;
			break;
		case VER_XMORE:
			if (strverscmp(version1.c_str(),version2.c_str())>=0) return true;
			else return false;
			break;
		case VER_XLESS:
			if (strverscmp(version1.c_str(),version2.c_str())<=0) return true;
			else return false;
			break;
		default:
			printf("unknown condition %d!!!!!!!!!!!!!!!!!!!!!!!!!\n", condition);
			return true;
	}
	return true;
}

PACKAGE_LIST mpkgDatabase::get_other_versions(string package_name)
{
	PACKAGE_LIST pkgList;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", package_name);
	get_packagelist(sqlSearch, &pkgList, true);
	return pkgList;
}

PACKAGE mpkgDatabase::get_max_version(PACKAGE_LIST pkgList, DEPENDENCY *dep)
{
	PACKAGE candidate;
	string ver;
	for (int i=0; i<pkgList.size(); i++)
	{
		if (pkgList.get_package(i)->reachable() \
				&& strverscmp(pkgList.get_package(i)->get_version().c_str(), ver.c_str())>0 \
				&& mpkgDatabase::checkVersion(pkgList.get_package(i)->get_version(), \
					atoi(dep->get_condition().c_str()), \
					dep->get_package_version())
		   )
		{
			candidate = *pkgList.get_package(i);
			ver = pkgList.get_package(i)->get_version();
		}
	}
	return candidate;
}


int mpkgDatabase::check_file_conflicts(PACKAGE *package)
{
	int package_id;
	int prev_package_id=package->get_id();
	string fname;
	SQLTable sqlTable;
	SQLRecord sqlFields;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_IN);
	if (package->get_files()->size()==0) return 0; // If a package has no files, it cannot conflict =)
	for (int i=0;i<package->get_files()->size();i++)
	{

		sqlFields.addField("packages_package_id");
		sqlFields.addField("file_name");
		fname=package->get_files()->get_file(i)->get_name();
		if (fname[fname.length()-1]!='/') 
		{

			sqlSearch.addField("file_name", package->get_files()->get_file(i)->get_name());
		}
		else
		{

		}
	}
	db.get_sql_vtable(&sqlTable, sqlFields, "files", sqlSearch);
	if (!sqlTable.empty())
	{
		for (int k=0;k<sqlTable.getRecordCount() ;k++) // Excluding from check packages who are already installed
		{
			package_id=atoi(sqlTable.getValue(k, "packages_package_id").c_str());

			if (package_id!=prev_package_id)
			{

				if (get_installed(package_id) || get_action(package_id)==ST_INSTALL)
				{
					printf("File %s conflicts with package ID %d, backing up\n", sqlTable.getValue(k, "file_name").c_str(), package_id);
					return backupFile(sqlTable.getValue(k, "file_name"), package_id, package->get_id());
				}
			}
		}
	}
	return 0; // End of check_file_conflicts
}

int mpkgDatabase::add_conflict_record(int conflicted_id, int overwritten_id, string file_name)
{
	PACKAGE pkg;
	get_package(overwritten_id, &pkg);

	SQLRecord sqlFill;
	sqlFill.addField("conflicted_package_id", IntToStr(conflicted_id));
	sqlFill.addField("conflicted_file_name", file_name);
	sqlFill.addField("backup_file", pkg.get_name() + "_" + pkg.get_md5() + "/" + file_name);
	return db.sql_insert("conflicts", sqlFill);
}

int mpkgDatabase::delete_conflict_record(int conflicted_id, string file_name)
{
	SQLRecord sqlFill;
	sqlFill.addField("conflicted_package_id", IntToStr(conflicted_id));
	sqlFill.addField("backup_file", file_name);
	return db.sql_delete("conflicts", sqlFill);
}

FILE_LIST mpkgDatabase::get_conflict_records(int conflicted_id)
{
	FILE_LIST ret;
	SQLRecord sqlSearch;
	sqlSearch.addField("conflicted_package_id", IntToStr(conflicted_id));
	SQLRecord sqlFields;
	sqlFields.addField("backup_file");
	sqlFields.addField("conflict_file_name");
	SQLTable fTable;
	db.get_sql_vtable(&fTable, sqlFields, "conflicts", sqlSearch);
	FILES tmp;
	for (int i=0; i<fTable.getRecordCount(); i++)
	{
		tmp.set_name(fTable.getValue(i, "conflict_file_name"));
		
		tmp.backup_file = SYS_BACKUP + fTable.getValue(i, "backup_file");
		ret.add(tmp);
	}
	return ret;
}

int mpkgDatabase::backupFile(string filename, int overwritten_package_id, int conflicted_package_id)
{
	if (FileExists(SYS_ROOT+filename))
	{
		PACKAGE pkg;
		get_package(overwritten_package_id, &pkg);
		string bkpDir = SYS_BACKUP + pkg.get_name() + "_" + pkg.get_md5();
		string bkpDir2 = bkpDir + "/" + filename.substr(0, filename.find_last_of("/"));
		string mkd = "mkdir -p " + bkpDir2;
		
		string mv = "mv ";
	        mv += SYS_ROOT + filename + " " + bkpDir2 + "/";
		if (system(mkd.c_str())!=0 ||  system(mv.c_str())!=0)
		{
			return MPKGERROR_FILEOPERATIONS;
		}
		add_conflict_record(conflicted_package_id, overwritten_package_id, filename);
	}
	else
	{
		string t = SYS_ROOT+filename;
	}
	return 0;
}

int _cleanBackupCallback(const char *filename, const struct stat *file_status, int filetype)
{
	if (filetype == FTW_D && strcmp(filename, SYS_BACKUP)!=0 ) rmdir(filename);
	return 0;
}

void mpkgDatabase::clean_backup_directory()
{
	ftw(SYS_BACKUP, _cleanBackupCallback, 20);
}

int mpkgDatabase::clean_package_filelist (PACKAGE *package)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package->get_id()));
	return db.sql_delete("files", sqlSearch);
}

int mpkgDatabase::add_descriptionlist_record(int package_id, DESCRIPTION_LIST *desclist)
{
	SQLTable sqlTable;
	SQLRecord sqlValues;
	for (unsigned int i=0; i<desclist->size(); i++)
	{
		sqlValues.clear();
		sqlValues.addField("description_language", desclist->get_description(i)->get_language());
		sqlValues.addField("description_short_text", desclist->get_description(i)->get_shorttext());
		sqlValues.addField("description_text", desclist->get_description(i)->get_text());
		sqlValues.addField("packages_package_id", IntToStr(package_id));
		sqlTable.addRecord(sqlValues);
	}
	if (!sqlTable.empty())
	{
		return db.sql_insert("descriptions", sqlTable);
	}
	else
	{
		 return 0;
	}

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
		sqlValues.addField("file_type", IntToStr(filelist->get_file(i)->get_type()));
		sqlTable.addRecord(sqlValues);
	}
	if (!sqlTable.empty())
	{
		return db.sql_insert("files", sqlTable);
	}
	else return 0;
}

// Adds location list linked to package_id
int mpkgDatabase::add_locationlist_record(int package_id, LOCATION_LIST *locationlist) // returns 0 if ok, anything else if failed.
{
	debug("core.cpp: add_locationlist_record(): begin");
	int serv_id;
	SQLTable sqlLocations;
	SQLRecord sqlLocation;
	for (int i=0;i<locationlist->size();i++)
	{
		debug("core.cpp: add_locationlist_record(): adding location "+IntToStr(i));
		if (!locationlist->get_location(i)->get_server()->IsEmpty())
		{
			debug("core.cpp: add_locationlist_record(): adding server");
			serv_id=add_server_record(locationlist->get_location(i)->get_server());
			if (serv_id<=0)
			{
				debug("core.cpp: add_locationlist_record(): add_server_record() error, returned "+IntToStr(serv_id));
				return -1;
			}
			debug("core.cpp: add_locationlist_record(): serv_id="+IntToStr(serv_id)+", adding location to sqlLocations");
			sqlLocation.clear();
			sqlLocation.addField("servers_server_id", IntToStr(serv_id));
			sqlLocation.addField("packages_package_id", IntToStr(package_id));
			sqlLocation.addField("location_path", locationlist->get_location(i)->get_path());
			sqlLocations.addRecord(sqlLocation);
		}
		else debug("core.cpp: add_locationlist_record(): location "+IntToStr(i)+" incomplete (has no server), cannot add this");
	}
	int ret=1;
	if (sqlLocations.empty()) debug ("core.cpp: add_locationlist_record(): No valid locations found for package (Package_ID="+IntToStr(package_id)+")");
	else ret=db.sql_insert("locations", sqlLocations);
	if (ret!=0) debug("core.cpp: add_locationlist_record(): db.sql_insert(locations) failed, code="+IntToStr(ret));
	else debug("core.cpp: add_locationlist_record(): successful end");
	return ret;
}

int mpkgDatabase::add_server_record(SERVER *server)	// Returns server id if success, or -1 if failed.
{
	int server_id;
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("server_id");
	SQLRecord sqlSearch;
	sqlSearch.addField("server_url", server->get_url());
	if (db.get_sql_vtable(&sqlTable, sqlFields, "servers", sqlSearch)!=0)
		return -1;

	if(sqlTable.empty())
	{
		sqlSearch.addField("server_priority", server->get_priority());
		
		db.sql_insert("servers", sqlSearch);
		server_id=get_last_id("servers","server_id");
		if (!server->get_tags()->IsEmpty() && server_id!=0) add_server_taglist_record(server_id, server->get_tags());
	}
	else
	{
		server_id=atoi(sqlTable.getValue(0,"server_id").c_str());
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
	int tag_id=0;
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
		}
		tag_id=atoi(sqlTable.getValue(0, "tags_id").c_str());
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
	int ret = db.sql_insert("tags_links", sqlInsert);
	return ret;
}



// Adds package - full structure (including files, locations, deps, and tags), returning package_id
int mpkgDatabase::add_package_record (PACKAGE *package)
{
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
	sqlInsert.addField("package_available", IntToStr(package->available()));
	sqlInsert.addField("package_installed", IntToStr(package->installed()));
	sqlInsert.addField("package_configexist", IntToStr(package->configexist()));
	sqlInsert.addField("package_action", IntToStr(package->action()));

	sqlInsert.addField("package_md5", package->get_md5());
	sqlInsert.addField("package_filename", package->get_filename());

	db.sql_insert("packages", sqlInsert);
	
	// Retrieving package ID
	int package_id=get_last_id("packages", "package_id");
	package->set_id(package_id);
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

	// INSERT INTO DESCRIPTIONS
	add_descriptionlist_record(package_id, package->get_descriptions());

	return package_id;
}	



//--------------------Mid-level functions-------------------------

int mpkgDatabase::get_package(int package_id, PACKAGE *package, bool GetExtraInfo)
{
	if (package_id<0)
	{
		printf("Requested to find a package without ID\n");
		return MPKGERROR_INCORRECTDATA;
	}
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
		return MPKGERROR_NOPACKAGE;
	if (sqlTable.getRecordCount()>1)
		return MPKGERROR_AMBIGUITY;

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
	//package->set_available(atoi(sqlTable.getValue(0,"package_available").c_str()));
	package->set_installed(atoi(sqlTable.getValue(0,"package_installed").c_str()));
	package->set_configexist(atoi(sqlTable.getValue(0,"package_configexist").c_str()));
	package->set_action(atoi(sqlTable.getValue(0,"package_action").c_str()));
	package->set_md5(sqlTable.getValue(0,"package_md5"));
	package->set_filename(sqlTable.getValue(0,"package_filename"));
	if (GetExtraInfo)
	{
		get_filelist(package_id, package->get_files());
		get_scripts(package_id, package->get_scripts());
	}
	get_locationlist(package_id, package->get_locations());
	get_dependencylist(package_id, package->get_dependencies());
	get_taglist(package_id, package->get_tags());
	get_descriptionlist(package_id, package->get_descriptions());
	return 0;
}

int mpkgDatabase::get_packagelist (SQLRecord sqlSearch, PACKAGE_LIST *packagelist, bool GetExtraInfo, bool ultraFast)
{
	debug("get_packagelist start");
	SQLTable sqlTable;
	SQLRecord sqlFields;
	PACKAGE package;
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "packages", sqlSearch);
	if (sql_ret!=0)
	{
		return MPKGERROR_SQLQUERYERROR;
	}
	if (!consoleMode)
	{
	actionBus.setCurrentAction(ACTIONID_GETPKGLIST);
	actionBus.setActionProgressMaximum(ACTIONID_DBLOADING, sqlTable.getRecordCount());

	actionBus.setActionProgressMaximum(ACTIONID_GETPKGLIST, sqlTable.getRecordCount());
	actionBus.setActionProgress(ACTIONID_GETPKGLIST, 0);
	}
	packagelist->clear(sqlTable.getRecordCount());
	for (int i=0; i<sqlTable.getRecordCount(); i++)
	{
		if (!consoleMode) actionBus.setActionProgress(ACTIONID_GETPKGLIST, i);
		debug("retrieving package "+IntToStr(i));
		package.clear();
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
		
		//package.set_available(atoi(sqlTable.getValue(i,"package_available").c_str()));
		package.set_installed(atoi(sqlTable.getValue(i,"package_installed").c_str()));
		package.set_configexist(atoi(sqlTable.getValue(i,"package_configexist").c_str()));
		package.set_action(atoi(sqlTable.getValue(i,"package_action").c_str()));

		package.set_md5(sqlTable.getValue(i, "package_md5"));
		package.set_filename(sqlTable.getValue(i, "package_filename"));
		if (GetExtraInfo)
		{
			get_filelist(package.get_id(), package.get_files());
			get_scripts(package.get_id(), package.get_scripts());
		}
		package.sync();
		get_locationlist(package.get_id(), package.get_locations());
		if (!ultraFast)
		{
			get_dependencylist(package.get_id(), package.get_dependencies());
			get_taglist(package.get_id(), package.get_tags());
		}
#ifdef ENABLE_INTERNATIONAL
		
		debug("description list...");
		get_descriptionlist(package.get_id(), package.get_descriptions());
#endif
		
		debug("setting package...");
		packagelist->set_package(i, package);
		debug("done.");
	}
	debug("get_packagelist end");
	return 0;

}

int mpkgDatabase::get_descriptionlist(int package_id, DESCRIPTION_LIST *desclist, string language)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("description_language");
	sqlFields.addField("short_description_text");
	sqlFields.addField("description_text");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	if (!language.empty()) sqlSearch.addField("description_language", language);
	
	DESCRIPTION desc;
	
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "descriptions", sqlSearch);
	if (sql_ret!=0)
	{
		return -1;
	}
	
	for (int row=0; row<sqlTable.getRecordCount(); row++)
	{
		desc.set_language(sqlTable.getValue(row, "description_language"));
		desc.set_text(sqlTable.getValue(row, "description_text"));
		desc.set_shorttext(sqlTable.getValue(row, "short_description_text"));
		desclist->add(desc);
		desc.clear();
	}
	return 0;
}

int mpkgDatabase::get_filelist(int package_id, FILE_LIST *filelist, bool config_only)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("file_name");
	sqlFields.addField("file_type");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	FILES file;

	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "files", sqlSearch);
	if (sql_ret!=0) 
	{
		return -1;
	}

	for (int row=0;row<sqlTable.getRecordCount();row++)
	{
		if (!config_only)
		{
			file.set_name(sqlTable.getValue(row, "file_name"));
			file.set_type(atoi(sqlTable.getValue(row, "file_type").c_str()));
			filelist->add(file);
		}
		else
		{
			if (sqlTable.getValue(row, "file_type")==IntToStr(FTYPE_CONFIG))
			{
				file.set_name(sqlTable.getValue(row, "file_name"));
				file.set_type(atoi(sqlTable.getValue(row, "file_type").c_str()));
				filelist->add(file);
			}
		}
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
		for (int i=0; i<id_sqlTable.getRecordCount(); i++)
		{
			sqlSearch.addField("tags_id", id_sqlTable.getValue(i, "tags_tag_id"));
		}
	}
	else
	{
		return 0;
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
		return atoi(sqlTable.getValue(sqlTable.getRecordCount()-1, field).c_str());
	}
	else
	{
		return 0;
	}
}


int mpkgDatabase::get_package_id(PACKAGE *package)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_id");
	SQLRecord sqlSearch;
#ifndef NO_MD5_COMPARE
	sqlSearch.addField("package_md5", package->get_md5());
#endif
#ifdef NO_MD5_COMPARE
	sqlSearch.addField("package_name", package->get_name());
	sqlSearch.addField("package_version", package->get_version());
	sqlSearch.addField("package_arch", package->get_arch());
	sqlSearch.addField("package_build", package->get_build());
#endif
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
		return atoi(sqlTable.getValue(0, "package_id").c_str());
	}
	if (sqlTable.getRecordCount()>1)
	{
		return -1;
	}

	return -1;
}

int mpkgDatabase::set_installed(int package_id, int status)
{
       	SQLRecord sqlUpdate;
	sqlUpdate.addField("package_installed", IntToStr(status));
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));
	return db.sql_update("packages", sqlUpdate, sqlSearch);
}
int mpkgDatabase::set_configexist(int package_id, int status)
{
       	SQLRecord sqlUpdate;
	sqlUpdate.addField("package_configexist", IntToStr(status));
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));
	return db.sql_update("packages", sqlUpdate, sqlSearch);
}
int mpkgDatabase::set_action(int package_id, int status)
{
       	SQLRecord sqlUpdate;
	sqlUpdate.addField("package_action", IntToStr(status));
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));
	return db.sql_update("packages", sqlUpdate, sqlSearch);
}


int mpkgDatabase::get_installed(int package_id)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_installed");
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
		return atoi(sqlTable.getValue(0, "package_installed").c_str());
	}
	if (sqlTable.getRecordCount()>1)
		return -3;
	
	return -100;
}

int mpkgDatabase::get_action(int package_id)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_action");
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
		return atoi(sqlTable.getValue(0, "package_installed").c_str());
	}
	if (sqlTable.getRecordCount()>1)
		return -3;
	
	return -100;
}

int mpkgDatabase::get_configexist(int package_id)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_configexist");
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
		return atoi(sqlTable.getValue(0, "package_configexist").c_str());
	}
	if (sqlTable.getRecordCount()>1)
		return -3;
	
	return -100;
}

int mpkgDatabase::get_available(int package_id)
{
	SQLTable sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("packages_package_id");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	int sql_ret=db.get_sql_vtable(&sqlTable, sqlFields, "locations", sqlSearch);
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
		//return atoi(sqlTable.getValue(0, "package_available").c_str());
	}
	if (sqlTable.getRecordCount()>1)
		return 1;
	
	return -100;
}


int mpkgDatabase::add_scripts_record(int package_id, SCRIPTS *scripts)
{
	// Check if data is already in DB
	SQLTable sqlTable;
	SQLRecord sqlSearch;
	SQLRecord sqlFields;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	db.get_sql_vtable(&sqlTable, sqlFields, "scripts", sqlSearch);
			
	SQLRecord sqlInsert;
	sqlInsert.addField("packages_package_id", IntToStr(package_id));
	sqlInsert.addField("preinstall", scripts->get_preinstall());
	sqlInsert.addField("postinstall", scripts->get_postinstall());
	sqlInsert.addField("preremove", scripts->get_preremove());
	sqlInsert.addField("postremove", scripts->get_postremove());

	if (sqlTable.empty()) return db.sql_insert("scripts", sqlInsert);
	else return db.sql_update("scripts", sqlInsert, sqlSearch);
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

int mpkgDatabase::get_purge(string package_name)
{
	debug("get_purge start");
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", package_name);
	SQLTable sqlTable;
	SQLTable sqlTableFiles;
	SQLRecord sqlFields;
	sqlFields.addField("package_id");
	sqlFields.addField("package_configexist");
	sqlFields.addField("package_installed");
	sqlFields.addField("package_action");
	if (db.get_sql_vtable(&sqlTable, sqlFields, "packages", sqlSearch)!=0)
	{
		return -1;
	}
	if (sqlTable.empty())
	{
		debug("Nothing found");
		return 0;
	}
	int id=0;
	for (int i=0; i<sqlTable.getRecordCount(); i++)
	{
		debug("searching..."); // HERE IS AN ERROR: IF IT WAS SAME PACKAGE, THEN AT THIS MOMENT, PACKAGE HAS ALREADY STATUS "INSTALL". NEED FIX
		if (sqlTable.getValue(i, "package_configexist")==IntToStr(ST_CONFIGEXIST))
		{
			id=atoi(sqlTable.getValue(i, "package_id").c_str());
			debug("id set to "+IntToStr(id));
			break;
		}
		/*if (sqlTable.getValue(i, "package_action")==IntToStr(ST_INSTALL))	//TEMP DISABLE - I don't remember why this code needed.
		{
			sqlSearch.clear();
			sqlFields.clear();

			sqlSearch.addField("packages_package_id", sqlTable.getValue(i, "package_id"));
			sqlSearch.addField("file_type", IntToStr(FTYPE_CONFIG));
			db.get_sql_vtable(&sqlTableFiles, sqlFields, "files", sqlSearch);
			if (!sqlTableFiles.empty())
			{
				id=atoi(sqlTable.getValue(i, "package_id").c_str());
				break;
			}
		}*/
	}
	return id;
}
//--------------------------------------SQL PART----------------------------------------
SQLProxy* mpkgDatabase::getSqlDb()
{
	return &db;
}
// class SQLRecord
vector<string> SQLRecord::getRecordValues()
{
	vector<string> output;
	for (unsigned int i=0;i<field.size();i++)
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
string SQLRecord::getFieldName(unsigned int num)
{
	if (num<field.size() && num >=0)
	{
		return PrepareSql(field[num].fieldname);
	}
	else return "__OVERFLOW__";
}

string SQLRecord::getValue(string fieldname)
{
	for (unsigned int i=0;i<field.size();i++)
	{
		if (field[i].fieldname==fieldname) return PrepareSql(field[i].value);
	}
	return ""; // Means error
}

bool SQLRecord::setValue(string fieldname, string value)
{
	for (unsigned int i=0; i<field.size(); i++)
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
	int pos;
	SQLField tmp;
	tmp.fieldname=fieldname;
	tmp.value=value;
	field.push_back(tmp);
	pos=field.size();
	return pos;
}

string SQLRecord::getValueI(unsigned int num)
{
	if (num<field.size() && num>=0)
	{
		return PrepareSql(field[num].value);
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

void SQLRecord::setEqMode(int mode)
{
	eq_type=mode;
}

int SQLRecord::getEqMode()
{
	return eq_type;
}

SQLRecord::SQLRecord()
{
	search_type=SEARCH_AND;
	eq_type=EQ_EQUAL;
}
SQLRecord::~SQLRecord(){}


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

string SQLTable::getValue(unsigned int num, string fieldname)
{
	if (num<table.size())
	{
		return table[num].getValue(fieldname);
	}
	else return "__OVERFLOW__";
}

SQLRecord SQLTable::getRecord(unsigned int num)
{
	if (num<table.size() && num>=0) return table[num];
	else
	{
		// returning empty...
		debug("core.cpp: SQLTable::getRecord(): incorrect num");
		SQLRecord ret;
		return ret;
	}
}

void SQLTable::addRecord(SQLRecord record)
{
	int pos=table.size();
	table.resize(pos+1);
	table[pos]=record;
}


SQLTable::SQLTable(){}
SQLTable::~SQLTable(){}



