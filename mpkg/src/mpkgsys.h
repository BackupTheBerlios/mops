/*********************************************************
 * MOPSLinux packaging system: general functions (header)
 * $Id: mpkgsys.h,v 1.6 2007/04/27 23:50:15 i27249 Exp $
 * ******************************************************/


#ifndef MPKGSYS_H_
#define MPKGSYS_H_

#include "config.h"
#include "local_package.h"
#include "debug.h"
#include "mpkg.h"
#include "repository.h"
#include "actions.h"
#include "converter.h"
#include "errorcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>


namespace mpkgSys {

#ifdef OLD_INSTALL_SYSTEM
	int install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker, bool do_upgrade=false);
	int install(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker);
	int uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker, int do_purge, bool do_upgrade=false);
	int upgrade (string pkgname, mpkgDatabase *db, DependencyTracker *DepTracker);
	int upgrade(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker);
#endif

#ifdef	NEW_INSTALL_SYSTEM
	// Данные функции однозначно выделяют пакет для действия, и проводят первичную проверку исполнимости действия.
	// Возвращаемые значения:
	// 	ID пакета - в случае успешного определения пакета и исполнимости действия
	// 	MPKGERROR_NOPACKAGE - если не удалось однозначно определить пакет (т.е. такого пакета не существует)
	// 	MPKGERROR_IMPOSSIBLE - если действие невыполнимо (например, запрошен на установку недоступный пакет)
	// 	MPKGERROR_SQLQUERYERROR - в случае ошибки в SQL
	// 	MPKGERROR_AMBIGUITY - в случае невозможности однозначного определения пакета.
	//
	// Установка (обновление)
	int requestInstall(PACKAGE *package, mpkgDatabase *db, DependencyTracker *DepTracker);
	int requestInstall(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker);
	int requestInstall(string package_name, mpkgDatabase *db, DependencyTracker *DepTracker);
	
	// Удаление
	int requestUninstall(PACKAGE *package, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge=false);
	int requestUninstall(int package_id, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge=false);
	int requestUninstall(string package_name, mpkgDatabase *db, DependencyTracker *DepTracker, bool purge=false);

#endif
	int build_package();
	int update_repository_data(mpkgDatabase *db, DependencyTracker *DepTracker);
	int _clean(const char *filename, const struct stat *file_status, int filetype);
	int clean_cache();
	int clean_queue(mpkgDatabase *db);
	int unqueue(int package_id, mpkgDatabase *db);
	int convert_directory(string output_dir);
	int _conv_dir(const char *filename, const struct stat *file_status, int filetype);
	int build_package();

}

#endif //MPKGSYS_H_
