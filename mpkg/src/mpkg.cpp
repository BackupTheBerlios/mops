/***********************************************************************
 * 	$Id: mpkg.cpp,v 1.13 2006/12/29 20:56:18 i27249 Exp $
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
		debug ("Package is new, adding to database");
		add_package_record(package);
		return 0;
	}
	if (pkg_id<0)
	{
		debug("Database error, cannot emerge");
		return 1;
	}
	// Раз пакет уже в базе (и в единственном числе - а иначе и быть не должно), сравниваем данные.
	// В случае необходимости, добавляем location.
	debug ("Package is already in database, updating locations if needed");
	PACKAGE db_package;
	LOCATION_LIST new_locations;
	get_package(pkg_id, &db_package, true);
	package->set_id(pkg_id);

	for (int j=0; j<package->get_locations()->size(); j++)
	{
		debug("J cycle");
		for (int i=0; i<db_package.get_locations()->size(); i++)
		{
			debug("I cycle");
			if (package->get_locations()->get_location(j)->get_server()!=db_package.get_locations()->get_location(i)->get_server() || \
					package->get_locations()->get_location(j)->get_path()!=db_package.get_locations()->get_location(i)->get_path())
			{
				debug("----------------->new location<--------------------");
				new_locations.add(*package->get_locations()->get_location(j));
			}
		}
	}
	if (!new_locations.IsEmpty()) add_locationlist_record(pkg_id, &new_locations);
	else debug ("no new locations");
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

void mpkgDatabase::commit_actions()
{
	// First: removing required packages
	PACKAGE_LIST remove_list;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_status", IntToStr(PKGSTATUS_REMOVE));
	get_packagelist(sqlSearch, &remove_list);
	debug ("Calling REMOVE for "+IntToStr(remove_list.size())+" packages");
	for (int i=0;i<remove_list.size();i++)
	{
		remove_package(remove_list.get_package(i));
	}


	// Second: installing required packages
	PACKAGE_LIST install_list;
	sqlSearch.setValue("package_status", IntToStr(PKGSTATUS_INSTALL));
	get_packagelist(sqlSearch, &install_list);
	debug("Calling FETCH");
	debug("Preparing to fetch "+IntToStr(install_list.size())+" packages");

	for (int i=0; i<install_list.size(); i++)
	{
		debug("Fetching package #"+IntToStr(i+1)+" with "+IntToStr(install_list.get_package(i)->get_locations()->size())+" locations");
		fetch_package(install_list.get_package(i));
	}
	debug("Calling INSTALL");
	for (int i=0;i<install_list.size();i++)
	{
		install_package(install_list.get_package(i));
	}
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
	int min_priority; int min_priority_id; int prev_min;
	min_priority=0;
	prev_min=-1;
	int server_priority;

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
				wget_sys="wget -O "+SYS_CACHE+package->get_filename()+" "\
					  + locationlist.get_location(i)->get_server()->get_url() \
					  + locationlist.get_location(i)->get_path() \
					  + package->get_filename();

				system(wget_sys.c_str());
				
				debug("Fetching from HTTP is half-implemented yet");
				break;
			case SRV_FTP:
				/*// temporary tech: wget =)
				string wget_sys;
				wget_sys="( cd "+SYS_CACHE+"; wget "+locationlist.get_location(i)->get_server()->get_url() + \
					  locationlist.get_location(i)->get_path() + \
					  package->get_filename() + \
					  " )";
				system(wget_sys.c_str());*/
				// Not implemented yet.
				debug("Fetching from FTP is not implemented yet");
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
	string sys;
	debug("Preparing scripts");
	string tmp_preinstall=get_tmp_file();
	string tmp_postinstall=get_tmp_file();

	string sys_preinstall="/bin/sh "+tmp_preinstall;
	string sys_postinstall="/bin/sh "+tmp_postinstall;
	FILE* f_preinstall;
	FILE* f_postinstall;

	// Creating and running PRE-INSTALL script
	f_preinstall=fopen(tmp_preinstall.c_str(), "w");
	if (f_preinstall)
	{
		for (int i=0;i<package->get_scripts()->get_preinstall(false).length();i++)
		{
			fputc(package->get_scripts()->get_preinstall(false)[i], f_preinstall);
		}
		fclose(f_preinstall);
		if (!DO_NOT_RUN_SCRIPTS)
		{
			system(sys_preinstall.c_str());
		}
		
	}
	else
	{
		printf("unable to write pre-install script: check permissions and disk space\n");
		fclose(f_preinstall);
	}

	// Extracting package
	debug("calling extract");
	string sys_cache=SYS_CACHE;
	string sys_root=SYS_ROOT;
	string create_root="mkdir "+sys_root;
	system(create_root.c_str());
	printf("Extracting package %s\n",package->get_name().c_str());
	sys="(cd "+sys_root+"; tar zxf "+sys_cache+package->get_filename()+" --exclude install > /dev/null)";
	system(sys.c_str());

	// Creating and running POST-INSTALL script
	f_postinstall=fopen(tmp_postinstall.c_str(), "w");
	if (f_postinstall)
	{
		for (int i=0;i<package->get_scripts()->get_postinstall(false).length();i++)
		{
			fputc(package->get_scripts()->get_postinstall(false)[i], f_postinstall);
		}
		fclose(f_postinstall);
		if (!DO_NOT_RUN_SCRIPTS)
		{
	//		system(sys_postinstall.c_str());
			runShellCommand(sys_postinstall);
		}
	}
	else
	{
		printf("unable to write post-install script: check permissions and disk space\n");
		fclose(f_postinstall);
	}

	set_status(package->get_id(), PKGSTATUS_INSTALLED);
	debug("*********************************************\n*        Package installed sussessfully     *\n*********************************************");
}

