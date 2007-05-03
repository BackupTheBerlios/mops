/* Dependency tracking
$Id: dependencies.cpp,v 1.24 2007/05/03 11:38:44 i27249 Exp $
*/


#include "dependencies.h"
#include "debug.h"
#include "constants.h"
PACKAGE_LIST* DependencyTracker::get_install_list()
{
	return &installList;
}

PACKAGE_LIST* DependencyTracker::get_remove_list()
{
	return &removeList;
}

PACKAGE_LIST* DependencyTracker::get_failure_list()
{
	return &failure_list;
}

void DependencyTracker::addToInstallQuery(PACKAGE *pkg)
{
	installQueryList.add(*pkg);
	printf("added. size = %d\n", installQueryList.size());
}
void DependencyTracker::addToRemoveQuery(PACKAGE *pkg)
{
	removeQueryList.add(*pkg);
}

int findBrokenPackages(PACKAGE_LIST pkgList, PACKAGE_LIST *output)
{
	int counter=0;
	for (int i=0; i<pkgList.size(); i++)
	{
		if (pkgList.get_package(i)->isBroken)
		{
			counter++;
			output->add(pkgList.get_package(i));
		}
	}
	return counter;
}
void filterDupes(PACKAGE_LIST *pkgList, bool removeEmpty)
{
	PACKAGE_LIST output;
	
	// Comparing by MD5.
	vector<string> known_md5;
	bool dupe;
	for (int i=0; i<pkgList->size(); i++)
	{
		dupe=false;
		if (removeEmpty && pkgList->get_package(i)->IsEmpty())
		{
			dupe=true;
		}
		else
		{
			for (int t=0; t<known_md5.size(); t++)
			{
				if (pkgList->get_package(i)->get_md5()==known_md5.at(t))
				{
					dupe=true;
					break;
				}
			}
		}
		if (!dupe) output.add(pkgList->get_package(i));
	}
	*pkgList=output;
}


int DependencyTracker::renderData()
{
	int failureCounter = 0;
	printf("installQueryList size = %d\n", installQueryList.size());
	PACKAGE_LIST installStream = renderRequiredList(&installQueryList);
	printf("installStream size = %d\n", installStream.size());
	for (int i=0; i<installStream.size(); i++)
	{
		printf("[%d] %s\n", i, installStream.get_package(i)->get_name().c_str());
	}
	PACKAGE_LIST removeStream = renderRemoveQueue(&removeQueryList);
	filterDupes(&installStream);
	filterDupes(&removeStream);
	printf("installStream size (after filter) = %d\n", installStream.size());

	muxStreams(installStream, removeStream);
	printf("installList size (final) = %d\n", installList.size());

	failureCounter = findBrokenPackages(installList, &failure_list);
	return failureCounter;
}
// Tree
PACKAGE_LIST DependencyTracker::renderRequiredList(PACKAGE_LIST *installationQueue)
{
	// installationQueue - user-composed request for installation
	// outStream - result, including all required packages.
	PACKAGE_LIST outStream;
	outStream.add(installationQueue);
	for (int i=0; i<outStream.size(); i++)
	{
		outStream.add(get_required_packages(outStream.get_package(i)));
	}
	return outStream;
}

PACKAGE_LIST DependencyTracker::get_required_packages(PACKAGE *package)
{
	// Returns a list of required packages. Broken ones is marked internally
	PACKAGE_LIST requiredPackages;
	PACKAGE tmpPackage;
	for (int i=0; i<package->get_dependencies()->size(); i++)
	{
		if (get_dep_package(package->get_dependencies()->get_dependency(i), &tmpPackage)!=0) {printf("broken pkg\n"); package->set_broken();}
		else requiredPackages.add(tmpPackage);
	}
	return requiredPackages;
}


int DependencyTracker::get_dep_package(DEPENDENCY *dep, PACKAGE *returnPackage)
{
	returnPackage->clear();
	returnPackage->isRequirement=true;
	returnPackage->set_name(dep->get_package_name());
	returnPackage->set_broken(true);
	returnPackage->set_requiredVersion(dep->get_version_data());
	PACKAGE_LIST reachablePackages;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", dep->get_package_name());
	db->get_packagelist(sqlSearch, &reachablePackages, false);
	
	if (reachablePackages.IsEmpty())
		return MPKGERROR_NOPACKAGE;
	
	PACKAGE_LIST candidates;
	for (int i=0; i<reachablePackages.size(); i++)
	{
		if (reachablePackages.get_package(i)->reachable() && meetVersion(dep->get_version_data(), reachablePackages.get_package(i)->get_version()))
		{
			candidates.add(reachablePackages.get_package(i));
		}
	}
	if (candidates.IsEmpty()) return MPKGERROR_NOPACKAGE;
	if (candidates.hasInstalledOnes()) *returnPackage = candidates.getInstalledOne();
	else *returnPackage = candidates.getMaxVersion();
	return MPKGERROR_OK;
}

