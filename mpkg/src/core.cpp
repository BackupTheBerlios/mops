/********************************************************************************
 *
 * 			Central core for MOPSLinux package system
 *			TODO: Should be reorganized to objects
 *	$Id: core.cpp,v 1.67 2007/08/14 14:29:54 i27249 Exp $
 *
 ********************************************************************************/

#include "core.h"
#include "debug.h"
#include "mpkg.h"



//------------------Library front-end--------------------------------
#define ENABLE_CONFLICT_CHECK
bool mpkgDatabase::checkVersion(string *version1, int condition, string *version2)
{
	//debug("checkVersion "+version1 + " vs " + version2);
	switch (condition)
	{
		case VER_MORE:
			if (strverscmp(version1->c_str(),version2->c_str())>0) return true;
			else return false;
			break;
		case VER_LESS:
			if (strverscmp(version1->c_str(),version2->c_str())<0) return true;
			else return false;
			break;
		case VER_EQUAL:
			if (strverscmp(version1->c_str(),version2->c_str())==0) return true;
			else return false;
			break;
		case VER_NOTEQUAL:
			if (strverscmp(version1->c_str(),version2->c_str())!=0) return true;
			else return false;
			break;
		case VER_XMORE:
			if (strverscmp(version1->c_str(),version2->c_str())>=0) return true;
			else return false;
			break;
		case VER_XLESS:
			if (strverscmp(version1->c_str(),version2->c_str())<=0) return true;
			else return false;
			break;
		default:
			mError("unknown condition " + IntToStr(condition));
			return true;
	}
	return true;
}

PACKAGE_LIST mpkgDatabase::get_other_versions(string *package_name)
{
	PACKAGE_LIST pkgList;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", package_name);
	get_packagelist(&sqlSearch, &pkgList);
	return pkgList;
}	// Seems to be deprecated

PACKAGE* mpkgDatabase::get_max_version(PACKAGE_LIST *pkgList, DEPENDENCY *dep)
{
	// Maybe optimization possible
	PACKAGE *candidate=NULL;
	string ver;
	for (int i=0; i<pkgList->size(); i++)
	{
		if (pkgList->get_package(i)->reachable() \
				&& strverscmp(pkgList->get_package(i)->get_version()->c_str(), ver.c_str())>0 \
				&& mpkgDatabase::checkVersion(pkgList->get_package(i)->get_version(), \
					atoi(dep->get_condition()->c_str()), \
					dep->get_package_version())
		   )
		{
			candidate = pkgList->get_package(i);
			ver = *pkgList->get_package(i)->get_version();
		}
	}
	if (candidate==NULL)
	{
		candidate = new PACKAGE;
		mError("max version not detected");
	}
	return candidate;
}


int mpkgDatabase::check_file_conflicts(PACKAGE *package)
{
	//printf("checking conflicts\n");
	mDebug("start");
	int package_id;
	int prev_package_id=package->get_id();
	string fname;
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_IN);
	sqlFields.addField("packages_package_id");
	sqlFields.addField("file_name");
	
	if (package->get_files()->size()==0)
	{
		return 0; // If a package has no files, it cannot conflict =)
	}
	for (unsigned int i=0;i<package->get_files()->size();i++)
	{

		fname=*package->get_files()->at(i).get_name();
		if (fname[fname.length()-1]!='/') 
		{
			sqlSearch.addField("file_name", package->get_files()->at(i).get_name());
		}
	}
	db.get_sql_vtable(sqlTable, sqlFields, "files", sqlSearch);
	if (!sqlTable->empty())
	{
		for (int k=0;k<sqlTable->getRecordCount() ;k++) // Excluding from check packages who are already installed
		{
			package_id=atoi(sqlTable->getValue(k, "packages_package_id")->c_str());
			if (package_id!=prev_package_id)
			{
				if (get_installed(package_id) || get_action(package_id)==ST_INSTALL)
				{
					say("File %s conflicts with package ID %d, backing up\n", sqlTable->getValue(k, "file_name")->c_str(), package_id);
					backupFile(sqlTable->getValue(k, "file_name"), package_id, package->get_id());
				}
			}
		}
	}
	delete sqlTable;
	return 0; // End of check_file_conflicts
}

