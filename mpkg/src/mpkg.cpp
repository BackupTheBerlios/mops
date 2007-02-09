/***********************************************************************
 * 	$Id: mpkg.cpp,v 1.25 2007/02/09 14:26:38 i27249 Exp $
 * 	MOPSLinux packaging system
 * ********************************************************************/
#include "mpkg.h"
#include "syscommands.h"

/** Scans database and do actions. Actually, packages will install in SYS_ROOT folder (for testing).
 * In real systems, set SYS_ROOT to "/"
 * @**/

mpkgDatabase::mpkgDatabase()
{
	// Empty for now...
}
mpkgDatabase::~mpkgDatabase(){}

int uninstall(vector<string> pkgnames)
{
	// nothing to do here at this time...
	return 0;
}

PACKAGE mpkgDatabase::get_installed_package(string pkg_name)
{
	PACKAGE_LIST packagelist;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", pkg_name);
	sqlSearch.addField("package_status", IntToStr(PKGSTATUS_INSTALLED));

	get_packagelist(sqlSearch, &packagelist);
	// We do NOT allow multiple packages with same name to be installed, so, we simply get first package of list.
	
	if (packagelist.size()>0)
		return *packagelist.get_package(0);
	else
	{
		PACKAGE ret;
		return ret;
	}
}
	

int mpkgDatabase::emerge_to_db(PACKAGE *package)
{
	debug("mpkgDatabase::emerge_to_db()");
	int pkg_id;
	pkg_id=get_package_id(package);
	if (pkg_id==0)
	{
		debug ("mpkg.cpp: emerge_to_db(): Package is new, adding to database");
		add_package_record(package);
		return 0;
	}
	if (pkg_id<0)
	{
		debug("mpkg.cpp: emerge_to_db(): Database error, cannot emerge");
		return 1;
	}
	// Раз пакет уже в базе (и в единственном числе - а иначе и быть не должно), сравниваем данные.
	// В случае необходимости, добавляем location.
	debug ("mpkg.cpp: emerge_to_db(): Package is already in database, updating locations if needed");
	PACKAGE db_package;
	LOCATION_LIST new_locations;
	get_package(pkg_id, &db_package, true);
	package->set_id(pkg_id);

	for (int j=0; j<package->get_locations()->size(); j++)
	{
		debug("mpkg.cpp: emerge_to_db(): J cycle");
		for (int i=0; i<db_package.get_locations()->size(); i++)
		{
			debug("mpkg.cpp: emerge_to_db(): I cycle");
			if (package->get_locations()->get_location(j)->get_server()!=db_package.get_locations()->get_location(i)->get_server() || \
					package->get_locations()->get_location(j)->get_path()!=db_package.get_locations()->get_location(i)->get_path())
			{
				debug("mpkg.cpp: emerge_to_db(): ----------------->new location<--------------------");
				new_locations.add(*package->get_locations()->get_location(j));
			}
		}
	}
	if (!new_locations.IsEmpty()) add_locationlist_record(pkg_id, &new_locations);
	else debug ("mpkg.cpp: emerge_to_db(): no new locations");
	return 0;
}


string mpkgDatabase::get_file_md5(string filename)
{
	debug("get_file_md5 start");
	string tmp_md5=get_tmp_file();

	string sys="md5sum "+filename+" > "+tmp_md5;
	system(sys.c_str());
	FILE* md5=fopen(tmp_md5.c_str(), "r");
	if (!md5)
	{
		fprintf(stderr, "Unable to open md5 temp file\n");
		return "";
	}
	char _c_md5[1000];
	fscanf(md5, "%s", &_c_md5);
	string md5str;
	md5str=_c_md5;
	fclose(md5);
	return md5str;
}