PACKAGE_LIST DependencyTracker::renderRemoveQueue(PACKAGE_LIST *removeQueue)
{
	// removeQueue - user-composed remove queue
	// removeStream - result. Filtered.
	PACKAGE_LIST removeStream;
	removeStream.add(*removeQueue);
	for (int i=0; i<removeStream.size(); i++)
	{
		removeStream.add(get_dependant_packages(removeStream.get_package(i)));
	}
	return removeStream;
}

PACKAGE_LIST DependencyTracker::get_dependant_packages(PACKAGE *package)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("package_installed", "1");
	PACKAGE_LIST installedPackages;
	PACKAGE_LIST dependantPackages;
	db->get_packagelist(sqlSearch, &installedPackages,false);
	for (int i=0; i<installedPackages.size(); i++)
	{
		if (installedPackages.get_package(i)->isItRequired(package))
		{
			dependantPackages.add(installedPackages.get_package(i));
		}
	}
	// Setting appropriary actions
	for (int i=0; i<dependantPackages.size(); i++)
	{
		dependantPackages.get_package(i)->set_action(ST_REMOVE);
	}
	return dependantPackages;
}

int DependencyTracker::muxStreams(PACKAGE_LIST installStream, PACKAGE_LIST removeStream)
{
	PACKAGE_LIST install_list;
	PACKAGE_LIST remove_list;
	PACKAGE_LIST conflict_list;
	PACKAGE_LIST installedList;
	PACKAGE_LIST installQueuedList;
	PACKAGE_LIST removeQueuedList;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_installed", "1");
	db->get_packagelist(sqlSearch, &installedList, false);

	sqlSearch.clear();
	sqlSearch.addField("package_action", IntToStr(ST_INSTALL));
	db->get_packagelist(sqlSearch, &installQueuedList, false);
#ifdef EXTRACHECK_REMOVE_QUEUE
	sqlSearch.clear();
	sqlSearch.setSearchMode(SEARCH_IN);
	sqlSearch.addField("package_action", IntToStr(ST_REMOVE));
	sqlSearch.addField("package_action", IntToStr(ST_PURGE));
	db->get_packagelist(sqlSearch, &removeQueuedList);
#endif
	bool found;
	// What we should do?
	// 1. Remove from removeStream all items who are in installStream.
	// 2. Add to remove_list resulting removeStream;
	for (int i=0; i<removeStream.size(); i++)
	{
		found=false;
		for (int t=0;t<installStream.size();t++)
		{
			if (installStream.get_package(t)==installStream.get_package(i))
			{
				found=true;
				break;
			}
		}
		if (!found) remove_list.add(removeStream.get_package(i));
	}
	//printf("Stage 2: remove_list size = %d\n", remove_list.size());
	// 3. Add to remove_list all installed packages who conflicts with installStream (means have the same name and other md5)
	PACKAGE_LIST proxyinstalledList = installedList + installQueuedList;
	for (int i=0; i<installStream.size(); i++)
	{
		found=false;
		for (int t=0; t<proxyinstalledList.size(); t++)
		{
			if (proxyinstalledList.get_package(t)->installed() && \
					proxyinstalledList.get_package(t)->get_name() == installStream.get_package(i)->get_name() && \
					proxyinstalledList.get_package(t)->get_md5() != installStream.get_package(i)->get_md5() \
			   )
			{
				proxyinstalledList.get_package(t)->set_action(ST_REMOVE);
				conflict_list.add(proxyinstalledList.get_package(t));
				break;
			}
		}
	}
	//printf("Stage 3: conflict list size = %d\n", conflict_list.size());
	// 3.1 Filter conflict_list. Search for packages who required anything in conflict_list and it cannot be replaced by anything in installStream.
	remove_list += conflict_list;
#ifdef BACKTRACE_DEPS
	PACKAGE_LIST removeCandidates;
	PACKAGE_LIST removeQueue2;
	PACKAGE_LIST willInstalled = installStream + installedList;
	removeCandidates = conflict_list;
	for (int i=0; i<conflict_list.size(); i++)
	{
		removeCandidates = get_dependant_packages(conflict_list.get_package(i));
		for (int t=0; t<removeCandidates.size(); t++)
		{
			if (checkBrokenDeps(removeCandidates.get_package(t), willInstalled)) removeQueue2.add(removeCandidates.get_package(t));
		}
	}
	//printf("Stage 3.1 removeQueue2 size (berofe filtering) = %d\n", removeQueue2.size());
	removeQueue2 = renderRemoveQueue (&removeQueue2);
	//printf("Stage 3.1: removeQueue2 size (after filtering) = %d\n", removeQueue2.size());
	remove_list += removeQueue2;
#endif
	//printf("Stage 3.1: remove_list size = %d\n", remove_list.size());






	// 4. Put in install_list all installStream
	install_list = installStream;
	// 5. Return results.
	installList = install_list;
	removeList = remove_list;
}