int mpkgDatabase::add_conflict_record(int conflicted_id, int overwritten_id, string *file_name)
{
//	printf("adding conflict record: conflicted_id = %d, overwritten_id = %d, file = %s\n", conflicted_id, overwritten_id, file_name->c_str());
	PACKAGE pkg;
	get_package(overwritten_id, &pkg);

	SQLRecord sqlFill;
	sqlFill.addField("conflicted_package_id", conflicted_id);
	sqlFill.addField("conflict_file_name", file_name);
	string x = *pkg.get_name() + "_" + *pkg.get_md5() + "/" + *file_name;
	sqlFill.addField("backup_file",&x );
	return db.sql_insert("conflicts", sqlFill);
}

int mpkgDatabase::delete_conflict_record(int conflicted_id, string *file_name)
{
	SQLRecord sqlFill;
	sqlFill.addField("conflicted_package_id", conflicted_id);
	sqlFill.addField("backup_file", file_name);
	return db.sql_delete("conflicts", sqlFill);
}

void mpkgDatabase::get_conflict_records(int conflicted_id, vector<FILES> *ret)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("conflicted_package_id", conflicted_id);
	SQLRecord sqlFields;
	sqlFields.addField("backup_file");
	sqlFields.addField("conflict_file_name");
	SQLTable fTable;
	db.get_sql_vtable(&fTable, sqlFields, "conflicts", sqlSearch);
	ret->clear();
	ret->resize(fTable.getRecordCount());
	for (int i=0; i<fTable.getRecordCount(); i++)
	{
		ret->at(i).set_name(fTable.getValue(i, "conflict_file_name"));
		ret->at(i).set_backup_file(fTable.getValue(i, "backup_file"));
	}
}

int mpkgDatabase::backupFile(string *filename, int overwritten_package_id, int conflicted_package_id)
{
//	printf("backing up %s\n", filename->c_str());
	if (FileExists(SYS_ROOT + *filename))
	{
//		printf("backupFile: file exists\n");
		PACKAGE pkg;
		get_package(overwritten_package_id, &pkg);
		string bkpDir = SYS_BACKUP + *pkg.get_name() + "_" + *pkg.get_md5();
		string bkpDir2 = bkpDir + "/" + filename->substr(0, filename->find_last_of("/"));
		string mkd = "mkdir -p " + bkpDir2;
		
		string mv = "mv ";
	        mv += SYS_ROOT + *filename + " " + bkpDir2 + "/";
		if (!simulate)
		{
			if (system(mkd.c_str())!=0 ||  system(mv.c_str())!=0)
			{
				mError("Error while backup");
				return MPKGERROR_FILEOPERATIONS;
			}
		}
//		printf("adding record\n");
		add_conflict_record(conflicted_package_id, overwritten_package_id, filename);
		// Adding some logging facility
		FILE *log = fopen("/var/log/mpkg-backups.log","a");
		if (log)
		{
			string target_name = *pkg.get_name() + "-" + pkg.get_fullversion();
			get_package(conflicted_package_id, &pkg);
			string overwriter_name = *pkg.get_name() + "-" + pkg.get_fullversion();
			fprintf(log, "FILE: [%s] TARGET: [%s] OVERWRITER: [%s]\n", filename->c_str(), target_name.c_str(), overwriter_name.c_str());
			fclose(log);
		}
		else
		{
			mError("Unable to open log file /var/log/mpkg-backups.log");
		}
	}
	else
	{
		//printf("backupFile: file %s%s doesn't exist\n", SYS_ROOT.c_str(), filename->c_str());
	}
	return 0;
}

int _cleanBackupCallback(const char *filename, const struct stat *file_status, int filetype)
{
	unsigned short x=0, y=0;

	if (file_status->st_ino!=0) x=y;
	if (filetype == FTW_D && strcmp(filename, SYS_BACKUP)!=0 && !simulate ) rmdir(filename);
	return 0;
}

void mpkgDatabase::clean_backup_directory()
{
	ftw(SYS_BACKUP, _cleanBackupCallback, 20);
}