int mpkgDatabase::commit_actions()
{
	// Zero: purging required packages
	// First: removing required packages
	PACKAGE_LIST remove_list;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_status", IntToStr(PKGSTATUS_REMOVE));
	if (get_packagelist(sqlSearch, &remove_list)!=0) return -1;
	debug ("Calling REMOVE for "+IntToStr(remove_list.size())+" packages");
	for (int i=0;i<remove_list.size();i++)
	{
		if (remove_package(remove_list.get_package(i))!=0) return -2;
	}
	sqlSearch.clear();

	PACKAGE_LIST purge_list;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_status", IntToStr(PKGSTATUS_REMOVE_PURGE));
	sqlSearch.addField("package_status", IntToStr(PKGSTATUS_PURGE));
	if (get_packagelist(sqlSearch, &purge_list)!=0) return -3;
	//printf("purge list size = %d\n", purge_list.size());
	for (int i=0; i<purge_list.size(); i++)
	{
		if (purge_package(purge_list.get_package(i))!=0) return -4;
	}
	sqlSearch.clear();

	// Second: installing required packages
	PACKAGE_LIST install_list;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_status", IntToStr(PKGSTATUS_INSTALL));
	if (get_packagelist(sqlSearch, &install_list)!=0) return -5;
	debug("Calling FETCH");
	debug("Preparing to fetch "+IntToStr(install_list.size())+" packages");

	for (int i=0; i<install_list.size(); i++)
	{
		debug("Fetching package #"+IntToStr(i+1)+" with "+IntToStr(install_list.get_package(i)->get_locations()->size())+" locations");
		if (fetch_package(install_list.get_package(i))!=0) return -6;
	}
	debug("Calling INSTALL");
	for (int i=0;i<install_list.size();i++)
	{
		if (install_package(install_list.get_package(i))!=0) return -7;
	}
	return 0;
}

