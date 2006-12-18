/* Dependency tracking
$Id: dependencies.cpp,v 1.3 2006/12/18 10:00:49 i27249 Exp $
*/


#include "dependencies.h"
#include "debug.h"
PACKAGE_LIST* DependencyTracker::get_install_list()
{
	return &install_list;
}

PACKAGE_LIST* DependencyTracker::get_remove_list()
{
	return &remove_list;
}

PACKAGE_LIST* DependencyTracker::get_failure_list()
{
	return &failure_list;
}

void DependencyTracker::PrintFailure(PACKAGE* package)
{
	string dep_type;
	for (int i=0; i<package->get_dependencies()->size();i++)
	{
		dep_type=package->get_dependencies()->get_dependency(i)->get_type();
		printf("[%s]: %s %s %s %s, result: %s\n", \
				package->get_name().c_str(), \
				dep_type.c_str(), \
				package->get_dependencies()->get_dependency(i)->get_package_name().c_str(), \
				package->get_dependencies()->get_dependency(i)->get_vcondition().c_str(), \
				package->get_dependencies()->get_dependency(i)->get_package_version().c_str(), \
				package->get_dependencies()->get_dependency(i)->get_vbroken().c_str());
	}
}



bool DependencyTracker::commitToDb()
{
	for (int i=0; i<install_list.size(); i++)
	{
		set_status(IntToStr(install_list.get_package(i)->get_id()), install_list.get_package(i)->get_status());
	}

	// Because removing of packages still not defined exaclty, leave this commented out...
	/*for (int i=0; i<remove_list.size(); i++)
	{
		set_status(remove_list.get_package(i)->get_id(), remove_list.get_package(i)->get_status());
	}*/

	return true;
}

bool DependencyTracker::checkVersion(string version1, int condition, string version2)
{
	debug("checkVersion "+version1 + " vs " + version2);
	switch (condition)
	{
		case VER_MORE:
			if (version1>version2) return true;
			else return false;
			break;
		case VER_LESS:
			if (version1<version2) return true;
			else return false;
			break;
		case VER_EQUAL:
			if (version1==version2) return true;
			else return false;
			break;
		case VER_NOTEQUAL:
			if (version1!=version2) return true;
			else return false;
			break;
		case VER_XMORE:
			if (version1>=version2) return true;
			else return false;
			break;
		case VER_XLESS:
			if (version1<=version2) return true;
			else return false;
			break;
		default:
			return true;
	}
	return true;
}