int mpkgDatabase::clean_package_filelist (PACKAGE *package)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", package->get_id());
	return db.sql_delete("files", sqlSearch);
}
#ifdef ENABLE_INTERNATIONAL
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
#endif
// Adds file list linked to package_id (usually for package adding)
int mpkgDatabase::add_filelist_record(int package_id, vector<FILES> *filelist)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlValues;
	for (unsigned int i=0;i<filelist->size();i++)
	{
		sqlValues.clear();
		sqlValues.addField("file_name", filelist->at(i).get_name());
		sqlValues.addField("packages_package_id", package_id);
		sqlValues.addField("file_type", filelist->at(i).get_type());
		sqlTable->addRecord(&sqlValues);
	}
	if (!sqlTable->empty())
	{
		int ret = db.sql_insert("files", *sqlTable);
		delete sqlTable;
		return ret;
	}
	else
	{
		delete sqlTable;
		return 0;
	}
}

// Adds location list linked to package_id
int mpkgDatabase::add_locationlist_record(int package_id, vector<LOCATION> *locationlist) // returns 0 if ok, anything else if failed.
{
	SQLTable *sqlLocations = new SQLTable;
	SQLRecord sqlLocation;
	for (unsigned int i=0;i<locationlist->size();i++)
	{
		sqlLocation.clear();
		sqlLocation.addField("server_url", locationlist->at(i).get_server_url());
		sqlLocation.addField("packages_package_id", package_id);
		sqlLocation.addField("location_path", locationlist->at(i).get_path());
		sqlLocations->addRecord(&sqlLocation);
		
	}
	int ret=1;
	if (!sqlLocations->empty()) ret=db.sql_insert("locations", *sqlLocations);
	delete sqlLocations;
	return ret;
}

// Adds dependency list linked to package_id
int mpkgDatabase::add_dependencylist_record(int package_id, vector<DEPENDENCY> *deplist)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlInsert;
	for (unsigned int i=0;i<deplist->size();i++)
	{
		sqlInsert.clear();
		sqlInsert.addField("packages_package_id", package_id);
		sqlInsert.addField("dependency_condition",deplist->at(i).get_condition() );
		sqlInsert.addField("dependency_type", deplist->at(i).get_type());
		sqlInsert.addField("dependency_package_name", deplist->at(i).get_package_name());
		sqlInsert.addField("dependency_package_version", deplist->at(i).get_package_version());
		sqlTable->addRecord(&sqlInsert);
	}
	int ret = db.sql_insert("dependencies", *sqlTable);
	delete sqlTable;
	return ret;
}

// Adds tag list linked to package_id. It checks existance of tag in tags table, and creates if not. Next, it calls add_tag_link() to link package and tags
int mpkgDatabase::add_taglist_record (int package_id, vector<string> *taglist)
{
	int tag_id=0;
	SQLRecord sqlInsert;
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlSearch;
	SQLRecord sqlFields;
	sqlFields.addField("tags_id");
	for (unsigned int i=0; i<taglist->size();i++)
	{
		sqlSearch.clear();
		sqlSearch.addField("tags_name", &taglist->at(i)); 
		db.get_sql_vtable(sqlTable, sqlFields, "tags", sqlSearch);

		// If tag doesn't exist in database - adding it. We make it by one query per tag, because tag count is wery small
		if (sqlTable->empty())
		{
			sqlInsert.clear();
			sqlInsert.addField("tags_name", &taglist->at(i));
			db.sql_insert("tags", sqlInsert);
			db.get_sql_vtable(sqlTable, sqlFields, "tags", sqlSearch);
		}
		tag_id=atoi(sqlTable->getValue(0, "tags_id")->c_str());
		add_tag_link(package_id, tag_id);
	}
	delete sqlTable;
	return 0;
}

// Creates a link between package_id and tag_id
int mpkgDatabase::add_tag_link(int package_id, int tag_id)
{
	SQLRecord sqlInsert;
	sqlInsert.addField("packages_package_id", package_id);
	sqlInsert.addField("tags_tag_id", tag_id);
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
	sqlInsert.addField("package_installed", package->installed());
	sqlInsert.addField("package_configexist", package->configexist());
	sqlInsert.addField("package_action", package->action());

	sqlInsert.addField("package_md5", package->get_md5());
	sqlInsert.addField("package_filename", package->get_filename());

	db.sql_insert("packages", sqlInsert);
	
	// Retrieving package ID
	int package_id=db.getLastID();//get_last_id("packages", "package_id");
	package->set_id(package_id);
	if (package_id==0) exit(-100);
	
	// INSERT INTO FILES
	if (!package->get_files()->empty()) add_filelist_record(package_id, package->get_files());
	
	// INSERT INTO LOCATIONS
	if (!package->get_locations()->empty()) add_locationlist_record(package_id, package->get_locations());

	//INSERT INTO DEPENDENCIES
	if (!package->get_dependencies()->empty()) add_dependencylist_record(package_id, package->get_dependencies());

	// INSERT INTO TAGS
	if (!package->get_tags()->empty()) add_taglist_record(package_id, package->get_tags());
	
#ifdef ENABLE_INTERNATIONAL
	// INSERT INTO DESCRIPTIONS
	add_descriptionlist_record(package_id, package->get_descriptions());
#endif
	return package_id;
}	