int mpkgDatabase::fetch_package(PACKAGE *package)
{
		// Смотрим все доступные locations пакета, точнее - их сервера. 
		// Сервера подразделяются по типам на следующие группы:
		// cache - это локальный кеш пакетов. URL: cache://
		// file - по сути - псевдосервер, его наличие означает что пакет ставят напрямую с локального носителя. URL: file://
		// network - сетевой репозиторий. URL: http://, ftp://, rsync://, smb://, nfs://, https://, ftps://, и другие (какие реализуем). Пока только первые 2.
		// cdrom - по сути локальный репозиторий, но имеющий свою специфику. Вид URL: cdrom://cdrom_id/
		//
		// Тип сервера определяется по его URL методом SERVER::get_type().
		// 
		// Если тип сервера file, то сначала проверяется существование файла, после создается symlink из кеша на файл. Если файла нету - ищем дальше.
		// Если тип сервера cache, то достаточно просто проверить наличие файла в указанном месте и его md5. Если файла нет - ищем дальше.
		// 
		// Если тип сервера network - то мы запускаем выкачивание файла. Если файл скачался - проверяем его md5. Если файл не скачался или сумма не совпадает
		// то пробуем следующий сервер до тех пор пока файл не окажется в кеше. Кеш по умолчанию - SYS_CACHE.
		//
		// Если тип сервера cdrom - то мы сначала требуем вставить нужный диск в нужное устройство, монтируем его, и дальше как будто это file:
		// проверяем существование, создаем симлинк.
		//
		// Приоритет серверов определяется методом SERVER::get_priority(). Высший приоритет ВСЕГДА имеют сервера file. Умолчальная расстановка такова:
		// 1. cache (приоритеты от 3000 до 3999)
		// 2. cdrom (приоритеты от 2000 до 2999)
		// 3. network (приоритеты от 1 до 1999)
		// Сервер, имеющий приоритет 0, считается отключенным и никогда не принимает участие в выборке.
		//
		// TODO: многопотоковая загрузка с равноприоритетных серверов.

	// Step 1. Checking for local copy of file (filesystem or cache)
	debug("INIT/Fetching...");
	LOCATION_LIST locationlist; 	// Sorted location list
	LOCATION location;
	int min_priority=0; int min_priority_id=0; int prev_min=0;
	min_priority=0;
	prev_min=-1;
	int server_priority=0;

	int _srv_type;
	string _fname;
	string _sys;
	FILE *_ftmp;
	debug("Package has "+IntToStr(package->get_locations()->size())+" locations");
	debug("Sorting...");



	for (int x=0; x<package->get_locations()->size();x++)
	{
		for (int i=0; i<package->get_locations()->size(); i++)
		{
			server_priority=atoi(package->get_locations()->get_location(i)->get_server()->get_priority().c_str());
			if ((i==0 || server_priority<min_priority) && server_priority>prev_min)
			{
				min_priority=server_priority;
				min_priority_id=i;
			}
		}
		for (int i=0; i<package->get_locations()->size(); i++)
		{
			if (i==min_priority_id) locationlist.add(*package->get_locations()->get_location(i));
		}
		prev_min=min_priority;
	}
	debug("Sorted. We have got "+IntToStr(locationlist.size())+" locations sorted.");

#ifdef ENABLE_DEBUG
	for (int i=0; i<locationlist.size();i++)
	{
		debug("Priority: "+locationlist.get_location(i)->get_server()->get_priority());
	}
#endif
	for (int i=0; i<locationlist.size(); i++) // First, searching local file servers.
	{
		debug("Searching file sources");
		if (locationlist.get_location(i)->get_server()->get_type()==SRV_FILE)
		{
			debug("TYPE=SRV_FILE, checking file existance");
			// Checking file existance
			string _fname;
			string _sys;
			_fname=locationlist.get_location(i)->get_path()+package->get_filename();
			debug("_fname="+_fname);
			_ftmp=fopen(_fname.c_str(),"r");
			if (_ftmp)
			{
				debug("File "+_fname+" exists , proceeding next: checking md5");
				fclose(_ftmp);
				// Checking md5
				if (package->get_md5()==get_file_md5(_fname))
				{
					debug("md5 ok");
					_sys="ln -fs "+_fname+" "+SYS_CACHE;
					debug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>!!!!!!!!!!>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Creating symlink: "+_sys);
					system(_sys.c_str());
					return 0; // File successfully delivered thru symlink, we can exit function now
				}
				else
				{
					printf("md5 incorrect\n");
				}
			}
			else
			{
				debug("File not found, searching next server");
			}
		}
	} // End of searching local file servers

	string wget_sys;
	for (int i=0; i<locationlist.size(); i++) // Second: looking for all other servers ordering by priority.
	{
		_srv_type=locationlist.get_location(i)->get_server()->get_type();
		switch (_srv_type)
		{
			case SRV_CACHE:
				// Checking file existance
				_fname = SYS_CACHE + locationlist.get_location(i)->get_path() + package->get_filename();
				_ftmp=fopen(_fname.c_str(),"r");
				if (_ftmp)
				{
					debug("File "+_fname+" exists , proceeding next: checking md5");
					fclose(_ftmp);
					// Checking md5
					if (package->get_md5()==get_file_md5(_fname))
					{
						debug("md5 ok");
						return 0; // File successfully delivered thru symlink, we can exit function now
					}
					else
					{
						debug("md5 incorrect");
					}
				}
				else
				{
					fclose(_ftmp);
					debug("File not found, searching next server");
				}
				break;
			case SRV_CDROM:
				// Not implemented yet.
				debug("Fetching from CD-COM is not implemented yet");
				break;
			case SRV_HTTP:
				// Not implemented yet.
				// temporary tech: wget =)
				// TODO: usage of cache, md5 checking
				wget_sys="wget -q -O "+SYS_CACHE+package->get_filename()+" "\
					  + locationlist.get_location(i)->get_server()->get_url() \
					  + locationlist.get_location(i)->get_path() \
					  + package->get_filename();
				printf(_("Downloading package %s..."), package->get_filename().c_str());
				if (system(wget_sys.c_str())==0)
				{
					printf(_("done.\n"));
				}
				else printf(_("FAILED\n"));
				
				debug("Fetching from HTTP is half-implemented yet");
				break;
			case SRV_FTP:
				/*// temporary tech: wget =)*/
				// TODO: same as all others - cache, md5...
				
				wget_sys="wget -q -O "+SYS_CACHE+package->get_filename()+" "\
					  + locationlist.get_location(i)->get_server()->get_url() \
					  + locationlist.get_location(i)->get_path() \
					  + package->get_filename();
				printf(_("Downloading package %s..."), package->get_filename().c_str());
				if (system(wget_sys.c_str())==0)
				{
					printf(_("done.\n"));
				}
				else printf(_("FAILED\n"));
	
				// Not implemented yet.
				debug("Fetching from FTP is temporary implemented yet");
				break;
			case SRV_SMB:
				// Not implemented yet.
				debug("Fetching from SMB is not implemented yet");
				break;
			case SRV_HTTPS:
				// Not implemented yet.
				debug("Fetching from HTTPS is not implemented yet");
				break;
			default:
				debug ("Server type not recognized or not supported");
				break;
		}
	}
	// If we reach this point - this means that we cannot get package. Returning error...
	return 1;
} // End of mpkgDatabase::fetch_package();

