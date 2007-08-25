/* Dependency tracking - header
$Id: dependencies.h,v 1.14 2007/08/25 18:54:49 i27249 Exp $
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

void filterDupes(PACKAGE_LIST *pkgList, bool removeEmpty=true);

class DependencyTracker
{
	private:
		PACKAGE_LIST installList;
		PACKAGE_LIST removeList;
		PACKAGE_LIST failure_list;

		PACKAGE_LIST installQueryList;
		PACKAGE_LIST removeQueryList;

		PACKAGE_LIST installedPackages;

		PACKAGE_LIST packageCache;

		void createPackageCache();
		void fillInstalledPackages();
		void fillByName(string *name, PACKAGE_LIST *p);
		void fillByAction(int ACTION, PACKAGE_LIST *p);
		bool cacheCreated;
		mpkgDatabase *db;

	public:
		void addToInstallQuery(PACKAGE *pkg);
		void addToRemoveQuery(PACKAGE *pkg);
		PACKAGE_LIST* get_install_list();
		PACKAGE_LIST* get_remove_list();
		PACKAGE_LIST* get_failure_list();
		int renderData();	// Returns 0 if all ok, failure count if something fails (broken dependencies, etc)
		bool commitToDb();
		
		PACKAGE_LIST renderRequiredList(PACKAGE_LIST *installationQueue);
		PACKAGE_LIST get_required_packages(PACKAGE *package);
		int get_dep_package(DEPENDENCY *dep, PACKAGE *returnPackage);
		PACKAGE_LIST renderRemoveQueue(PACKAGE_LIST *removeQueue);
		PACKAGE_LIST get_dependant_packages(PACKAGE *package);
		void muxStreams(PACKAGE_LIST installStream, PACKAGE_LIST removeStream);
		bool checkBrokenDeps(PACKAGE *pkg, PACKAGE_LIST searchList);

		DependencyTracker(mpkgDatabase *mpkgDB);
		~DependencyTracker();

};
#endif //DEPENDENCIES_H_