//--------------------Mid-level functions-------------------------

void mpkgDatabase::createDBCache()
{
	if (db.internalDataChanged)
	{
		mDebug("Creating cache");
		SQLRecord sqlSearch;
		get_packagelist(&sqlSearch, &packageDBCache);
		db.internalDataChanged=false;
	}
}

int mpkgDatabase::get_package(int package_id, PACKAGE *package)//, bool GetExtraInfo)
{
	createDBCache();
	for (int i=0; i<packageDBCache.size(); i++)
	{
		if (packageDBCache.get_package(i)->get_id()==package_id)
		{
			*package=*packageDBCache.get_package(i);
			return 0;
		}
	}
	return MPKGERROR_NOPACKAGE;
}

int mpkgDatabase::get_packagelist (SQLRecord *sqlSearch, PACKAGE_LIST *packagelist)//, bool GetExtraInfo, bool ultraFast)
{
	if (sqlSearch->empty() && !db.internalDataChanged)
	{
		mDebug("SQL optimization using full cache");
		*packagelist=packageDBCache;
		return 0;
	}

#ifdef OPTIMIZE_SQL_QUERIES
	if (sqlSearch->size()==1 && sqlSearch->getEqMode()==EQ_EQUAL )
	{
		if (*sqlSearch->getFieldName(0)=="package_name")
		{
			mDebug("SQL optimization by package_name");
			string pattern=*sqlSearch->getValue("package_name");
			createDBCache(GetExtraInfo);
			packagelist->clear();
			for (unsigned int i=0; i<packageDBCache.size(); i++)
			{
				if (*packageDBCache.get_package(i)->get_name()==pattern)
				{
					packagelist->add(packageDBCache.get_package(i));
				}
			}
			return 0;
		}
		if (*sqlSearch->getFieldName(0)=="package_id")
		{
			mDebug("SQL optimization by package_id");
			int id=atoi(sqlSearch->getValue("package_id")->c_str());
			createDBCache(GetExtraInfo);
			packagelist->clear();
			for (unsigned int i=0; i<packageDBCache.size(); i++)
			{
				if (packageDBCache.get_package(i)->get_id()==id)
				{
					packagelist->add(packageDBCache.get_package(i));
					return 0;
				}
			}
			return 0;
		}


	}
#endif

	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	PACKAGE package;
	int sql_ret=db.get_sql_vtable(sqlTable, sqlFields, "packages", *sqlSearch);
	if (sql_ret!=0)
	{
		return MPKGERROR_SQLQUERYERROR;
	}
	packagelist->clear(sqlTable->getRecordCount());
	
	for (int i=0; i<sqlTable->getRecordCount(); i++)
	{
		packagelist->get_package(i)->set_id(atoi(sqlTable->getValue(i, "package_id")->c_str()));
		packagelist->get_package(i)->set_name(sqlTable->getValue(i, "package_name"));
		packagelist->get_package(i)->set_version(sqlTable->getValue(i, "package_version"));
		packagelist->get_package(i)->set_arch(sqlTable->getValue(i, "package_arch"));
		packagelist->get_package(i)->set_build(sqlTable->getValue(i, "package_build"));
		packagelist->get_package(i)->set_compressed_size(sqlTable->getValue(i, "package_compressed_size"));
		packagelist->get_package(i)->set_installed_size(sqlTable->getValue(i, "package_installed_size"));
		packagelist->get_package(i)->set_short_description(sqlTable->getValue(i, "package_short_description"));
		packagelist->get_package(i)->set_description(sqlTable->getValue(i, "package_description"));
		packagelist->get_package(i)->set_changelog(sqlTable->getValue(i, "package_changelog"));
		packagelist->get_package(i)->set_packager(sqlTable->getValue(i, "package_packager"));
		packagelist->get_package(i)->set_packager_email(sqlTable->getValue(i, "package_packager_email"));
		packagelist->get_package(i)->set_installed(atoi(sqlTable->getValue(i,"package_installed")->c_str()));
		packagelist->get_package(i)->set_configexist(atoi(sqlTable->getValue(i,"package_configexist")->c_str()));
		packagelist->get_package(i)->set_action(atoi(sqlTable->getValue(i,"package_action")->c_str()));

		packagelist->get_package(i)->set_md5(sqlTable->getValue(i, "package_md5"));
		packagelist->get_package(i)->set_filename(sqlTable->getValue(i, "package_filename"));
#ifdef ENABLE_INTERNATIONAL
		get_descriptionlist(packagelist->get_package(i)->get_id(), packagelist->get_package(i)->get_descriptions());
#endif
	}
	get_full_taglist(packagelist);
	get_full_dependencylist(packagelist);
	get_full_locationlist(packagelist);
	if (sqlSearch->empty())
	{
		packageDBCache=*packagelist;
		db.internalDataChanged=false;
	}

	delete sqlTable;
	return 0;

}
#ifdef ENABLE_INTERNATIONAL
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
#endif