// Emerge :-)
RESULT DependencyTracker::merge(PACKAGE *package, bool suggest_skip)
{
	//Step 1. Check install (maybe, package is already installed?)
	//Actions: 	if package is already installed - do nothing
	//		if package is already marked for install - this means that it passed all checks before, do nothing
	//		if package if marked to remove or remove_purge, just mark it as installed
	//		if package is not installed - do the full procedure
	//		if package is marked to available_purge - do the full procedure
	//		if package is marked as removed - do the full procedure 
	//		if package is not installed and unavailable - hm... how can it be? ) we fail the procedure due to unavailability
	//		if file conflict - return DEP_FILECONFLICT;
	//Step 2. Check dependencies and build a list
	debug("DependencyTracker::merge");
	RESULT status=0;
	status=check_install_package(package); // Checking status of package - is it principially possible to be installed?
	debug("Checking status of package...");
	if (status==CHKINSTALL_INSTALLED || status==CHKINSTALL_INSTALL) // If package is already installed or already marked for install - it is already good :-)
	{
		debug("Package is already installed or marked for install");
		return DEP_OK;
	}
	if (status==CHKINSTALL_REMOVE_PURGE || status==CHKINSTALL_REMOVE) // Package is installed, but someone wants to remove it - but changes his desigion - ok ;-)
	{
		debug("Package is installed, but marked to remove - adding to install-list, там разберутся.");
		package->set_status(PKGSTATUS_INSTALLED);
		package->set_id(atoi(get_package_id(package).c_str()));
		install_list.add(*package); // next procedure resolves that package is already in the system and just changes the status (who run after merge)
		return DEP_OK;
	}
	if (status==CHKINSTALL_UNAVAILABLE) // Failure - package unavailable
	{
		debug("Package is unavailable");
		package->set_id(atoi(get_package_id(package).c_str()));
		failure_list.add(*package);
		return DEP_UNAVAILABLE;
	}
	if (status==CHKINSTALL_FILECONFLICT) // Failure - file conflict
	{
		debug("File conflict detected");
		package->set_id(atoi(get_package_id(package).c_str()));
		failure_list.add(*package);
		return DEP_FILECONFLICT;
	}
	if (status==CHKINSTALL_DBERROR)	// Database error...
	{
		debug ("Database error detected, maybe multiple package records?");
		failure_list.add(*package);
		return DEP_DBERROR;
	}
	if (status==CHKINSTALL_NOTFOUND) // No such package... WTF? :-)
	{
		debug("Package not found! WTF? Error adding to DB?....");
		failure_list.add(*package);
		return DEP_NOTFOUND;
	}

	debug("CHECK passed. Continue with dependencies");
	package->set_status(PKGSTATUS_INSTALL);
	// In all other cases - continue with full procedure
	PACKAGE_LIST package_list;	// Used to store list of packages with name required by dependency, independent to version
	PACKAGE_LIST candidates;	// Contains all packages that meets dependency conditions
	bool already_resolved=false; 	// Used for single dependency, sets to TRUE if a dependency is already resolved (e.g. no action is needed to resolve dep)
	bool this_dep_failed=false;	// Sets to true if a dependency is already broken
	bool dep_all_ok=true;		// This flag is TRUE until at least one dependency fails to resolve
	int dep_tree_result;		// Temporary variable to store child dependency resolution result
	bool _type_dep=false;
	// Running thru all package dependencies
	for (int i=0;i<package->get_dependencies()->size();i++)
	{
		debug("Checking dependency...");
		already_resolved=false;	// Reset flag
		this_dep_failed=false; 	// Another reset
		_type_dep=false;
		while(!_type_dep && suggest_skip && i<package->get_dependencies()->size()) //Skipping suggestions
		{
			if (package->get_dependencies()->get_dependency(i)->get_type(false)=="SUGGEST")
			{
				debug("Skipping suggestion"+package->get_dependencies()->get_dependency(i)->get_package_name(false));
				i++;
			}
			else _type_dep=true;
			
		}
		if (i>=package->get_dependencies()->size()) break;
		debug("searching complaining packages...");
		get_packagelist("select * from packages where package_name='" + package->get_dependencies()->get_dependency(i)->get_package_name() + "';", &package_list); 
		
		if (package_list.IsEmpty()) // If no packages with required name found
		{
			debug("no packages with required name found");
			dep_all_ok=false;	// Oops, dependency resolution error!
			this_dep_failed=true;	// This dependency is failed, mark it
			package->get_dependencies()->get_dependency(i)->set_broken(DEP_NOTFOUND); // Mark dependency as broken because required package not found
		}
		
		for (int p=0; p<package_list.size()&&!already_resolved&&!this_dep_failed; p++) // Checking version condition for all the packages
		{
			// Does the package meets version requirements?
			if (checkVersion(package_list.get_package(p)->get_version(), \
					 atoi(package->get_dependencies()->get_dependency(i)->get_condition().c_str()), \
					 package->get_dependencies()->get_dependency(i)->get_package_version()))
			{
				// if it meets the requirements and not already installed - add it to candidates!
				if (package_list.get_package(p)->get_status()!=PKGSTATUS_INSTALLED) candidates.add(*package_list.get_package(p));
				else already_resolved=true; // But, if it is installed - just keep it untouched, and mark that this dep is already resolved.
			}
		}

		if (candidates.size()==0 && !already_resolved && !this_dep_failed) // If no candidates found, and no other failures was up, and not already resolved 
		{
			dep_all_ok=false;	// Failure - not resolved
			this_dep_failed=true;	// Again...
			package->get_dependencies()->get_dependency(i)->set_broken(DEP_VERSION);	// Mark as broken due to version mismatch (no pkg with req.ver. found)
		}
		if (candidates.size()>=1)	// If there are at least one candidate...
		{
			PACKAGE child;
			child=candidates.findMaxVersion();
			dep_tree_result=merge(&child);	// Try to merge a one who has highest version, and get results
			if (dep_tree_result!=DEP_OK) // If candidate fails to install...
			{
				package->get_dependencies()->get_dependency(i)->set_broken(DEP_CHILD); 	// Mark as broken due to child dependencies failure
				dep_all_ok=false;	// Dependency failure - not resolved, mark it
				this_dep_failed=true;	// Again... but only for this dependency
			}
		}
	}

	// Finalizing results
	if (dep_all_ok) // If all dependencies passes the tests
	{
		debug("dep ok, adding to install list");
		package->set_id(atoi(get_package_id(package).c_str()));
		package->set_status(PKGSTATUS_INSTALL);
		install_list.add(*package); 	// Add to install list
		return DEP_OK;			// return success code
	}
	else	// If not...
	{
		debug("dep failed, adding to failed");
		package->set_id(atoi(get_package_id(package).c_str()));
		failure_list.add(*package); 	// add package to failed list (it will contain all dependency error records)
		return DEP_BROKEN;		// return failure code
	}
} // DependencyTracker::merge end.

RESULT DependencyTracker::unmerge(PACKAGE *package)
{
	PACKAGE_LIST package_list;
	PACKAGE_LIST required_by;
	
	char **table;
	int rows;
	int cols;
	string dep_query="select packages_package_id from dependencies where packages_package_name='"+package->get_name()+"';";
	get_sql_table(&dep_query,&table,&rows,&cols);

	string sql_query="select * from packages where package_id=";
	if (rows!=0)
	{
		for (int i=1; i<=rows;i++)
		{
			sql_query+=table[i];
			if (i<rows) sql_query+=" or package_id=";
			else sql_query+=";";
		}
		get_packagelist(sql_query, &package_list);
		for (int i=0;i<package_list.size();i++)
		{
			if (package_list.get_package(i)->get_status()==PKGSTATUS_INSTALLED) unmerge(package_list.get_package(i));
		}
	}
	remove_list.add(*package);
	return 0;
}

DependencyTracker::DependencyTracker(){}
DependencyTracker::~DependencyTracker(){}