int mpkgDatabase::remove_package(PACKAGE* package)
{
	// Running pre-remove scripts
	debug("REMOVE PACKAGE::Preparing scripts");
	string tmp_preremove=get_tmp_file();
	string tmp_postremove=get_tmp_file();

	string sys_preremove="/bin/sh "+tmp_preremove;
	string sys_postremove="/bin/sh "+tmp_postremove;
	FILE* f_preremove;
	FILE* f_postremove;

	// Creating and running PRE-INSTALL script
	f_preremove=fopen(tmp_preremove.c_str(), "w");
	if (f_preremove)
	{
		for (int i=0;i<package->get_scripts()->get_preremove(false).length();i++)
		{
			fputc(package->get_scripts()->get_preremove(false)[i], f_preremove);
		}
		fclose(f_preremove);
		if(!DO_NOT_RUN_SCRIPTS)
		{

	//		system(sys_preremove.c_str());
			runShellCommand(sys_preremove);
		}
		
	}
	else
	{
		printf("unable to write pre-remove script: check permissions and disk space\n");
		fclose(f_preremove);
	}

	// removing package
	debug("calling remove");
	string sys_cache=SYS_CACHE;
	string sys_root=SYS_ROOT;
	string fname;
	printf("Removing package %s\n",package->get_name().c_str());
	int gauss=0;
	int rm_ret;
	int dir_depth;
	double step=package->get_files()->size()/70;
	debug("Package has "+IntToStr(package->get_files()->size())+" files");

	for (int i=0; i<package->get_files()->size(); i++)
	{
		fname=sys_root + package->get_files()->get_file(i)->get_name(false);
		if (fname[fname.length()-1]=='/')
		{
			rm_ret=rmdir(fname.c_str());
			if (rm_ret!=0)
			{
				//if (rm_ret!=ENOTEMPTY) debug ("failed to delete directory");
			}
		}
		else
		{
			if (unlink (fname.c_str())!=0)
			{
				//printf("Cannot delete file %s\n", fname.c_str());
			}
		}
		if ((i-gauss*step)/step > 1)
		{
			printf(">");
			gauss++;
		}
	}
	printf("\n");

	// Run 2: clearing empty directories
	vector<string>empty_dirs;
	string edir;
	for (int i=0; i<package->get_files()->size(); i++)
	{
		fname=sys_root + package->get_files()->get_file(i)->get_name(false);
		for (int d=0; d<fname.length(); d++)
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
	f_postremove=fopen(tmp_postremove.c_str(), "w");
	if (f_postremove)
	{
		for (int i=0;i<package->get_scripts()->get_postremove(false).length();i++)
		{
			fputc(package->get_scripts()->get_postremove(false)[i], f_postremove);
		}
		fclose(f_postremove);
		if (!DO_NOT_RUN_SCRIPTS)
		{
			system(sys_postremove.c_str());
		}
	}
	else
	{
		printf("unable to write post-remove script: check permissions and disk space\n");
		fclose(f_postremove);
	}

	set_status(package->get_id(), PKGSTATUS_AVAILABLE);
	debug("*********************************************\n*        Package removed sussessfully     *\n*********************************************");

	
}

int mpkgDatabase::update_package_data(int package_id, PACKAGE *package)
{
	PACKAGE old_package;
	if (get_package(package_id, &old_package)!=0)
		return -1;
	
	SQLRecord sqlUpdate;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", IntToStr(package_id));

	// 1. Updating direct package data
	if (package->get_md5()!=old_package.get_md5())
	{
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
		sqlUpdate.addField("package_filename", package->get_filename());
	}

	// 3. Updating status
	if (package->get_status()!=old_package.get_status())
	{
		sqlUpdate.addField("package_status", IntToStr(package->get_status()));
	}

	// 4. Updating locations
	
	// Note about method used for updating locations:
	// If locations are not identical:
	// 	Step 1. Remove all existing package locations from "locations" table. Servers are untouched.
	// 	Step 2. Add new locations.
	// Note: after end of updating procedure for all packages, it will be good to do servers cleanup - delete all servers who has no locations.
	
	if (*package->get_locations()!=*old_package.get_locations())
	{
		SQLRecord loc_sqlDelete;
		loc_sqlDelete.addField("packages_package_id", IntToStr(package_id));

		if (db.sql_delete("locations", loc_sqlDelete)!=0) return -2;
		if (add_locationlist_record(package_id, package->get_locations())!=0) return -3;
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
	
	if (db.sql_update("packages", sqlUpdate, sqlSearch)!=0) return -8;
	return 0;
}




int mpkgDatabase::updateRepositoryData(PACKAGE_LIST *newPackages)
{
	printf(_("Got data about %d new packages, importing\n"), newPackages->size());
	PACKAGE_LIST currentPackages;
	SQLRecord sqlSearch;
	PACKAGE tmpPackage;
	if (get_packagelist(sqlSearch, &currentPackages)!=0)
		return -1;

	// Step 1. Adding new data, updating old
	int package_id;
	int package_status;
	for (int i=0; i< newPackages->size(); i++)
	{
		package_id=get_package_id(newPackages->get_package(i));
		if (package_id>0)
		{
			// Such package found, updating data if needed
			package_status=get_status(package_id);
			if (package_status!=PKGSTATUS_UNAVAILABLE)
				newPackages->get_package(i)->set_status(package_status);
			update_package_data(package_id, newPackages->get_package(i));
		}
		if (package_id==0)
		{
			add_package_record(newPackages->get_package(i));
		}
		if (package_id<0)
			return -1;
	}
	
	// Step 2. Clean up old package data
	int package_num;
	for (int i=0; i<currentPackages.size(); i++)
	{
		if (currentPackages.get_package(i)->get_status()==PKGSTATUS_INSTALL \
				|| currentPackages.get_package(i)->get_status()==PKGSTATUS_AVAILABLE)
		{
			package_num=0;
			for (int s=0; s<newPackages->size(); s++)
			{
				if (currentPackages.get_package(i)->get_name()==newPackages->get_package(i)->get_name() \
						&& currentPackages.get_package(i)->get_version()==newPackages->get_package(i)->get_version() \
						&& currentPackages.get_package(i)->get_arch()==newPackages->get_package(i)->get_arch() \
						&& currentPackages.get_package(i)->get_build()==newPackages->get_package(i)->get_build())
				{
					package_num=s;
					break;
				}
			}
			if (package_num==0)
			{
				currentPackages.get_package(i)->get_locations()->clear();
				currentPackages.get_package(i)->set_status(PKGSTATUS_UNAVAILABLE);
				update_package_data(currentPackages.get_package(i)->get_id(), currentPackages.get_package(i));
			}
		}
	}

	// Step 3. Clean up servers and tags (remove unused items)
	// TODO
}