int mpkgDatabase::get_filelist(int package_id, vector<FILES> *filelist, bool config_only)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("file_name");
	sqlFields.addField("file_type");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", package_id);

	db.get_sql_vtable(sqlTable, sqlFields, "files", sqlSearch);
	filelist->clear();
	filelist->resize(sqlTable->getRecordCount());
	for (int row=0;row<sqlTable->getRecordCount();row++)
	{
		if (!config_only)
		{
			filelist->at(row).set_name(sqlTable->getValue(row, "file_name"));
			filelist->at(row).set_type(atoi(sqlTable->getValue(row, "file_type")->c_str()));
		}
		else
		{
			if (*sqlTable->getValue(row, "file_type")==IntToStr(FTYPE_CONFIG))
			{
				filelist->at(row).set_name(sqlTable->getValue(row, "file_name"));
				filelist->at(row).set_type(atoi(sqlTable->getValue(row, "file_type")->c_str()));
			}
		}
	}
	delete sqlTable;
	return 0;
}

void mpkgDatabase::get_full_filelist(PACKAGE_LIST *pkgList)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlSearch;
	SQLRecord sqlFields;
	sqlFields.addField("file_name");
	sqlFields.addField("file_type");
	sqlFields.addField("packages_package_id");
	//printf("query start\n");
	db.get_sql_vtable(sqlTable, sqlFields, "files", sqlSearch);
	//printf("query end\n");
	FILES tmp;
	int package_id;
	unsigned int counter=0;
	for (int i=0; i<sqlTable->size(); i++)
	{
		package_id=atoi(sqlTable->getValue(i, "packages_package_id")->c_str());
		for (int t=0; t<pkgList->size(); t++)
		{
			counter++;
			if (pkgList->get_package(t)->get_id()==package_id)
			{
				tmp.set_name(sqlTable->getValue(i, "file_name"));
				tmp.set_type(atoi(sqlTable->getValue(i, "file_type")->c_str()));
				pkgList->get_package(t)->get_files()->push_back(tmp);
			}
		}
	}
	//printf("linking end\n");
	delete sqlTable;
	for (int i=0; i<pkgList->size(); i++)
	{
		pkgList->get_package(i)->sync();
	}
	//printf("syncling end, total %d iterations\n", counter);
}



int mpkgDatabase::get_dependencylist(int package_id, vector<DEPENDENCY> *deplist)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("dependency_condition");
	sqlFields.addField("dependency_type");
	sqlFields.addField("dependency_package_name");
	sqlFields.addField("dependency_package_version");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", package_id);

	db.get_sql_vtable(sqlTable, sqlFields, "dependencies", sqlSearch);

	deplist->clear();
	deplist->resize(sqlTable->getRecordCount());
	for (int row=0; row<sqlTable->getRecordCount(); row++)
	{
		deplist->at(row).set_condition(sqlTable->getValue(row, "dependency_condition"));
		deplist->at(row).set_type(sqlTable->getValue(row, "dependency_type"));
		deplist->at(row).set_package_name(sqlTable->getValue(row, "dependency_package_name"));
		deplist->at(row).set_package_version(sqlTable->getValue(row, "dependency_package_version"));
	}
	return 0;
}

