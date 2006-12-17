/***********************************************************************************
 * 	$Id: mpkg.h version 0.1
 * 	MOPSLinux Package System
 * ********************************************************************************/

#ifndef MPKG_H_
#define MPKG_H_
#include "debug.h"
#include "core.h"
#include "dependencies.h"
#include "constants.h"
#include "local_package.h"
#include "config.h"
class mpkgDatabase
{
	public:
		void commit_actions(); // Commits ALL actions planned in database
		int fetch_package(PACKAGE *package); // Downloads package to local cache, returns 0 if ok, 1 if failed. Also, checks if package is already downloaded.
		int install_package(PACKAGE *package); // PHYSICALLY install package (extract, execute scripts)
		int remove_package(PACKAGE *package); // PHYSICALLY remove package (delete files, execute remove scripts)
//		int update_package(int removing_id, string install_filename); // Updates a package. Meta-function: first remove old, next install new
		string get_file_md5(string filename);
		mpkgDatabase();
		~mpkgDatabase();
};



#endif //MPKG_H_