bool DependencyTracker::checkBrokenDeps(PACKAGE *pkg, PACKAGE_LIST searchList) // Tests if all items in searchList can be installed without pkg
{
	bool passed;
	// Step 1. Check if it conflicts with someone in searchlist
	int alternateID;
	for (int i=0; i<searchList.size(); i++)
	{
		if (pkg->get_name() == searchList.get_package(i)->get_name()) return true; // Temp solution - we should check if a version change can broke something.
	}

	for (int i=0; i<pkg->get_dependencies()->size(); i++)
	{
		passed=false;
		for (int t=0; t<searchList.size(); t++)
		{
			if (pkg->get_dependencies()->get_dependency(i)->get_package_name() == searchList.get_package(t)->get_name() && \
					meetVersion(pkg->get_dependencies()->get_dependency(i)->get_version_data(), searchList.get_package(t)->get_version()))
			{
				passed=true;
				break;
			}
		}
		if (!passed) return false;
	}
	return true;
	
}


/*





void DependencyTracker::PrintFailure(PACKAGE* package)
{
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

*/

bool DependencyTracker::commitToDb()
{
	printf("commitToDb: %d packages to install\n", installList.size());
	for (int i=0; i<installList.size(); i++)
	{
		db->set_action(installList.get_package(i)->get_id(), ST_INSTALL);// installList.get_package(i)->action());
	}

	for (int i=0; i<removeList.size(); i++)
	{
		db->set_action(removeList.get_package(i)->get_id(), removeList.get_package(i)->action());
	}

	return true;
}
/*
bool DependencyTracker::checkVersion(string version1, int condition, string version2)
{
	//printf("Comparing %s with %s\n", version1.c_str(), version2.c_str());

	debug("checkVersion "+version1 + " vs " + version2);
	switch (condition)
	{
		case VER_MORE:
			if (strverscmp(version1.c_str(),version2.c_str())>0)  return true;
			else return false;
			break;
		case VER_LESS:
			if (strverscmp(version1.c_str(),version2.c_str())<0) return true;
			else return false;
			break;
		case VER_EQUAL:
			if (strverscmp(version1.c_str(),version2.c_str())==0) return true;
			else return false;
			break;
		case VER_NOTEQUAL:
			if (strverscmp(version1.c_str(),version2.c_str())!=0) return true;
			else return false;
			break;
		case VER_XMORE:
			if (strverscmp(version1.c_str(),version2.c_str())>=0) return true;
			else return false;
			break;
		case VER_XLESS:
			if (strverscmp(version1.c_str(),version2.c_str())<=0) return true;
			else return false;
			break;
		default:
			printf("unknown condition %d!!!!!!!!!!!!!!!!!!!!!!!!!\n", condition);
			return true;
	}
	return true;
}

// Emerge :-)

RESULT DependencyTracker::merge(PACKAGE *package, bool suggest_skip, bool do_normalize)
{
	printf("Emerging %s-%s (build %s)\n",package->get_name().c_str(), package->get_version().c_str(), package->get_build().c_str());
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
	
	if (!do_normalize)
	{
		if (package->installed() || package->action()==ST_INSTALL) // If package is already installed or already marked for install - it is already good :-)
		{
			printf("Package %s is already installed or marked for install\n", package->get_name().c_str());
			return DEP_OK;
		}
	}

	if (package->installed())
	{
		if (package->action()!=ST_NONE)
		{
			//package->set_id(db->get_package_id(package));
			install_list.add(*package);
		}
		return DEP_OK;
	}

	if (!package->reachable())
	{
		package->set_err_type(DEP_UNAVAILABLE);
		failure_list.add(*package);
		return DEP_UNAVAILABLE;
	}

	for (int t=0;t<install_list.size();t++)
	{
		// Skip package if it is already in install list. TODO: check version
		if (install_list.get_package(t)->get_name()==package->get_name())
			return DEP_OK;
	}


	// OLDSTYLE SCAN HALTED HERE - TODO TODO TODO!!!!
	debug("Status check passed. Continue with dependencies");
	package->set_action(ST_INSTALL);
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
		sqlSearch.clear();
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
			debug("found "+IntToStr(package_list.size()) + " packages with required name, filtering...");
			// Does the package meets requirements?
			if (package_list.get_package(p)->reachable())
			{
				debug("availability check passed");
				if (this->checkVersion(package_list.get_package(p)->get_version(), \
					 atoi(package->get_dependencies()->get_dependency(i)->get_condition().c_str()), \
					 package->get_dependencies()->get_dependency(i)->get_package_version()))
				{
					// if it meets the requirements and not already installed - add it to candidates!
					debug("Package meets requirements, adding to candidates");
					if (!package_list.get_package(p)->installed()) candidates.add(*package_list.get_package(p));
					else already_resolved=true; // But, if it is installed - just keep it untouched, and mark that this dep is already resolved.
				}
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
		package->set_action(ST_INSTALL);
		if (!do_normalize) install_list.add(*package); 	// Add to install list
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
		// And, at last of all, check if any package was broken without this package and can be restored.
		

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
			PrintFailure(package);
		}
		return DEP_BROKEN;		// return failure code
	}
} // DependencyTracker::merge end.

RESULT DependencyTracker::unmerge(PACKAGE *package, int do_purge, bool do_upgrade)
{
	debug("Dependency UNMERGE: "+package->get_name());
	
	PACKAGE_LIST package_list;
	PACKAGE_LIST required_by;
	if (do_purge==0)
		package->set_action(ST_REMOVE);
	else package->set_action(ST_PURGE);
	
	remove_list.add(*package);
	if (do_upgrade) return 0; // Exit here if we making upgrade
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
			if (package_list.get_package(i)->installed()) unmerge(package_list.get_package(i), do_purge);
		}
	}
	return 0;
}

int DependencyTracker::normalize()
{
#ifdef DEP_NORMALIZE
	printf("init variables\n");
	PACKAGE_LIST ninstall_list;
	PACKAGE_LIST nremove_list;
	PACKAGE_LIST navailable_list;
	PACKAGE_LIST doremove_list;

	SQLRecord installSearch, removeSearch, availableSearch;
	installSearch.setSearchMode(SEARCH_OR);
	removeSearch.setSearchMode(SEARCH_OR);
	availableSearch.setSearchMode(SEARCH_OR);

	installSearch.addField("package_action", IntToStr(ST_INSTALL));
	installSearch.addField("package_installed", IntToStr(ST_INSTALLED));

	removeSearch.addField("package_action", IntToStr(ST_REMOVE));
	removeSearch.addField("package_action", IntToStr(ST_PURGE));

	availableSearch.addField("package_available", IntToStr(ST_AVAILABLE));

	printf("sql requests\n");
	printf("1...\n");
	db->get_packagelist(installSearch, &ninstall_list,false);
	printf("2...\n");
	db->get_packagelist(removeSearch, &nremove_list,false);
	printf("3...\n");
	db->get_packagelist(availableSearch, &navailable_list,false);

	printf("cycle (install_list)\n");
	for (int i=0; i<ninstall_list.size(); i++)
	{
		if (merge(ninstall_list.get_package(i), true, true)!=DEP_OK)
		{
			doremove_list.add(*ninstall_list.get_package(i));
		}
	}
	//install_list.clear();
	printf("cycle (remove list)\n");
	for (int i=0; i<doremove_list.size(); i++)
	{
		unmerge(doremove_list.get_package(i), 0, true);
	}
	printf("normalize complete\n");
#endif
	return 0;


}
	*/

DependencyTracker::DependencyTracker(mpkgDatabase *mpkgDB)
{
	db=mpkgDB;
}
DependencyTracker::~DependencyTracker(){}
/*
depTreeItem::depTreeItem(PACKAGE packageItem)
{
	thisItem = packageItem;
	//item = db.get_other_versions(packageItem->get_name());
	for (int i=0; i<thisItem.get_dependencies()->size(); i++)
	{
		 
		deps.push_back(*new depTreeItem(get_max_version(get_other_versions(thisItem.get_dependencies()->get_dependency(i)->get_package_name()), thisItem.get_dependencies()->get_dependency(i))));
	}	

}
depTreeItem::~depTreeItem(){}


	
*/