void mpkgDatabase::get_full_dependencylist(PACKAGE_LIST *pkgList) //TODO: incomplete
{
	SQLRecord fields;
	SQLRecord search;
	SQLTable deplist;
	db.get_sql_vtable(&deplist, fields, "dependencies", search);
	string pid_str;
	DEPENDENCY dep_tmp;
	for (int i=0; i<deplist.size(); i++)
	{
		for (int p=0; p<pkgList->size(); p++)
		{
			if (pkgList->get_package(p)->get_id()==atoi(deplist.getValue(i, "packages_package_id")->c_str()))
			{
				dep_tmp.set_condition(deplist.getValue(i, "dependency_condition"));
				dep_tmp.set_type(deplist.getValue(i, "dependency_type"));
				dep_tmp.set_package_name(deplist.getValue(i, "dependency_package_name"));
				dep_tmp.set_package_version(deplist.getValue(i, "dependency_package_version"));
				pkgList->get_package(p)->get_dependencies()->push_back(dep_tmp);
			}
		}
	}
}

void mpkgDatabase::get_full_taglist(PACKAGE_LIST *pkgList)
{
	int counter=0;
	int success=0;
	SQLRecord fields;
	SQLRecord search;
	SQLTable tags;
	SQLTable links;

	db.get_sql_vtable(&tags, fields, "tags", search);
	
	fields.addField("packages_package_id");
	fields.addField("tags_tag_id");
	db.get_sql_vtable(&links, fields, "tags_links", search);
	string tag_id_str;
	for (int i=0; i<links.size(); i++)
	{
		for (int p=0; p<pkgList->size(); p++)
		{
			counter++;
			if (pkgList->get_package(p)->get_id()==atoi(links.getValue(i, "packages_package_id")->c_str()))
			{
				tag_id_str=*links.getValue(i, "tags_tag_id");
				for (int t=0; t<tags.size(); t++)
				{
					if (*tags.getValue(t, "tags_id")==tag_id_str)
					{
						success++;
						pkgList->get_package(p)->get_tags()->push_back(*tags.getValue(t, "tags_name"));
						break;
					}
				}
			}
		}
	}
}



int mpkgDatabase::get_taglist(int package_id, vector<string> *taglist)
{
	// Step 1. Read link table, and create a list of tag's ids.
	SQLTable *id_sqlTable = new SQLTable;
	SQLRecord id_sqlFields;
	id_sqlFields.addField("tags_tag_id");
	SQLRecord id_sqlSearch;
	id_sqlSearch.addField("packages_package_id", package_id);
	db.get_sql_vtable(id_sqlTable, id_sqlFields, "tags_links", id_sqlSearch);

	taglist->clear();
	SQLTable *sqlTable;
	SQLRecord sqlFields;
	sqlFields.addField("tags_name");
	SQLRecord sqlSearch;

	if (!id_sqlTable->empty())
	{
		sqlTable = new SQLTable;
		sqlSearch.setSearchMode(SEARCH_OR);
		for (int i=0; i<id_sqlTable->getRecordCount(); i++)
		{
			sqlSearch.addField("tags_id", id_sqlTable->getValue(i, "tags_tag_id"));
		}
	}
	else
	{
		delete id_sqlTable;
		return 0;
	}
	delete id_sqlTable;

	// Step 2. Read the tags with readed ids
	db.get_sql_vtable(sqlTable, sqlFields, "tags", sqlSearch);

/*
	if (!sqlTable->empty())
	{
		taglist->resize(sqlTable->getRecordCount());
		for (int i=0; i<sqlTable->getRecordCount(); i++)
		{
			taglist->at(i)=*sqlTable->getValue(i, "tags_name");
		}
	}*/
	delete sqlTable;
	return 0;
}

void mpkgDatabase::get_available_tags(vector<string> *output)
{
	SQLTable sqlTable;
	SQLRecord sqlSearch;
	SQLRecord sqlFields;
	sqlFields.addField("tags_name");
	db.get_sql_vtable(&sqlTable, sqlFields, "tags", sqlSearch);
	output->clear();
	output->resize(sqlTable.size());
	for (int i=0; i<sqlTable.size(); i++)
	{
		output->at(i)=*sqlTable.getValue(i,"tags_name");
	}
}

