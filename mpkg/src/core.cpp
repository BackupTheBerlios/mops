/********************************************************************************
 *
 * 			Central core for MOPSLinux package system
 *			TODO: Should be reorganized to objects
 *	$Id: core.cpp,v 1.81 2007/12/10 03:12:58 i27249 Exp $
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
int compareVersions(string *version1, string *beta1, string *version2, string *beta2) // 0 == equal, 1 == v1>v2, -1 == v1<v2
{
	int ret;
	ret = strverscmp(version1->c_str(), version2->c_str());
	if (ret==0)
	{
		if (*beta2=="0" || beta2->empty()) {
			if (*beta1=="0" || beta1->empty()) return ret;
		}
		ret = strverscmp(beta1->c_str(), beta2->c_str());
	}
	return ret;
}
PACKAGE* mpkgDatabase::get_max_version(PACKAGE_LIST *pkgList, DEPENDENCY *dep)
{
	// Maybe optimization possible
	PACKAGE *candidate=NULL;
	string ver, beta;
	for (int i=0; i<pkgList->size(); i++)
	{
		if (pkgList->get_package(i)->reachable() \
				&& compareVersions(pkgList->get_package(i)->get_version(), pkgList->get_package(i)->get_betarelease(), &ver, &beta)>0 \
				/*&& strverscmp(pkgList->get_package(i)->get_version()->c_str(), ver.c_str())>0 \*/
				&& mpkgDatabase::checkVersion(pkgList->get_package(i)->get_version(), \
					atoi(dep->get_condition()->c_str()), \
					dep->get_package_version())
		   )
		{
			candidate = pkgList->get_package(i);
			ver = *pkgList->get_package(i)->get_version();
			beta = *pkgList->get_package(i)->get_betarelease();
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
	if (verbose) say(_("Checking file conflicts for package %s\n"), package->get_name()->c_str());
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
	PACKAGE tmpP;
	bool hasBackups=false;
	if (package->get_files()->size()==0) return 0; // If a package has no files, it cannot conflict =)
	for (unsigned int i=0;i<package->get_files()->size();i++)
	{
		fname=*package->get_files()->at(i).get_name();
		if (fname[fname.length()-1]!='/') 
		{
			sqlSearch.addField("file_name", package->get_files()->at(i).get_name());
		}
	}
	db.get_sql_vtable(sqlTable, sqlFields, "files", sqlSearch);
	int fPackages_package_id = sqlTable->getFieldIndex("packages_package_id");
	int fFile_name = sqlTable->getFieldIndex("file_name");
	// TODO: Next: to be refactored to detect already backed up files, and some speedups (get queries together)
	// TODO 2: REWRITE ALL CODE ABOUT BACKUP! Sometimes it's action destroys the whole system. Will disable backups for now...
	if (!sqlTable->empty())
	{
		for (int k=0;k<sqlTable->getRecordCount() ;k++) // Excluding from check packages who are already installed
		{
			package_id=atoi(sqlTable->getValue(k, fPackages_package_id)->c_str());
			if (package_id!=prev_package_id)
			{
				if (get_installed(package_id) || get_action(package_id)==ST_INSTALL)
				{
					get_package(package_id, &tmpP);
					//say("File %s conflicts with package %s, backing up\n", sqlTable->getValue(k, "file_name")->c_str(), tmpP.get_name()->c_str());
					printf("Warning: overlapped file %s\n", sqlTable->getValue(k, fFile_name)->c_str());
					hasBackups=true;
					backupFile(sqlTable->getValue(k, fFile_name), package_id, package->get_id());
				}
			}
		}
	}
	delete sqlTable;
	if (hasBackups) {
		say("An overlapped files detected. Break the operation within 3 seconds if you not sure\n");
		sleep(3);
	}
	return 0; // End of check_file_conflicts
}

