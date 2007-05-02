/*********************************************************
 * MOPSLinux packaging system: general functions
 * $Id: mpkgsys.cpp,v 1.18 2007/05/02 12:27:15 i27249 Exp $
 * ******************************************************/

#include "mpkgsys.h"

string output_dir;

// Cleans system cache
int mpkgSys::clean_cache()
{
	ftw(SYS_CACHE.c_str(), _clean, 100);
	return 0;
}

int mpkgSys::clean_queue(mpkgDatabase *db)
{
	PACKAGE_LIST toInstall;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", IntToStr(ST_INSTALL));
	sqlSearch.addField("package_action", IntToStr(ST_REMOVE));
	sqlSearch.addField("package_action", IntToStr(ST_PURGE));
	db->get_packagelist(sqlSearch, &toInstall);
	for (int i=0; i<toInstall.size();i++)
	{
		db->set_action(toInstall.get_package(i)->get_id(), ST_NONE);
	}
	return 0;
}

int mpkgSys::unqueue(int package_id, mpkgDatabase *db)
{
	db->set_action(package_id, ST_NONE);
	return 0;
}


int mpkgSys::build_package()
{
    printf("building package...\n");
    if (FileNotEmpty("install/data.xml"))
    {
	    PackageConfig p("install/data.xml");
	    if (!p.parseOk) return -100;
	    string pkgname;
	    string sysline;
	    pkgname=p.getName()+"-"+p.getVersion()+"-"+p.getArch()+"-"+p.getBuild();
	    printf("Creating package %s\n", pkgname.c_str());
#ifdef APPLE_DEFINED
	    sysline = "tar czf "+pkgname+".tgz *";
#else
	    sysline="makepkg -l y -c n "+pkgname+".tgz";
#endif
	    system(sysline.c_str());
    }
    return 0;
}

int mpkgSys::update_repository_data(mpkgDatabase *db, DependencyTracker *DepTracker)
{
	// Функция, с которой начинается обновление данных.
	
	Repository rep;		// Объект репозиториев
	PACKAGE_LIST availablePackages;		// Список пакетов, полученных из всех репозиториев...
	PACKAGE_LIST tmpPackages;		// Список пакетов, полученных из текущего репозитория (временное хранилище)

	// А есть ли у нас вообще репозитории? Может нам и ловить-то нечего?...
	// Впрочем, надо все равно пойти на принцип и пометить все пакеты как недоступные. Ибо это действительно так.
	// Поэтому - проверка устранена.

	printf(_("Updating package data from %d repositories...\n"), REPOSITORY_LIST.size());
	
	int total_packages=0; // Счетчик полученных пакетов.

	// Поехали! Запрашиваем каждый репозиторий через функцию get_index()
	for (unsigned int i=0; i<REPOSITORY_LIST.size(); i++)
	{
		tmpPackages.clear();					//Очищаем временный список.
		rep.get_index(REPOSITORY_LIST[i], &tmpPackages);	// Получаем список пакетов.
		
		if (!tmpPackages.IsEmpty())				// Если мы таки получили что-то, добавляем это в список.
		{
			total_packages+=tmpPackages.size();		// Увеличим счетчик
			availablePackages.add_list(&tmpPackages);	// Прибавляем данные к общему списку.
		}
	}
	printf("Total %d packages received, filtering...\n", total_packages);
	// Вот тут-то и начинается самое главное. Вызываем фильтрацию пакетов (действие будет происходить в функции updateRepositoryData.
	int ret=db->updateRepositoryData(&availablePackages);
	printf("Update complete.\n");
	return ret;
}

// Установка (обновление)
// Делаются следующие проверки:
// 1) Проверяется есть ли такой пакет в базе
// 2) Проверяется его доступность
// Если все ок, направляем в DepTracker
	
int mpkgSys::requestInstall(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	PACKAGE tmpPackage;
	int ret = db->get_package(package_id, &tmpPackage);
	if (ret == 0)
	{
		if (tmpPackage.available())
		{
			tmpPackage.set_action(ST_INSTALL);
			DepTracker->addToInstallQuery(&tmpPackage);
			return tmpPackage.get_id();
		}
		else return MPKGERROR_IMPOSSIBLE;
	}
	else return ret;
}

int mpkgSys::requestInstall(PACKAGE *package, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	return requestInstall(package->get_id(), db, DepTracker);
}


int mpkgSys::requestInstall(string package_name, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", package_name);
	PACKAGE_LIST candidates;
	int ret = db->get_packagelist(sqlSearch, &candidates, false);
	int id=-1;
	if (ret == 0)
	{
		id = candidates.getMaxVersionID(package_name);
		if (id>=0)
		{
			return requestInstall(id, db, DepTracker);
		}
		else return MPKGERROR_NOPACKAGE;
	}
	else return ret;
}	
// Удаление
int mpkgSys::requestUninstall(PACKAGE *package, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge)
{
	return requestUninstall(package->get_id(), db, DepTracker, purge);
}