int mpkgDatabase::get_locationlist(int package_id, vector<LOCATION> *location_list)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("location_id");
	sqlFields.addField("server_url");
	sqlFields.addField("location_path");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", package_id);
	db.get_sql_vtable(sqlTable, sqlFields, "locations", sqlSearch);
	location_list->clear();
	location_list->resize(sqlTable->getRecordCount());
	for (int i=0; i<sqlTable->getRecordCount(); i++)
	{
		location_list->at(i).set_id(atoi(sqlTable->getValue(i, "location_id")->c_str()));
		location_list->at(i).set_server_url(sqlTable->getValue(i, "server_url"));
		location_list->at(i).set_path(sqlTable->getValue(i, "location_path"));
	}
	delete sqlTable;
	return 0;
}

void mpkgDatabase::get_full_locationlist(PACKAGE_LIST *pkgList)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	SQLRecord sqlSearch;
	sqlFields.addField("location_path");
	sqlFields.addField("location_id");
	sqlFields.addField("server_url");
	sqlFields.addField("packages_package_id");

	db.get_sql_vtable(sqlTable, sqlFields, "locations", sqlSearch);

	int package_id;
	LOCATION tmp;
	for (int i=0; i<sqlTable->size(); i++)
	{
		package_id = atoi(sqlTable->getValue(i, "packages_package_id")->c_str());
		for (int t=0; t<pkgList->size(); t++)
		{
			if (pkgList->get_package(t)->get_id()==package_id)
			{
				tmp.set_id(atoi(sqlTable->getValue(i, "location_id")->c_str()));
				tmp.set_server_url(sqlTable->getValue(i, "server_url"));
				tmp.set_path(sqlTable->getValue(i, "location_path"));
				pkgList->get_package(t)->get_locations()->push_back(tmp);
			}
		}
	}
	delete sqlTable;
}

int mpkgDatabase::get_package_id(PACKAGE *package)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_id");
	SQLRecord sqlSearch;
	sqlSearch.addField("package_md5", package->get_md5());
	db.get_sql_vtable(sqlTable, sqlFields, "packages", sqlSearch);

	if (sqlTable->empty())
	{
		delete sqlTable;
		return 0;
	}
	if (sqlTable->getRecordCount()==1)
	{
		int ret = atoi(sqlTable->getValue(0, "package_id")->c_str());
		delete sqlTable;
		return ret;
	}
	if (sqlTable->getRecordCount()>1)
	{
		delete sqlTable;
		mError("Multiple package records, internal error!");
		return -1;
	}

	return -1;
}

int mpkgDatabase::set_installed(int package_id, int status)
{
       	SQLRecord sqlUpdate;
	sqlUpdate.addField("package_installed", status);
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);
	return db.sql_update("packages", sqlUpdate, sqlSearch);
}
int mpkgDatabase::set_configexist(int package_id, int status)
{
       	SQLRecord sqlUpdate;
	sqlUpdate.addField("package_configexist", status);
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);
	return db.sql_update("packages", sqlUpdate, sqlSearch);
}
int mpkgDatabase::set_action(int package_id, int status)
{
       	SQLRecord sqlUpdate;
	sqlUpdate.addField("package_action", status);
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);
	return db.sql_update("packages", sqlUpdate, sqlSearch);
}


int mpkgDatabase::get_installed(int package_id)
{
	mDebug("checking " + IntToStr(package_id) );
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_installed");
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);
	db.get_sql_vtable(sqlTable, sqlFields, "packages", sqlSearch);

	if (sqlTable->empty())
	{
		delete sqlTable;
		return -1;
	}
	if (sqlTable->getRecordCount()==1)
	{
		int ret = atoi(sqlTable->getValue(0, "package_installed")->c_str());
		delete sqlTable;
		return ret;
	}
	if (sqlTable->getRecordCount()>1)
	{
		delete sqlTable;
		return -3;
	}
	
	return -100;
}

int mpkgDatabase::get_action(int package_id)
{
	mDebug("requesting " + IntToStr(package_id));
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_action");
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);
	db.get_sql_vtable(sqlTable, sqlFields, "packages", sqlSearch);

	if (sqlTable->empty())
	{
		delete sqlTable;
		return -1;
	}
	if (sqlTable->getRecordCount()==1)
	{
		int ret = atoi(sqlTable->getValue(0, "package_action")->c_str());
		delete sqlTable;
		return ret;
	}
	if (sqlTable->getRecordCount()>1)
	{
		delete sqlTable;
		return -3;
	}
	mError("shouldn't be here");
	return -100;
}

