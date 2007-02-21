/*********************************************************
 * MOPSLinux packaging system: general functions (header)
 * $Id: mpkgsys.h,v 1.3 2007/02/21 16:01:28 i27249 Exp $
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>


namespace mpkgSys {
	int build_package();
	int install(string fname, mpkgDatabase *db, DependencyTracker *DepTracker, bool do_upgrade=false);
	int uninstall(string pkg_name, mpkgDatabase *db, DependencyTracker *DepTracker, int do_purge, bool do_upgrade=false);
	int update_repository_data(mpkgDatabase *db, DependencyTracker *DepTracker);
	int _clean(const char *filename, const struct stat *file_status, int filetype);
	int clean_cache();
	int clean_queue(mpkgDatabase *db);
	int convert_directory(string output_dir);
	int _conv_dir(const char *filename, const struct stat *file_status, int filetype);
	int upgrade (string pkgname, mpkgDatabase *db, DependencyTracker *DepTracker);
	int build_package();

}

#endif //MPKGSYS_H_