int mpkgDatabase::add_conflict_record(int conflicted_id, int overwritten_id, string *file_name)
{
	PACKAGE pkg;
	get_package(overwritten_id, &pkg,true);

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
	//sqlFields.addField("overwritten_package_id");
	SQLTable fTable;
	db.get_sql_vtable(&fTable, sqlFields, "conflicts", sqlSearch);
	ret->clear();
	ret->resize(fTable.getRecordCount());

	int fConflict_file_name = fTable.getFieldIndex("conflict_file_name");
	int fBackup_file = fTable.getFieldIndex("backup_file");
	for (int i=0; i<fTable.getRecordCount(); i++)
	{
		ret->at(i).set_name(fTable.getValue(i, fConflict_file_name));
		ret->at(i).set_backup_file(fTable.getValue(i, fBackup_file));
		ret->at(i).owner_id = -1;
		//ret->at(i).owner_id = atoi(fTable.getValue(i, fOverwritten_id)->c_str());
		ret->at(i).overwriter_id = conflicted_id;
	}
}
void mpkgDatabase::get_backup_records(PACKAGE *package, vector<FILES> *ret)
{
	SQLRecord sqlSearch;
	sqlSearch.setEqMode(EQ_CUSTOMLIKE);
	string backupDir = *package->get_name() + "_" + *package->get_md5()+"%";
	sqlSearch.addField("backup_file", &backupDir);
	SQLRecord sqlFields;
	sqlFields.addField("backup_file");
	sqlFields.addField("conflict_file_name");
	sqlFields.addField("conflicted_package_id");
	SQLTable fTable;
	db.get_sql_vtable(&fTable, sqlFields, "conflicts", sqlSearch);
	ret->clear();
	ret->resize(fTable.getRecordCount());

	int fConflict_file_name = fTable.getFieldIndex("conflict_file_name");
	int fBackup_file = fTable.getFieldIndex("backup_file");
	int fOverwriter_id = fTable.getFieldIndex("conflicted_package_id");
	for (int i=0; i<fTable.getRecordCount(); i++)
	{
		//printf("Found backup: %s\n", fTable.getValue(i, fBackup_file)->c_str());
		ret->at(i).set_name(fTable.getValue(i, fConflict_file_name));
		ret->at(i).set_backup_file(fTable.getValue(i, fBackup_file));
		ret->at(i).owner_id = package->get_id();
		ret->at(i).overwriter_id = atoi(fTable.getValue(i, fOverwriter_id)->c_str());
	}
}

int mpkgDatabase::backupFile(string *filename, int overwritten_package_id, int conflicted_package_id)
{
	if (FileExists(SYS_ROOT + *filename))
	{
		PACKAGE pkg;
		get_package(overwritten_package_id, &pkg, true);
		string bkpDir = SYS_BACKUP + *pkg.get_name() + "_" + *pkg.get_md5();
		string bkpDir2 = bkpDir + "/" + filename->substr(0, filename->find_last_of("/"));
		string mkd = "mkdir -p " + bkpDir2;
		
		string mv = "mv ";
	        mv += SYS_ROOT + *filename + " " + bkpDir2 + "/";
		if (verbose) say("%s\n", mv.c_str());
		if (!simulate)
		{
			if (system(mkd.c_str())!=0 ||  system(mv.c_str())!=0)
			{
				mError("Error while backup");
				return MPKGERROR_FILEOPERATIONS;
			}
		}
		add_conflict_record(conflicted_package_id, overwritten_package_id, filename);
		FILE *log = fopen("/var/log/mpkg-backups.log","a");
		if (log)
		{
			string target_name = *pkg.get_name() + "-" + pkg.get_fullversion();
			get_package(conflicted_package_id, &pkg, true);
			string overwriter_name = *pkg.get_name() + "-" + pkg.get_fullversion();
			fprintf(log, "FILE: [%s] TARGET: [%s] OVERWRITER: [%s]\n", filename->c_str(), target_name.c_str(), overwriter_name.c_str());
			fclose(log);
		}
		else
		{
			mError("Unable to open log file /var/log/mpkg-backups.log");
		}
	}
	return 0;
}

int _cleanBackupCallback(const char *filename, const struct stat *file_status, int filetype)
{
	unsigned short x=0, y=0;

	if (file_status->st_ino!=0) x=y;
	if (filetype == FTW_D && strcmp(filename, SYS_BACKUP.c_str())!=0 && !simulate ) rmdir(filename);
	return 0;
}

