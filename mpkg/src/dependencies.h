/* Dependency tracking - header
$Id: dependencies.h,v 1.8 2007/04/27 23:50:15 i27249 Exp $
*/



#ifndef DEPENDENCIES_H_
#define DEPENDENCIES_H_
#include "mpkg.h"
#include "core.h"
#include "conditions.h"
//#include "mpkg.h"
#define DEP_OK 0
#define DEP_NOTFOUND 1
#define DEP_CONFLICT 2
#define DEP_VERSION 3
#define DEP_CHILD 4
#define DEP_BROKEN 5
#define DEP_UNAVAILABLE 6
#define DEP_DBERROR 7
#define DEP_FILECONFLICT 8

PACKAGE_LIST getDependencies(PACKAGE *package, PACKAGE_LIST *pkgList);


class DependencyTracker
{
	private:
		PACKAGE_LIST install_list;
		PACKAGE_LIST remove_list;
		PACKAGE_LIST failure_list;

		PACKAGE_LIST installQueryList;
		PACKAGE_LIST removeQueryList;
		mpkgDatabase *db;

	public:
		int renderFinalQuery();
		void addToInstallQuery(PACKAGE *pkg);
		void addToRemoveQuery(PACKAGE *pkg);
		PACKAGE_LIST* get_install_list();
		PACKAGE_LIST* get_remove_list();
		PACKAGE_LIST* get_failure_list();
		RESULT merge(PACKAGE *package, bool suggest_skip=true, bool do_normalize=false);
		RESULT unmerge(PACKAGE *package, int do_purge=0, bool do_upgrade=false);
		int normalize();
		bool commitToDb();
		void PrintFailure(PACKAGE *package);
		bool checkVersion(string version1, int condition, string version2);
		DependencyTracker(mpkgDatabase *mpkgDB);
		~DependencyTracker();
};

class depTreeItem: private mpkgDatabase
{
	public:
		PACKAGE thisItem;
		PACKAGE_LIST item;
		vector<depTreeItem> deps;
		depTreeItem(PACKAGE packageItem);
		~depTreeItem();
};
#endif //DEPENDENCIES_H_
