/*********************************************************
 * MOPSLinux packaging system: general functions
 * $Id: mpkgsys.cpp,v 1.20 2007/05/03 11:38:44 i27249 Exp $
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
	printf("requestInstall\n");
	PACKAGE tmpPackage;
	int ret = db->get_package(package_id, &tmpPackage);
	if (ret == 0)
	{
		if (tmpPackage.installed())
		{
			printf("Package %s %s cannot be installed, because it is already installed.\n", tmpPackage.get_name().c_str(), tmpPackage.get_fullversion().c_str());
		}
		if (!tmpPackage.available())
		{
			printf("Package %s %s cannot be installed, because it is unavailable\n", tmpPackage.get_name().c_str(), tmpPackage.get_fullversion().c_str());
		}
		if (tmpPackage.available() && !tmpPackage.installed())
		{
			tmpPackage.set_action(ST_INSTALL);
			DepTracker->addToInstallQuery(&tmpPackage);
			return tmpPackage.get_id();
		}
		else
		{
			printf("Impossible\n");
			return MPKGERROR_IMPOSSIBLE;
		}
	}
	else
	{
		printf("requestInstall: get_package error: returned %d\n", ret);
		return ret;
	}
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
	bool process=false;
	if (ret == 0)
	{
		if (tmpPackage.configexist())
		{
			if (purge)
			{
				tmpPackage.set_action(ST_PURGE);
				process=true;
			}
			else if (tmpPackage.installed())
			{
				tmpPackage.set_action(ST_REMOVE);
				process=true;
			}
		}
		if (process)
		{
			printf("Processing...\n");
			DepTracker->addToRemoveQuery(&tmpPackage);
			return tmpPackage.get_id();
		}
		else
		{
			if (purge) printf("Package %s %s cannot be purged, because it is already purged\n", tmpPackage.get_name().c_str(), tmpPackage.get_fullversion().c_str());
			else printf("Package %s %s cannot be uninstalled, because it is already removed\n", tmpPackage.get_name().c_str(), tmpPackage.get_fullversion().c_str());
			return MPKGERROR_IMPOSSIBLE;
		}
	}
	else
	{
		printf("get_package returned %d\n", ret);
		return ret;
	}
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