void mpkgDatabase::clean_backup_directory()
{
	ftw(SYS_BACKUP.c_str(), _cleanBackupCallback, 20);
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
		sqlInsert.addField("dependency_build_only", deplist->at(i).isBuildOnly());
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
	int fTags_id = 0; //sqlTable->getFieldIndex("tags_id"); // WARNING: if you change the field numbering, change this too!!!
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
		tag_id=atoi(sqlTable->getValue(0, fTags_id)->c_str());
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
	sqlInsert.addField("package_betarelease", package->get_betarelease());
	sqlInsert.addField("package_installed_by_dependency", package->get_installed_by_dependency());
	sqlInsert.addField("package_type", package->get_type());

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

int mpkgDatabase::get_package(int package_id, PACKAGE *package, bool no_cache)//, bool GetExtraInfo)
{
	if (no_cache && db.internalDataChanged)
	{
		SQLRecord sqlSearch;
		PACKAGE_LIST pkgList;
		sqlSearch.addField("package_id", package_id);
		get_packagelist(&sqlSearch, &pkgList);
		if (pkgList.size()==1) {
			*package = *pkgList.get_package(0);
			return 0;
		}
		else return MPKGERROR_NOPACKAGE;

	}
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

int mpkgDatabase::get_packagelist (SQLRecord *sqlSearch, PACKAGE_LIST *packagelist, bool ultraFast)//, bool GetExtraInfo, bool ultraFast)
{
	// Ultrafast will request only basic info about package
	
	if (sqlSearch->empty() && !db.internalDataChanged)
	{
		mDebug("SQL optimization using full cache");
		*packagelist=packageDBCache;
		return 0;
	}
//#define OPTIMIZE_SQL_QUERIES 		// Disabled because of unknown (forgotten) behaviour
#ifdef OPTIMIZE_SQL_QUERIES
	printf("SQL optimization enabled\n");
	if (sqlSearch->size()==1 && sqlSearch->getEqMode()==EQ_EQUAL )
	{
		if (*sqlSearch->getFieldName(0)=="package_name")
		{
			printf("SQL optimization by package_name\n");
			string pattern=*sqlSearch->getValue("package_name");
			createDBCache();
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
			printf("SQL optimization by package_id\n");
			int id=atoi(sqlSearch->getValue("package_id")->c_str());
			createDBCache();
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
	
	// Creating field index
	int fPackage_id = sqlTable->getFieldIndex("package_id");
	int fPackage_name = sqlTable->getFieldIndex("package_name");
	int fPackage_version = sqlTable->getFieldIndex("package_version");
	int fPackage_arch = sqlTable->getFieldIndex("package_arch");
	int fPackage_build = sqlTable->getFieldIndex("package_build");
	int fPackage_compressed_size = sqlTable->getFieldIndex("package_compressed_size");
	int fPackage_installed_size = sqlTable->getFieldIndex("package_installed_size");
	int fPackage_short_description = sqlTable->getFieldIndex("package_short_description");
	int fPackage_description = sqlTable->getFieldIndex("package_description");
	int fPackage_changelog = sqlTable->getFieldIndex("package_changelog");
	int fPackage_packager = sqlTable->getFieldIndex("package_packager");
	int fPackage_packager_email = sqlTable->getFieldIndex("package_packager_email");
	int fPackage_installed = sqlTable->getFieldIndex("package_installed");
	int fPackage_configexist = sqlTable->getFieldIndex("package_configexist");
	int fPackage_action = sqlTable->getFieldIndex("package_action");
	int fPackage_md5 = sqlTable->getFieldIndex("package_md5");
	int fPackage_filename = sqlTable->getFieldIndex("package_filename");
	int fPackage_betarelease =sqlTable->getFieldIndex("package_betarelease");
	int fPackage_installed_by_dependency = sqlTable->getFieldIndex("package_installed_by_dependency");
	int fPackage_type = sqlTable->getFieldIndex("package_type");


	for (int i=0; i<sqlTable->getRecordCount(); i++)
	{
		packagelist->get_package(i)->set_id(atoi(sqlTable->getValue(i, fPackage_id)->c_str()));
		packagelist->get_package(i)->set_name(sqlTable->getValue(i, fPackage_name));
		packagelist->get_package(i)->set_version(sqlTable->getValue(i, fPackage_version));
		packagelist->get_package(i)->set_arch(sqlTable->getValue(i, fPackage_arch));
		packagelist->get_package(i)->set_build(sqlTable->getValue(i, fPackage_build));
		packagelist->get_package(i)->set_compressed_size(sqlTable->getValue(i, fPackage_compressed_size));
		packagelist->get_package(i)->set_installed_size(sqlTable->getValue(i, fPackage_installed_size));
		packagelist->get_package(i)->set_short_description(sqlTable->getValue(i, fPackage_short_description));
		packagelist->get_package(i)->set_description(sqlTable->getValue(i, fPackage_description));
		packagelist->get_package(i)->set_changelog(sqlTable->getValue(i, fPackage_changelog));
		packagelist->get_package(i)->set_packager(sqlTable->getValue(i, fPackage_packager));
		packagelist->get_package(i)->set_packager_email(sqlTable->getValue(i, fPackage_packager_email));
		packagelist->get_package(i)->set_installed(atoi(sqlTable->getValue(i,fPackage_installed)->c_str()));
		packagelist->get_package(i)->set_configexist(atoi(sqlTable->getValue(i, fPackage_configexist)->c_str()));
		packagelist->get_package(i)->set_action(atoi(sqlTable->getValue(i, fPackage_action)->c_str()));

		packagelist->get_package(i)->set_md5(sqlTable->getValue(i, fPackage_md5));
		packagelist->get_package(i)->set_filename(sqlTable->getValue(i, fPackage_filename));
		packagelist->get_package(i)->set_betarelease(sqlTable->getValue(i, fPackage_betarelease));
		packagelist->get_package(i)->set_installed_by_dependency(atoi(sqlTable->getValue(i, fPackage_installed_by_dependency)->c_str()));
		packagelist->get_package(i)->set_type(atoi(sqlTable->getValue(i, fPackage_type)->c_str()));
#ifdef ENABLE_INTERNATIONAL
		get_descriptionlist(packagelist->get_package(i)->get_id(), packagelist->get_package(i)->get_descriptions());
#endif
	}
	if (!ultraFast) 
	{
		get_full_taglist(packagelist);
		get_full_dependencylist(packagelist);
	}
	get_full_locationlist(packagelist);
	if (!ultraFast && sqlSearch->empty())
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
int mpkgDatabase::get_sql_vtable(SQLTable *output, SQLRecord fields, string table_name, SQLRecord search)
{
	return db.get_sql_vtable(output, fields, table_name, search);
}

void mpkgDatabase::get_full_filelist(PACKAGE_LIST *pkgList)
{
	SQLTable *sqlTable = new SQLTable;
	SQLRecord sqlSearch;
	SQLRecord sqlFields;
	sqlFields.addField("file_name");
	sqlFields.addField("file_type");
	sqlFields.addField("packages_package_id");
	db.get_sql_vtable(sqlTable, sqlFields, "files", sqlSearch);
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
	delete sqlTable;
	for (int i=0; i<pkgList->size(); i++)
	{
		pkgList->get_package(i)->sync();
	}
}




void mpkgDatabase::get_full_dependencylist(PACKAGE_LIST *pkgList) //TODO: incomplete
{
	SQLRecord fields;
	SQLRecord search;
	SQLTable deplist;
	db.get_sql_vtable(&deplist, fields, "dependencies", search); // Emerging the list of all existing dependencies
	string pid_str;
	DEPENDENCY dep_tmp;

	// Creating index
	int fPackages_package_id = deplist.getFieldIndex("packages_package_id");
	int fDependency_condition = deplist.getFieldIndex("dependency_condition");
	int fDependency_package_name = deplist.getFieldIndex("dependency_package_name");
	int fDependency_type = deplist.getFieldIndex("dependency_type");
	int fDependency_package_version = deplist.getFieldIndex("dependency_package_version");
	int fDependency_build_only = deplist.getFieldIndex("dependency_build_only");
	
	// Processing
	int currentDepID;
	for (int i=0; i<deplist.size(); i++)
	{
		currentDepID = atoi(deplist.getValue(i, fPackages_package_id)->c_str());
		for (int p=0; p<pkgList->size(); p++)
		{
			if (pkgList->get_package(p)->get_id()==currentDepID)
			{
				dep_tmp.set_condition(deplist.getValue(i, fDependency_condition));
				dep_tmp.set_type(deplist.getValue(i, fDependency_type));
				dep_tmp.set_package_name(deplist.getValue(i, fDependency_package_name));
				dep_tmp.set_package_version(deplist.getValue(i, fDependency_package_version));
				dep_tmp.setBuildOnly(atoi(deplist.getValue(i, fDependency_build_only)->c_str()));
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
	
	// Index
	
	int fLinksPackages_package_id = links.getFieldIndex("packages_package_id");
	int fLinksTags_tag_id = links.getFieldIndex("tags_tag_id");

	int fTagsTags_id = tags.getFieldIndex("tags_id");
	int fTagsTags_name = tags.getFieldIndex("tags_name");
	int currentLinkPackageID;
	for (int i=0; i<links.size(); i++)
	{
		currentLinkPackageID = atoi(links.getValue(i, fLinksPackages_package_id)->c_str());
		for (int p=0; p<pkgList->size(); p++)
		{
			counter++;
			if (pkgList->get_package(p)->get_id()==currentLinkPackageID)
			{
				tag_id_str=*links.getValue(i, fLinksTags_tag_id);
				for (int t=0; t<tags.size(); t++)
				{
					if (*tags.getValue(t, fTagsTags_id)==tag_id_str)
					{
						success++;
						pkgList->get_package(p)->get_tags()->push_back(*tags.getValue(t, fTagsTags_name));
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
	int fTags_name = sqlTable.getFieldIndex("tags_name");
	for (int i=0; i<sqlTable.size(); i++)
	{
		output->at(i)=*sqlTable.getValue(i,fTags_name);
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

	// Index
	int fPackages_package_id = sqlTable->getFieldIndex("packages_package_id");
	int fLocation_id = sqlTable->getFieldIndex("location_id");
	int fServer_url = sqlTable->getFieldIndex("server_url");
	int fLocation_path = sqlTable->getFieldIndex("location_path");

	for (int i=0; i<sqlTable->size(); i++)
	{
		package_id = atoi(sqlTable->getValue(i, fPackages_package_id)->c_str());
		for (int t=0; t<pkgList->size(); t++)
		{
			if (pkgList->get_package(t)->get_id()==package_id)
			{
				tmp.set_id(atoi(sqlTable->getValue(i, fLocation_id)->c_str()));
				tmp.set_server_url(sqlTable->getValue(i, fServer_url));
				tmp.set_path(sqlTable->getValue(i, fLocation_path));
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
		int ret = atoi(sqlTable->getValue(0, sqlTable->getFieldIndex("package_id"))->c_str());
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
		int ret = atoi(sqlTable->getValue(0, sqlTable->getFieldIndex("package_installed"))->c_str());
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
		int ret = atoi(sqlTable->getValue(0, sqlTable->getFieldIndex("package_action"))->c_str());
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
		int ret = atoi(sqlTable->getValue(0, sqlTable->getFieldIndex("package_configexist"))->c_str());
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
	int fPackage_configexist = sqlTable.getFieldIndex("package_configexist");
	int fPackage_id = sqlTable.getFieldIndex("package_id");
	for (int i=0; i<sqlTable.getRecordCount(); i++)
	{
		if (*sqlTable.getValue(i, fPackage_configexist)==IntToStr(ST_CONFIGEXIST))
		{
			id=atoi(sqlTable.getValue(i, fPackage_id)->c_str());
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

void SQLRecord::setValue(unsigned int field_index, string *value)
{
	field[field_index].value = *value;
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

int SQLRecord::getFieldIndex(string fieldname)
{
	for (unsigned int i=0;i<field.size();i++)
	{
		if (field[i].fieldname==fieldname)
		{
			return i;
		}
	}
	return -1;
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

string* SQLTable::getValue(unsigned int num, unsigned int field_index)
{
	return table[num].getValueI(field_index);
}

int SQLTable::getFieldIndex(string fieldname)
{
	if (table.size()==0) return -2;
	else return table[0].getFieldIndex(fieldname);
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