int mpkgDatabase::install_package(PACKAGE* package)
{
//#define IDEBUG
	// First of all: EXTRACT file list and scripts!!!
	LocalPackage lp(SYS_CACHE+package->get_filename());
#ifdef IDEBUG
	printf("calling fill_scripts\n");
#endif
	bool no_purge=true;
	FILE_LIST old_config_files;
	int purge_id=get_purge(package->get_name()); // returns package id if this previous package config files are not removed, or 0 if purged.
	debug("purge_id="+IntToStr(purge_id));
	if (purge_id==0)
	{
		no_purge=false;
	}
	lp.fill_scripts(package);
	lp.fill_filelist(package);
	lp.fill_configfiles(package);
	if (check_file_conflicts(package)!=0)
	{
		printf("File conflict on package %s, it will be skipped!\n", package->get_name().c_str());
		return -5;
	}
	add_scripts_record(package->get_id(), package->get_scripts()); // Add paths to scripts to database

// Filtering file list...
	FILE_LIST package_files;
	if (!no_purge) add_filelist_record(package->get_id(), package->get_files());
	string sys;
	debug("Preparing scripts");
	if (!DO_NOT_RUN_SCRIPTS)
	{
		string preinst="/bin/sh "+package->get_scripts()->get_preinstall();
		system(preinst.c_str());
	}

	// Extracting package
	debug("calling extract");
	//lp.fill_filelist(package);
	string sys_cache=SYS_CACHE;
	string sys_root=SYS_ROOT;
	string create_root="mkdir "+sys_root+" 2>/dev/null";
	system(create_root.c_str());
	printf("Extracting package %s\n",package->get_name().c_str());
	sys="(cd "+sys_root+"; tar zxf "+sys_cache+package->get_filename()+" --exclude install";
	//If previous version isn't purged, do not overwrite config files
	if (no_purge)
	{
		debug("retrieving old config files");
		get_filelist(purge_id, &old_config_files, true);
		debug("no_purge flag IS SET, config files count = "+IntToStr(package->get_config_files()->size()));
		for (int i=0; i<package->get_config_files()->size(); i++)
		{
			// Writing new config files, skipping old
			for (int k=0; k < old_config_files.size(); k++)
			{
				if (package->get_config_files()->get_file(i)->get_name()==old_config_files.get_file(k)->get_name())
				{
					debug("excluding file "+package->get_config_files()->get_file(i)->get_name());
					sys+=" --exclude "+package->get_config_files()->get_file(i)->get_name();
				}
			}
		}
		for (int i=0; i<package->get_files()->size(); i++)
		{
			for (int k=0; k <= old_config_files.size(); k++)
			{
				if (k==old_config_files.size()) 
				{
					package_files.add(*package->get_files()->get_file(i));
					break;
				}
				if (package->get_files()->get_file(i)->get_name()==old_config_files.get_file(k)->get_name())
				{
					debug("Skipping file "+package->get_files()->get_file(i)->get_name());
					break;
				}
			}
		}
		debug("package_files size: "+IntToStr(package_files.size())+", package->get_files size: "+IntToStr(package->get_files()->size()));
		add_filelist_record(package->get_id(), &package_files);
	}
	sys+=" > /dev/null)";
	system(sys.c_str());
	
	// Creating and running POST-INSTALL script
	if (!DO_NOT_RUN_SCRIPTS)
	{
		string postinst="/bin/sh "+package->get_scripts()->get_postinstall();
		system(postinst.c_str());
	}

	set_status(package->get_id(), PKGSTATUS_INSTALLED);
	if (get_status(purge_id)==PKGSTATUS_REMOVED_AVAILABLE) set_status(purge_id, PKGSTATUS_AVAILABLE);
	if (get_status(purge_id)==PKGSTATUS_REMOVED_UNAVAILABLE) set_status(purge_id, PKGSTATUS_UNAVAILABLE);
	debug("*********************************************\n*        Package installed sussessfully     *\n*********************************************");
	return 0;
}