int mpkgSys::requestUninstall(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge)
{
	PACKAGE tmpPackage;
	int ret = db->get_package(package_id, &tmpPackage);
	if (ret == 0)
	{
		if (tmpPackage.installed())
		{
			if (purge) tmpPackage.set_action(ST_PURGE);
			else tmpPackage.set_action(ST_REMOVE);
			DepTracker->addToRemoveQuery(&tmpPackage);
			return tmpPackage.get_id();
		}
		else return MPKGERROR_IMPOSSIBLE;
	}
	else return ret;
}
int mpkgSys::requestUninstall(string package_name, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", package_name);
	sqlSearch.addField("package_installed", "1");
	PACKAGE_LIST candidates;
	int ret = db->get_packagelist(sqlSearch, &candidates, false);
	int id=-1;
	if (ret == 0)
	{
		if (candidates.size()>1) return MPKGERROR_AMBIGUITY;
		if (candidates.IsEmpty()) return MPKGERROR_NOPACKAGE;
		id = candidates.get_package(0)->get_id();
		if (id>=0)
		{
			return requestUninstall(id, db, DepTracker, purge);
		}
		else return MPKGERROR_NOPACKAGE;
	}
	else return ret;
}













#ifdef OLD_INSTALL_SYSTEM
int mpkgSys::install(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	PACKAGE tmp_package;
	if (db->get_package(package_id, &tmp_package, false)!=0)
	{
		printf("No such package\n");
		return -1;
	}
	DepTracker->merge(&tmp_package);
	return 0;
}

	

int mpkgSys::install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker, bool do_upgrade)
{
	// Step 1. Checking if it is just a name of a package
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", fname);
	PACKAGE_LIST candidates;
	PACKAGE tmp_pkg;
	db->get_packagelist(sqlSearch, &candidates);
	debug("candidates size = "+IntToStr(candidates.size()));
	bool alreadyInstalled=false;
	if (candidates.size()>0)
	{
		for (int i=0; i<candidates.size(); i++)
		{
			debug("Candidate #"+IntToStr(i)+": "+candidates.get_package(i)->get_name()+" with ID "+IntToStr(candidates.get_package(i)->get_id())+" and status "+ \
					candidates.get_package(i)->get_vstatus());
			if (!do_upgrade && candidates.get_package(i)->installed())
			{
				alreadyInstalled=true;
				printf(_("Package is already installed (ver. %s). For upgrade, choose upgrade option\n"), candidates.get_package(i)->get_fullversion().c_str());
				break;
			}
			if (candidates.get_package(i)->available())
			{
				debug("Status passed to installation");
				if (tmp_pkg.IsEmpty() || strverscmp(tmp_pkg.get_fullversion().c_str(), \
						       	candidates.get_package(i)->get_fullversion().c_str())<0)
				{
					debug("tmp stored successfully");
					tmp_pkg=*candidates.get_package(i);
				}
			}
		}
		if (alreadyInstalled)
		{
			printf("already installed\n");
			return 0;
		}
		if (tmp_pkg.IsEmpty())
		{
			printf("No suitable package found to install\n");
			return -1;
		}
		DepTracker->merge(&tmp_pkg);
		return 0;
	}
	else printf("no candidates");
	return -1;
}

int mpkgSys::uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker, int do_purge, bool do_upgrade)
{
	if (do_purge==0) printf(_("You are going to uninstall package %s\n"), pkg_name.c_str());
	if (do_purge==1) printf(_("You are going to purge package %s\n"), pkg_name.c_str());
	
	PACKAGE package=db->get_installed_package(pkg_name);
	if (package.IsEmpty())
	{
		debug("Not installed package");
		int id;
		if (do_purge==1) id=db->get_purge(pkg_name);
		else id=0;
		if (id==0)
		{
			if (do_purge) printf(_("Package %s is already purged\n"), pkg_name.c_str());
			else printf(_("Package %s is not installed\n"), pkg_name.c_str());
			return 0;
		}
		if (id<0)
		{
			printf("Internal error while calling get_purge(): error code = %d\n", id);
			return id;
		}
		printf("setting status to purge for ID %d\n", id);
		db->set_action(id, ST_PURGE);
		return 0;
	}
	DepTracker->unmerge(&package, do_purge, do_upgrade);
	return 0;
}

int mpkgSys::upgrade (string pkgname, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	printf("Upgrade: Calling uninstall\n");
	uninstall(pkgname, db, DepTracker, 0, true);
	printf("Upgrade: Calling install\n");
	install(pkgname, db, DepTracker, true);
	printf("Upgrade: normalizing\n");
	DepTracker->normalize();
	printf("Upgrade: committing deptracker\n");
	DepTracker->commitToDb();
	printf("Upgrade: committing actions\n");
	db->commit_actions();
	return 0;
}

int mpkgSys::upgrade(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	PACKAGE tmp_package;
	if (db->get_package(package_id, &tmp_package, false)!=0)
	{
		printf("No such package\n");
		return -1;
	}
	uninstall(tmp_package.get_name(), db, DepTracker, 0, true);
	install(package_id, db, DepTracker);
	DepTracker->normalize();
	DepTracker->commitToDb();
	db->commit_actions();
	return 0;
}
#endif // OLD_INSTALL_SYSTEM ---------------------------------------


int mpkgSys::_clean(const char *filename, const struct stat *file_status, int filetype)
{
	switch(filetype)
	{
		case FTW_F:
			unlink(filename);
			break;
		case FTW_D:
			rmdir(filename);
			break;
		default:
			unlink(filename);
	}
	return 0;
}

int mpkgSys::_conv_dir(const char *filename, const struct stat *file_status, int filetype)
{
	string _package=filename;
       	string ext;
	for (unsigned int i=_package.length()-4;i<_package.length();i++)
	{
		ext+=_package[i];
	}

	if (filetype==FTW_F && ext==".tgz")
	{
		convert_package(_package, output_dir);
	}
	return 0;

}


int mpkgSys::convert_directory(string out_dir)
{
	output_dir=out_dir;
	ftw("./", _conv_dir, 100);
	return 0;
}