int mpkgDatabase::get_configexist(int package_id)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("package_configexist");
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);
	db.get_sql_vtable(sqlTable, sqlFields, "packages", sqlSearch);

	if (sqlTable->empty())
	{
		delete sqlTable;
		return -1;
	}
	if (sqlTable->getRecordCount()==1)
	{
		int ret = atoi(sqlTable->getValue(0, "package_configexist")->c_str());
		delete sqlTable;
		return ret;
	}
	if (sqlTable->getRecordCount()>1)
	{
		delete sqlTable;
		return -3;
	}
	
	return -100;
}

int mpkgDatabase::get_available(int package_id)
{
	mError("Deprecated");
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlFields;
	sqlFields.addField("packages_package_id");
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", package_id);
	db.get_sql_vtable(sqlTable, sqlFields, "locations", sqlSearch);

	if (sqlTable->empty())
	{
		delete sqlTable;
		return 0;
	}
	else
	{
		delete sqlTable;
		return sqlTable->size();
		//int ret = atoi(sqlTable->getValue(0, "package_available")->c_str());
	}
}

int mpkgDatabase::get_purge(string *package_name)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", package_name);
	SQLTable sqlTable;
	SQLTable sqlTableFiles;
	SQLRecord sqlFields;
	sqlFields.addField("package_id");
	sqlFields.addField("package_configexist");
	if (db.get_sql_vtable(&sqlTable, sqlFields, "packages", sqlSearch)!=0)
	{
		return -1;
	}
	if (sqlTable.empty())
	{
		return 0;
	}
	int id=0;
	for (int i=0; i<sqlTable.getRecordCount(); i++)
	{
		if (*sqlTable.getValue(i, "package_configexist")==IntToStr(ST_CONFIGEXIST))
		{
			id=atoi(sqlTable.getValue(i, "package_id")->c_str());
			break;
		}
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
string* SQLRecord::getFieldName(unsigned int num)
{
	return &field[num].fieldname;
}

string* SQLRecord::getValue(string fieldname)
{
	//debug("fieldname = " + fieldname);
	for (unsigned int i=0;i<field.size();i++)
	{
		if (field[i].fieldname==fieldname)
		{
			PrepareSql(&field[i].value);
			return &field[i].value;
		}
	}
	mError("No such field " + fieldname);
	mError("Available fields:");
	for (unsigned int i=0; i<field.size(); i++)
	{
		mError(field[i].fieldname);
	}
	abort();
	return NULL; // Means error
}

bool SQLRecord::setValue(string fieldname, string *value)
{
	for (unsigned int i=0; i<field.size(); i++)
	{
		if (field[i].fieldname==fieldname)
		{
			field[i].value=*value;
			return true;
		}
	}
	return false;
}

void SQLRecord::addField(string fieldname, string *value)
{
	SQLField tmp;
	tmp.fieldname=fieldname;
	tmp.value=*value;
	field.push_back(tmp);
}

void SQLRecord::addField(string fieldname)
{
	string nullstr;
	addField(fieldname, &nullstr);
}

void SQLRecord::addField(string fieldname, int value)
{
	string istr = IntToStr(value);
	addField(fieldname, &istr);
}
string* SQLRecord::getValueI(unsigned int num)
{
		PrepareSql(&field[num].value);
		return &field[num].value;
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

int SQLTable::size()
{
	return table.size();
}
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

string* SQLTable::getValue(unsigned int num, string fieldname)
{
	if (num<table.size())
	{
		return table[num].getValue(fieldname);
	}
	else 
	{
		mError("Cannot find field " + fieldname+ " with ID " + IntToStr(num) + ": no such ID. Aborting...");
		abort();
		return NULL;
	}
}

SQLRecord* SQLTable::getRecord(unsigned int num)
{
	if (num<table.size()) return &table[num];
	else
	{
		mError("core.cpp: SQLTable::getRecord():  record number " + IntToStr(num) + "is out of range");
		abort();
		return NULL;
	}
}

void SQLTable::addRecord(SQLRecord* record)
{
	table.push_back(*record);
}


SQLTable::SQLTable(){}
SQLTable::~SQLTable(){}