int mpkgDatabase::purge_package(PACKAGE* package)
{
	// purging package config files.
	printf("calling purge\n");
	string sys_cache=SYS_CACHE;
	string sys_root=SYS_ROOT;
	string fname;
	printf("Removing configuration files of package %s...\n", package->get_name().c_str());
	debug("Package has "+IntToStr(package->get_config_files()->size())+" config files");
#ifdef PURGE_METHOD_1
	debug("purging using method 1");
	FILE_LIST remove_files=*package->get_config_files(); // Try to remove only config files
#else
	debug("Purging using method 2");
	FILE_LIST remove_files=*package->get_files(); // Try to remove ALL files
#endif

	printf("remove_files size = %d\n", remove_files.size());

	for (int i=0; i<remove_files.size(); i++)
	{
		fname=sys_root + remove_files.get_file(i)->get_name(false);
		
		if (fname[fname.length()-1]!='/')
		{
			if (unlink (fname.c_str())!=0)
			{
				printf("Cannot delete file %s\n", fname.c_str());
			}
		}
	}

	// Run 2: clearing empty directories
	vector<string>empty_dirs;
	string edir;
	for (int i=0; i<remove_files.size(); i++)
	{
		fname=sys_root + remove_files.get_file(i)->get_name(false);
		for (unsigned int d=0; d<fname.length(); d++)
		{
			edir+=fname[d];
			if (fname[d]=='/')
			{
				empty_dirs.resize(empty_dirs.size()+1);
				empty_dirs[empty_dirs.size()-1]=edir;
			}
		}

		for (int x=empty_dirs.size()-1;x>=0; x--)
		{
			rmdir(empty_dirs[x].c_str());
		}
		edir.clear();
		empty_dirs.clear();
	}

	printf("done\n");
	//set_purge(package->get_id());
	if (package->get_locations()->IsEmpty()) set_status(package->get_id(), PKGSTATUS_UNAVAILABLE);
	else set_status(package->get_id(), PKGSTATUS_AVAILABLE);
	cleanFileList(package->get_id());

	debug("*********************************************\n*        Package purged sussessfully     *\n*********************************************");
	return 0;
}



int mpkgDatabase::remove_package(PACKAGE* package)
{
	if (package->get_status()==PKGSTATUS_REMOVE || package->get_status()==PKGSTATUS_REMOVE_PURGE)
	{
		// Running pre-remove scripts
		debug("REMOVE PACKAGE::Preparing scripts");
		if(!DO_NOT_RUN_SCRIPTS)
		{
			string prerem="/bin/sh "+package->get_scripts()->get_preremove();
			system(prerem.c_str());
		}
		// removing package
		debug("calling remove");
		string sys_cache=SYS_CACHE;
		string sys_root=SYS_ROOT;
		string fname;
		printf("Removing package %s...",package->get_name().c_str());
		debug("Package has "+IntToStr(package->get_files()->size())+" files");

		// purge will be implemented in mpkgDatabase::purge_package(PACKAGE *package); so we skip config files here
		FILE_LIST remove_files=*package->get_files();
		for (int i=0; i<remove_files.size(); i++)
		{
			fname=sys_root + remove_files.get_file(i)->get_name(false);
			if (remove_files.get_file(i)->get_type()==FTYPE_PLAIN && fname[fname.length()-1]!='/')
			{
				if (unlink (fname.c_str())!=0)
				{
					printf("Cannot delete file %s: ", fname.c_str());
					perror("Reason: ");
				}
			}
		}
	
		// Run 2: clearing empty directories
		vector<string>empty_dirs;
		string edir;
		for (int i=0; i<remove_files.size(); i++)
		{
			fname=sys_root + remove_files.get_file(i)->get_name(false);
			for (unsigned int d=0; d<fname.length(); d++)
			{
				edir+=fname[d];
				if (fname[d]=='/')
				{
					empty_dirs.resize(empty_dirs.size()+1);
					empty_dirs[empty_dirs.size()-1]=edir;
				}
			}
	
			for (int x=empty_dirs.size()-1;x>=0; x--)
			{
				rmdir(empty_dirs[x].c_str());
			}
			edir.clear();
			empty_dirs.clear();
		}
	
		// Creating and running POST-INSTALL script
		if (!DO_NOT_RUN_SCRIPTS)
		{
			string postrem="/bin/sh "+package->get_scripts()->get_postremove();
			system(postrem.c_str());
		}
	}
	if (package->get_locations()->IsEmpty()) set_status(package->get_id(), PKGSTATUS_REMOVED_UNAVAILABLE);
	else set_status(package->get_id(), PKGSTATUS_REMOVED_AVAILABLE);
	cleanFileList(package->get_id());
	printf("done\n");
	debug("*********************************************\n*        Package removed sussessfully     *\n*********************************************");
	return 0;
}

