/* Dependency tracking
$Id: dependencies.cpp,v 1.8 2007/01/25 14:17:13 i27249 Exp $
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
/*	string dep_type;
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
	*/
	switch (package->get_err_type())
	{
		case DEP_OK: printf("OK\n");
			     break;

		case DEP_BROKEN: printf("broken dependencies:\n");
				 for (int i=0; i<package->get_dependencies()->size(); i++)
				 {
					 printf("%s %s %s: %s\n", package->get_dependencies()->get_dependency(i)->get_package_name().c_str(), \
							 package->get_dependencies()->get_dependency(i)->get_vcondition().c_str(), \
							 package->get_dependencies()->get_dependency(i)->get_package_version().c_str(), \
							 package->get_dependencies()->get_dependency(i)->get_vbroken().c_str());
				 }
				 break;

		case DEP_CHILD: printf("broken dependencies (child packages)\n");
				break;

		case DEP_FILECONFLICT: printf("file conflict\n");
				       break;

		case DEP_UNAVAILABLE: printf("package unavailable\n");
				      break;

		default:
				      printf("unknown error code %d\n", package->get_err_type());
	}

}



bool DependencyTracker::commitToDb()
{
	for (int i=0; i<install_list.size(); i++)
	{
		db->set_status(install_list.get_package(i)->get_id(), install_list.get_package(i)->get_status());
	}

	// Because removing of packages still not defined exaclty, leave this commented out...
	for (int i=0; i<remove_list.size(); i++)
	{
		db->set_status(remove_list.get_package(i)->get_id(), remove_list.get_package(i)->get_status());
	}

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
	status=db->check_install_package(package); // Checking status of package - is it principially possible to be installed?
	debug("Checking status of package...");
	if (status==CHKINSTALL_INSTALLED || status==CHKINSTALL_INSTALL) // If package is already installed or already marked for install - it is already good :-)
	{
		printf("Package %s is already installed or marked for install\n", package->get_name().c_str());
		return DEP_OK;
	}
	if (status==CHKINSTALL_REMOVE_PURGE || status==CHKINSTALL_REMOVE) // Package is installed, but someone wants to remove it - but changes his desigion - ok ;-)
	{
		debug("Package is installed, but marked to remove - adding to install-list, там разберутся.");
		package->set_status(PKGSTATUS_INSTALLED);
		package->set_id(db->get_package_id(package));
		install_list.add(*package); // next procedure resolves that package is already in the system and just changes the status (who run after merge)
		return DEP_OK;
	}
	if (status==CHKINSTALL_UNAVAILABLE) // Failure - package unavailable
	{
		debug("Package is unavailable");
		package->set_id(db->get_package_id(package));
		package->set_err_type(DEP_UNAVAILABLE);
		failure_list.add(*package);
		return DEP_UNAVAILABLE;
	}
	if (status==CHKINSTALL_FILECONFLICT) // Failure - file conflict
	{
		debug("File conflict detected");
		package->set_id(db->get_package_id(package));
		package->set_err_type(DEP_FILECONFLICT);
		failure_list.add(*package);
		return DEP_FILECONFLICT;
	}
	if (status==CHKINSTALL_DBERROR)	// Database error...
	{
		debug ("Database error detected, maybe multiple package records?");
		package->set_err_type(DEP_DBERROR);
		failure_list.add(*package);
		return DEP_DBERROR;
	}
	if (status==CHKINSTALL_NOTFOUND) // No such package... WTF? :-)
	{
		debug("Package not found! WTF? Error adding to DB?....");
		package->set_err_type(DEP_NOTFOUND);
		failure_list.add(*package);
		return DEP_NOTFOUND;
	}
	for (int t=0;t<install_list.size();t++)
	{
		// Skip package if it is already in install list. TODO: check version
		if (install_list.get_package(t)->get_name()==package->get_name())
			return DEP_OK;
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
	SQLRecord sqlSearch;
	// Running thru all package dependencies
	for (int i=0;i<package->get_dependencies()->size();i++)
	{
		package->get_dependencies()->get_dependency(i)->set_broken(DEP_OK); // By default, dep is OK
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
		sqlSearch.addField("package_name",  package->get_dependencies()->get_dependency(i)->get_package_name());
		db->get_packagelist(sqlSearch, &package_list); 
		
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
	} // FInish running thru deps

	// Finalizing results
	if (dep_all_ok) // If all dependencies passes the tests
	{
		debug("dep ok, adding to install list");
		package->set_id(db->get_package_id(package));
		package->set_status(PKGSTATUS_INSTALL);
		install_list.add(*package); 	// Add to install list
		// One thing we have forgotten - is there any packages in broken list, who awaited this package?
		for (int f=0;f<failure_list.size();f++)
		{
			for (int d=0;d<failure_list.get_package(f)->get_dependencies()->size();d++)
			{
				if (failure_list.get_package(f)->get_dependencies()->get_dependency(d)->get_broken()!=DEP_OK && \
						failure_list.get_package(f)->get_dependencies()->get_dependency(d)->get_package_name()==package->get_name())
				{
					// how to delete from list?... We NEED THIS! Ok, we cannot delete. But we will replace by "SPECIAL" package...
					PACKAGE placeholder;
					PACKAGE tmp;
					tmp=*failure_list.get_package(f);
					placeholder.set_name("@empty@");
					failure_list.set_package(f, placeholder); // Take a second chance to package...
					merge(&tmp);
				}
			}
		}
		return DEP_OK;			// return success code
	}
	else	// If not...
	{
		debug("dep failed, adding to failed");
		package->set_id(db->get_package_id(package));
		bool set;
		set=false;
		for (int f=0;f<failure_list.size() && !set ;f++)
		{
			if (failure_list.get_package(f)->get_name()=="@empty@")
			{
				failure_list.set_package(f, *package);
				set=true;
			}
		}
		if (!set)
		{
			package->set_err_type(DEP_BROKEN);
			failure_list.add(*package); 	// add package to failed list (it will contain all dependency error records)
		}
		return DEP_BROKEN;		// return failure code
	}
} // DependencyTracker::merge end.

RESULT DependencyTracker::unmerge(PACKAGE *package, int do_purge)
{
	debug("Dependency UNMERGE: "+package->get_name());
	PACKAGE_LIST package_list;
	PACKAGE_LIST required_by;
	if (do_purge==0)
		package->set_status(PKGSTATUS_REMOVE);
	else package->set_status(PKGSTATUS_REMOVE_PURGE);
	
	remove_list.add(*package);
	SQLTable dep_sqlTable;
	SQLRecord dep_sqlFields;
	SQLRecord dep_sqlSearch;
	
	dep_sqlFields.addField("packages_package_id");
	dep_sqlSearch.addField("dependency_package_name", package->get_name());
	if (db->getSqlDb()->get_sql_vtable(&dep_sqlTable, dep_sqlFields, "dependencies", dep_sqlSearch)!=0)
		return -1;

	//string dep_query="select packages_package_id from dependencies where dependency_package_name='"+package->get_name()+"';";
	//get_sql_table(&dep_query,&table,&rows,&cols);
	//debug("Querying linked packages, got "+IntToStr(rows) +" items");

	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	if (!dep_sqlTable.empty())
	{
		for (int i=0; i<dep_sqlTable.getRecordCount(); i++)
		{
			sqlSearch.addField("package_id", dep_sqlTable.getValue(i, "packages_package_id"));
		}
		if (db->get_packagelist(sqlSearch, &package_list)!=0)
			return -2;
		for (int i=0; i<package_list.size(); i++)
		{
			if (package_list.get_package(i)->get_status()==PKGSTATUS_INSTALLED) unmerge(package_list.get_package(i), do_purge);
		}
	}
	return 0;
}

DependencyTracker::DependencyTracker(mpkgDatabase *mpkgDB)
{
	db=mpkgDB;
}
DependencyTracker::~DependencyTracker(){}
