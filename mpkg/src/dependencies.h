#ifndef DEPENDENCIES_H_
#define DEPENDENCIES_H_

#include "core.h"
#include "conditions.h"

#define DEP_OK 0
#define DEP_NOTFOUND 1
#define DEP_CONFLICT 2
#define DEP_VERSION 3
#define DEP_CHILD 4
#define DEP_BROKEN 5
#define DEP_UNAVAILABLE 6
#define DEP_DBERROR 7
#define DEP_FILECONFLICT 8


class DependencyTracker
{
	private:
		PACKAGE_LIST install_list;
		PACKAGE_LIST remove_list;
		PACKAGE_LIST failure_list;

	public:
		PACKAGE_LIST* get_install_list();
		PACKAGE_LIST* get_remove_list();
		PACKAGE_LIST* get_failure_list();
		RESULT merge(PACKAGE *package, bool suggest_skip=true);
		RESULT unmerge(PACKAGE *package);
		bool commitToDb();
		void PrintFailure(PACKAGE *package);
		bool checkVersion(string version1, int condition, string version2);
		DependencyTracker();
		~DependencyTracker();
};

#endif //DEPENDENCIES_H_