int mpkgDatabase::cleanFileList(int package_id)
{
	printf("Cleaning up...\n");
	int status=get_status(package_id);
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", IntToStr(package_id));
	if (status==PKGSTATUS_REMOVED_AVAILABLE || status==PKGSTATUS_REMOVED_UNAVAILABLE) sqlSearch.addField("file_type", IntToStr(FTYPE_PLAIN));
	return db.sql_delete("files", sqlSearch);
}

int mpkgDatabase::update_package_data(int package_id, PACKAGE *package)
{
	// TODO: NEED OPTIMIZATIONS!!
	PACKAGE old_package;
	if (get_package(package_id, &old_package)!=0)
	{
		debug("mpkg.cpp: update_package_data(): get_package error: no package or error while querying database");
		return -1;
	}
	
	SQLRecord sqlUpdate;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));

	debug("mpkg.cpp: update_package_data(): updating direct package data");
	// 1. Updating direct package data
	if (package->get_md5()!=old_package.get_md5())
	{
		debug("mpkg.cpp: update_package_data(): md5 mismatch, updating description fields");
		sqlUpdate.addField("package_description", package->get_description());
		sqlUpdate.addField("package_short_description", package->get_short_description());
		sqlUpdate.addField("package_compressed_size", package->get_compressed_size());
		sqlUpdate.addField("package_installed_size", package->get_installed_size());
		sqlUpdate.addField("package_changelog", package->get_changelog());
		sqlUpdate.addField("package_packager", package->get_packager());
		sqlUpdate.addField("package_packager_email", package->get_packager_email());
		sqlUpdate.addField("package_md5", package->get_md5());
	}

	// 2. Updating filename
	if (package->get_filename()!=old_package.get_filename())
	{
		debug("mpkg.cpp: update_package_data(): filename mismatch, updating");
		sqlUpdate.addField("package_filename", package->get_filename());
	}

	// 3. Updating status
	if (package->get_status()!=old_package.get_status())
	{
		debug("mpkg.cpp: update_package_data(): status mismatch, updating");
		sqlUpdate.addField("package_status", IntToStr(package->get_status()));
	}

	// 4. Updating locations
	
	// Note about method used for updating locations:
	// If locations are not identical:
	// 	Step 1. Remove all existing package locations from "locations" table. Servers are untouched.
	// 	Step 2. Add new locations.
	// Note: after end of updating procedure for all packages, it will be good to do servers cleanup - delete all servers who has no locations.
	debug("mpkg.cpp: update_package_data(): checking locations");	
	if (*package->get_locations()!=*old_package.get_locations())
	{
		debug("mpkg.cpp: update_package_data(): locations mismatch, cleanup");
		debug("mpkg.cpp: update_package_data(): old has "+IntToStr(old_package.get_locations()->size())+" locations, but new has "+\
				IntToStr(package->get_locations()->size())+" ones");
		SQLRecord loc_sqlDelete;
		loc_sqlDelete.addField("packages_package_id", IntToStr(package_id));
		debug("mpkg.cpp: update_package_data(): deleting old locations relating this package");
		int sql_del=db.sql_delete("locations", loc_sqlDelete);
		if (sql_del!=0)
		{
			debug("mpkg.cpp: update_package_data(): unable to delete old locations: SQL error "+IntToStr(sql_del));
			return -2;
		}
		if (add_locationlist_record(package_id, package->get_locations())!=0)
		{
			debug("mpkg.cpp: update_package_data(): unable to add new locations: error in add_locationlist_record()");
			return -3;
		}
		if (old_package.get_status()!=PKGSTATUS_INSTALLED && package->get_locations()->IsEmpty())
		{
			set_status(old_package.get_id(), PKGSTATUS_UNAVAILABLE);
		}
	}

	// 5. Updating tags
	if (*package->get_tags()!=*old_package.get_tags())
	{
		SQLRecord taglink_sqlDelete;
		taglink_sqlDelete.addField("packages_package_id", IntToStr(package_id));

		if (db.sql_delete("tags_links", taglink_sqlDelete)!=0) return -4;
		if (add_taglist_record(package_id, package->get_tags())!=0) return -5;
	}

	// 6. Updating dependencies
	if (*package->get_dependencies()!=*old_package.get_dependencies())
	{
		SQLRecord dep_sqlDelete;
		dep_sqlDelete.addField("packages_package_id", IntToStr(package_id));

		if(db.sql_delete("dependencies", dep_sqlDelete)!=0) return -6;
		if (add_dependencylist_record(package_id, package->get_dependencies())!=0) return -7;
	}

	// 7, 8 - update scripts and file list. It is skipped for now, because we don't need this here (at least, for now).
	if (!sqlUpdate.empty())
	{
		if (db.sql_update("packages", sqlUpdate, sqlSearch)!=0)
		{
			debug("mpkg.cpp: update_package_data(): db.sql_update failed");
			return -8;
		}
	}
	else debug("mpkg.cpp: update_package_data(): sqlUpdate empty, nothing to update");
	debug("mpkg.cpp: update_package_data(): successful end");
	return 0;
}




