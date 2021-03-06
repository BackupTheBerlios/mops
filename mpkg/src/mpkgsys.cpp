/*********************************************************
 * MOPSLinux packaging system: general functions
 * $Id: mpkgsys.cpp,v 1.62 2007/12/11 11:52:59 i27249 Exp $
 * ******************************************************/

#include "mpkgsys.h"
#include "package.h"
string output_dir;

// Cleans system cache
int mpkgSys::clean_cache(bool symlinks_only)
{
	if (!symlinks_only && !dialogMode) say(_("Cleaning package cache\n"));
	if (!symlinks_only) ftw(SYS_CACHE.c_str(), _clean, 50);
	else ftw(SYS_CACHE.c_str(), _clean_symlinks, 50);
	return 0;
}

int mpkgSys::clean_queue(mpkgDatabase *db)
{
	PACKAGE_LIST toInstall;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", ST_INSTALL);
	sqlSearch.addField("package_action", ST_REMOVE);
	sqlSearch.addField("package_action", ST_PURGE);
	sqlSearch.addField("package_action", ST_REPAIR);
	sqlSearch.addField("package_action", ST_UPDATE);
	db->get_packagelist(&sqlSearch, &toInstall, true);
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


int mpkgSys::build_package(string out_directory, bool source)
{

	if (source) printf("Building in [%s], source package\n", out_directory.c_str());
	if (!source) printf("Building in [%s], binary package\n", out_directory.c_str());

	string pkgname;
	if (out_directory.empty()) out_directory = "./";
	out_directory+="/";

	    	//sysline = "tar czf "+pkgname+".tgz *"; // For macos
	if (FileNotEmpty("install/data.xml"))
    	{
	    	PackageConfig p("install/data.xml");
	    	if (!p.parseOk) 
		{
			mError("Parse error");
			return -100;
		}
		if (source)
		{
			pkgname =  p.getName()+"-"+p.getVersion()+"-"+p.getBuild();
	    		say(_("Packing source package to %s%s.spkg\n"), out_directory.c_str(), pkgname.c_str());
			string n = out_directory + p.getName()+"-"+p.getVersion()+"-"+p.getBuild()+".spkg";
			unlink(n.c_str());
			system("ls -la && tar -cf " + out_directory + p.getName()+"-"+p.getVersion()+"-"+p.getBuild()+".spkg *");
		}
		else {

			pkgname =  p.getName()+"-"+p.getVersion()+"-"+p.getArch()+"-"+p.getBuild();
			say(_("Packing binary package to %s%s.tgz\n"), out_directory.c_str(), pkgname.c_str());
			system("makepkg -l y -c n " + out_directory + pkgname+".tgz");
		}
	}
	else {
		mError("No XML data, cannot build package");
		return -1;
	}
	say(_("Package was built to %s/%s\n"), out_directory.c_str(), pkgname.c_str());
    	return 0;
}
int mpkgSys::update_repository_data(mpkgDatabase *db)//, DependencyTracker *DepTracker)
{
	actionBus.clear();
	actionBus.addAction(ACTIONID_DBUPDATE);
	actionBus.setActionProgressMaximum(ACTIONID_DBUPDATE, REPOSITORY_LIST.size());
	// Функция, с которой начинается обновление данных.
	
	Repository *rep = new Repository;		// Объект репозиториев
	PACKAGE_LIST *availablePackages = new PACKAGE_LIST;		// Список пакетов, полученных из всех репозиториев...
	PACKAGE_LIST *tmpPackages = new PACKAGE_LIST;		// Список пакетов, полученных из текущего репозитория (временное хранилище)

	// А есть ли у нас вообще репозитории? Может нам и ловить-то нечего?...
	// Впрочем, надо все равно пойти на принцип и пометить все пакеты как недоступные. Ибо это действительно так.
	// Поэтому - проверка устранена.
	Dialog d;
	if (!dialogMode) say(_("Updating package data from %d repository(s)...\n"), REPOSITORY_LIST.size());
	else d.execInfoBox("Получение списка пакетов из " + IntToStr(REPOSITORY_LIST.size()) + " репозиториев"); // TODO: LANG_GETTEXT_TRANSLATE
	int total_packages=0; // Счетчик полученных пакетов.

	actionBus.setCurrentAction(ACTIONID_DBUPDATE);
	// Поехали! Запрашиваем каждый репозиторий через функцию get_index()
	unsigned int cnt=1;
	for (unsigned int i=0; i<REPOSITORY_LIST.size(); i++)
	{
		delete tmpPackages;
		tmpPackages = new PACKAGE_LIST;					//Очищаем временный список.
		rep->get_index(REPOSITORY_LIST[i], tmpPackages);	// Получаем список пакетов.
		actionBus.setActionProgress(ACTIONID_DBUPDATE, cnt);
		cnt++;
		if (!tmpPackages->IsEmpty())				// Если мы таки получили что-то, добавляем это в список.
		{
			total_packages+=tmpPackages->size();		// Увеличим счетчик
			availablePackages->add_list(tmpPackages);	// Прибавляем данные к общему списку.
		}
		actionBus.setActionProgress(ACTIONID_DBUPDATE, cnt);
		cnt++;

	}
	delete rep;
	delete tmpPackages;
	// Вот тут-то и начинается самое главное. Вызываем фильтрацию пакетов (действие будет происходить в функции updateRepositoryData.
	int ret=db->updateRepositoryData(availablePackages);
	if (!dialogMode) say(_("Update complete.\n"));
	else {
		d.execInfoBox(_("Update complete.\n"));
	}

	actionBus.setActionState(ACTIONID_DBUPDATE);
	return ret;
}

// Установка (обновление)
// Делаются следующие проверки:
// 1) Проверяется есть ли такой пакет в базе
// 2) Проверяется его доступность
// Если все ок, направляем в DepTracker
	
int mpkgSys::requestInstall(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker, bool localInstall)
{
	mDebug("requested to install " + IntToStr(package_id));
	PACKAGE tmpPackage;
	int ret = db->get_package(package_id, &tmpPackage);
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", tmpPackage.get_name());
	PACKAGE_LIST candidates;
	db->get_packagelist(&sqlSearch, &candidates);
	mDebug("received " + IntToStr(candidates.size()) + " candidates to update"); 
	for (int i=0; i<candidates.size(); i++)
	{
		mDebug("checking " + IntToStr(i));
		if (candidates.get_package(i)->installed() && !candidates.get_package(i)->equalTo(&tmpPackage))
		{
			//if (!dialogMode) say(_("Updating package %s\n"), candidates.get_package(i)->get_name()->c_str());
			requestUninstall(*candidates.get_package(i)->get_name(), db, DepTracker);
		}
	}
	if (ret == 0)
	{
		if (tmpPackage.installed())
		{
			mError(_("Package ") + *tmpPackage.get_name() + " " + tmpPackage.get_fullversion() + _(" is already installed"));
		}
		if (!tmpPackage.available(localInstall))
		{
			mError(_("Package ") + *tmpPackage.get_name() + " " + tmpPackage.get_fullversion() + _(" is unavailable"));
		}
		if (tmpPackage.available(localInstall) && !tmpPackage.installed())
		{
			tmpPackage.set_action(ST_INSTALL);
			if (!ignoreDeps) DepTracker->addToInstallQuery(&tmpPackage);
			else db->set_action(tmpPackage.get_id(), ST_INSTALL);
			return tmpPackage.get_id();
		}
		else
		{
			return MPKGERROR_IMPOSSIBLE;
		}
	}
	else
	{
		mError(_("get_package error: returned ") + IntToStr (ret));
		return ret;
	}
}
int mpkgSys::requestInstallGroup(string groupName, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	mDebug("requesting data");
	SQLRecord sqlSearch;
	PACKAGE_LIST candidates;
	db->get_packagelist(&sqlSearch, &candidates);
	vector<string> install_list;
	for (int i=0; i<candidates.size(); i++)
	{
		for (unsigned int t=0; t<candidates.get_package(i)->get_tags()->size(); t++)
		{
			if (candidates.get_package(i)->get_tags()->at(t)==groupName)
			{
				install_list.push_back(*candidates.get_package(i)->get_name());
			}
		}
	}
	mDebug("Requesting to install " + IntToStr(install_list.size()) + " packages from group " + groupName);
	for (unsigned int i=0; i<install_list.size(); i++)
	{
		requestInstall(install_list[i],(string) "",(string) "", db, DepTracker);
	}
	return 0;
}

int mpkgSys::requestInstall(PACKAGE *package, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	if (!package->installedVersion.empty() && !package->installed()) requestUninstall(*package->get_name(), db, DepTracker); 
	return requestInstall(package->get_id(), db, DepTracker);
}


int mpkgSys::requestInstall(string package_name, string package_version, string package_build, mpkgDatabase *db, DependencyTracker *DepTracker)
{
	// Exclusion for here: package_name could be a filename of local placed package.
	bool tryLocalInstall=false;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", &package_name);
	if (!package_version.empty()) {
		sqlSearch.addField("package_version", &package_version);
		//printf("adding package version [%s] to search\n", package_version.c_str());
	}
	if (!package_build.empty()) sqlSearch.addField("package_build", &package_build);
	PACKAGE_LIST candidates;
	int ret = db->get_packagelist(&sqlSearch, &candidates);
	mDebug("received " + IntToStr(candidates.size()) + " candidates for " + package_name);
	// TODO: let user know if multiple packages with same name, version and build are available, and let him choose.
	int id=-1;
	if (ret == 0)
	{
		mDebug("checking id...");
		id = candidates.getMaxVersionID(&package_name);
		mDebug("id = " + IntToStr(id));
		if (id>=0)
		{
			mDebug("requesting to install " + candidates.get_package(candidates.getMaxVersionNumber(&package_name))->get_fullversion());
			return requestInstall(id, db, DepTracker);
		}
		else 
		{

			mDebug("trying local install");
			if (FileExists(package_name)) tryLocalInstall=true;
		}
	}
	
	if (!tryLocalInstall || !FileExists(package_name))
	{
		string error_package = package_name;
		if (!package_version.empty()) error_package += _(" version ") + package_version;
		if (!package_build.empty()) error_package += _(" build ") + package_build;
		mError(_("No such package: ") + error_package);
		return MPKGERROR_NOPACKAGE;
	}

	else
	{
		/*
		if (getExtension(package_name)=="spkg")
		{
			if (emerge_package(package_name, &package_name)!=0 || !FileExists(package_name)) {
				mError(_("Cannot build this spkg"));
				return MPKGERROR_NOPACKAGE;
			}
		}*/
		say(_("Installing local package %s\n"), package_name.c_str());
		LocalPackage _p(package_name);
		_p.injectFile();
		db->emerge_to_db(&_p.data);
		requestInstall(_p.data.get_id(), db, DepTracker, true);
		return 0;
	}
}	
// Удаление
int mpkgSys::requestUninstall(PACKAGE *package, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge)
{
	return requestUninstall(package->get_id(), db, DepTracker, purge);
}

int mpkgSys::requestUninstall(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge)
{
	mDebug("requestUninstall\n");
	PACKAGE tmpPackage;
	int ret = db->get_package(package_id, &tmpPackage);
	mDebug("uninstall called for id " + IntToStr(package_id) + ", name = " + *tmpPackage.get_name() + "-" + tmpPackage.get_fullversion());
	bool process=false;
	if (ret == 0)
	{
		if (tmpPackage.configexist())
		{
			if (purge)
			{
				mDebug("action set to purge");
				tmpPackage.set_action(ST_PURGE);
				process=true;
			}
			else if (tmpPackage.installed())
			{
				mDebug("action is remove");
				tmpPackage.set_action(ST_REMOVE);
				process=true;
			}
		}
		else
		{
			say(_("%s-%s doesn't present in the system\n"), tmpPackage.get_name()->c_str(), tmpPackage.get_fullversion().c_str());
		}
		if (process)
		{
			mDebug("Processing deps");
			if (!ignoreDeps) DepTracker->addToRemoveQuery(&tmpPackage);
			else {
				if (purge) db->set_action(tmpPackage.get_id(), ST_PURGE);
				else db->set_action(tmpPackage.get_id(), ST_REMOVE);
			}

			return tmpPackage.get_id();
		}
		else
		{
			if (purge) mError(_("Package ") + *tmpPackage.get_name() + " "+ tmpPackage.get_fullversion() + _(" is already purged"));
			else  mError(_("Package ") + *tmpPackage.get_name() + " " + tmpPackage.get_fullversion() + _(" is already purged"));
;
			return MPKGERROR_IMPOSSIBLE;
		}
	}
	else
	{
		return ret;
	}
}
int mpkgSys::requestUninstall(string package_name, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge)
{
	mDebug("requestUninstall of " + package_name);
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", &package_name);
	if (!purge) sqlSearch.addField("package_installed", 1);
	else sqlSearch.addField("package_configexist",1);
	PACKAGE_LIST candidates;
	int ret = db->get_packagelist(&sqlSearch, &candidates);
	mDebug("candidates to uninstall size = " + IntToStr(candidates.size()));
	int id=-1;
	if (ret == 0)
	{
		if (candidates.size()>1)
		{
			mError(_("Ambiguity in uninstall: multiple packages with some name are installed"));
			return MPKGERROR_AMBIGUITY;
		}
		if (candidates.IsEmpty())
		{
			mError(_("No packages to uninstall"));
			return MPKGERROR_NOPACKAGE;
		}
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
	unsigned short x=0, y=0;

	if (file_status->st_ino!=0) x=y;

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

int mpkgSys::_clean_symlinks(const char *filename, const struct stat *file_status, int filetype)
{
	unsigned short x=0, y=0;

	if (file_status->st_ino!=0) x=y;

	if (filetype==0) x=y;
	if (S_ISLNK(file_status->st_mode))
	{
		unlink(filename);
	}
	return 0;
}


int mpkgSys::_conv_dir(const char *filename, const struct stat *file_status, int filetype)
{
	unsigned short x=0, y=0;

	if (file_status->st_ino!=0) x=y;

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
	output_dir=getAbsolutePath(out_dir);
	ftw("./", _conv_dir, 100);
	return 0;
}