int mpkgDatabase::updateRepositoryData(PACKAGE_LIST *newPackages)
{
	//printf("mpkg.cpp: updateRepositoryData(): begin\n");
	//printf(_("Got data about %d new packages, importing\n"), newPackages->size());
	debug("mpkg.cpp: updateRepositoryData(): requesting current packages");
	PACKAGE_LIST currentPackages;
	SQLRecord sqlSearch;
	PACKAGE tmpPackage;
	if (get_packagelist(sqlSearch, &currentPackages)!=0)
	{
		debug("mpkg.cpp: updateRepositoryData(): failed to get list of current packages");
		return -1;
	}

	debug("mpkg.cpp: updateRepositoryData(): Step 1. Adding new data, updating old");
	int package_id;
	int package_status;
	for (int i=0; i< newPackages->size(); i++)
	{
		//printf("for (int i=0; i< newPackages->size(); i++) i=%d\n", i);
		//printf("#");

		package_id=get_package_id(newPackages->get_package(i));
		if (package_id>0)
		{
			debug("mpkg.cpp: updateRepositoryData(): Such package found, updating data if needed");
			package_status=get_status(package_id);
			debug("mpkg.cpp: updateRepositoryData(): package status: "+IntToStr(package_status));
			if (package_status!=PKGSTATUS_UNAVAILABLE)
				newPackages->get_package(i)->set_status(package_status);
			//printf("update_package_data call\n");
			update_package_data(package_id, newPackages->get_package(i));
		}
		if (package_id==0)
		{
			debug("mpkg.cpp: updateRepositoryData(): new package, calling add_package_record()"); 
			add_package_record(newPackages->get_package(i));
		}
		if (package_id<0)
		{
			debug("mpkg.cpp: updateRepositoryData(): query error");
			return -1;
		}
	}
	
	//printf("mpkg.cpp: updateRepositoryData(): Step 2. Clean up old package data (packages, that are no sources for now)\n");
	int package_num;
	for (int i=0; i<currentPackages.size(); i++)
	{
		if (currentPackages.get_package(i)->get_status()==PKGSTATUS_INSTALL \
				|| currentPackages.get_package(i)->get_status()==PKGSTATUS_AVAILABLE \
				|| currentPackages.get_package(i)->get_status()==PKGSTATUS_REMOVED_AVAILABLE)
		{
			package_num=0;
			for (int s=0; s<newPackages->size(); s++)
			{
				if (currentPackages.get_package(i)->get_name()==newPackages->get_package(s)->get_name() \
						&& currentPackages.get_package(i)->get_version()==newPackages->get_package(s)->get_version() \
						&& currentPackages.get_package(i)->get_arch()==newPackages->get_package(s)->get_arch() \
						&& currentPackages.get_package(i)->get_build()==newPackages->get_package(s)->get_build())
				{
					debug("mpkg.cpp: updateRepositoryData(): package_num="+IntToStr(s)+", found - skipping");
					package_num=s+1;
					break;
				}
			}
			if (package_num==0)
			{
				debug("mpkg.cpp: updateRepositoryData(): package_num=0, clearing package with id "+\
						IntToStr(currentPackages.get_package(i)->get_id()));
				currentPackages.get_package(i)->get_locations()->clear();
				
				if (currentPackages.get_package(i)->get_status()==PKGSTATUS_REMOVED_AVAILABLE)
					currentPackages.get_package(i)->set_status(PKGSTATUS_REMOVED_UNAVAILABLE);
				else
					currentPackages.get_package(i)->set_status(PKGSTATUS_UNAVAILABLE);

				update_package_data(currentPackages.get_package(i)->get_id(), currentPackages.get_package(i));
			}
		}
	}
	//printf("\n");

	// Step 3. Clean up servers and tags (remove unused items)
	// TODO
	//printf("mpkg.cpp: updateRepositoryData(): successful end\n");
	return 0;
}

