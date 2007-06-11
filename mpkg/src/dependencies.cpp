/* Dependency tracking
$Id: dependencies.cpp,v 1.32 2007/06/11 03:56:39 i27249 Exp $
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
	installQueryList.add(pkg);
}
void DependencyTracker::addToRemoveQuery(PACKAGE *pkg)
{
	removeQueryList.add(pkg);
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
			for (unsigned int t=0; t<known_md5.size(); t++)
			{
				if (*pkgList->get_package(i)->get_md5()==known_md5[t])
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
	// Retrieving common package list from database - we will use C++ logic.
	SQLRecord sqlSearch;
	sqlSearch.addField("package_installed", 1);
	db->get_packagelist(&sqlSearch, &installedPackages);

	mDebug("Rendering installations");
	int failureCounter = 0;
	PACKAGE_LIST installStream = renderRequiredList(&installQueryList);

/*	for (int i=0; i<installStream.size(); i++)
	{
		say("[%d] %s\n", i, installStream.get_package(i)->get_name()->c_str());
	}*/
	mDebug("Rendering removing");
	PACKAGE_LIST removeStream = renderRemoveQueue(&removeQueryList);
	mDebug("Filtering dupes: install");
	filterDupes(&installStream);
	mDebug("Filtering dupes: remove");
	filterDupes(&removeStream);

	mDebug("Muxing streams");
	muxStreams(installStream, removeStream);

	mDebug("Searching for broken packages");
	failureCounter = findBrokenPackages(installList, &failure_list);
	mDebug("done");
	return failureCounter;
}
// Tree
PACKAGE_LIST DependencyTracker::renderRequiredList(PACKAGE_LIST *installationQueue)
{
	// installationQueue - user-composed request for installation
	// outStream - result, including all required packages.
	PACKAGE_LIST outStream;
	PACKAGE_LIST req;
	outStream.add(installationQueue);
	for (int i=0; i<outStream.size(); i++)
	{
		req=get_required_packages(outStream.get_package(i));
		outStream.add(&req);
	}
	return outStream;
}

PACKAGE_LIST DependencyTracker::get_required_packages(PACKAGE *package)
{
	// Returns a list of required packages. Broken ones is marked internally
	PACKAGE_LIST requiredPackages;
	PACKAGE tmpPackage;
	for (unsigned int i=0; i<package->get_dependencies()->size(); i++)
	{
		if (get_dep_package(&package->get_dependencies()->at(i), &tmpPackage)!=0) {
			mError("package is broken"); package->set_broken();
		}
		else requiredPackages.add(&tmpPackage);
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
	db->get_packagelist(&sqlSearch, &reachablePackages);
	
	if (reachablePackages.IsEmpty())
	{
		mError("Required package "+ *dep->get_package_name() + " not found");
		return MPKGERROR_NOPACKAGE;
	}
	
	PACKAGE_LIST candidates;
	for (int i=0; i<reachablePackages.size(); i++)
	{
		if (reachablePackages.get_package(i)->reachable() && meetVersion(dep->get_version_data(), reachablePackages.get_package(i)->get_version()))
		{
			candidates.add(reachablePackages.get_package(i));
		}
	}
	if (candidates.IsEmpty())
	{
		mError(dep->getDepInfo() + "is required, but no suitable version was found");
		return MPKGERROR_NOPACKAGE;
	}
	if (candidates.hasInstalledOnes()) *returnPackage = *candidates.getInstalledOne();
	else *returnPackage = *candidates.getMaxVersion();
	return MPKGERROR_OK;
}

PACKAGE_LIST DependencyTracker::renderRemoveQueue(PACKAGE_LIST *removeQueue)
{
	// removeQueue - user-composed remove queue
	// removeStream - result. Filtered.
	PACKAGE_LIST removeStream;
	removeStream.add(removeQueue);
	for (int i=0; i<removeStream.size(); i++)
	{
		removeStream.push_back(get_dependant_packages(removeStream.get_package(i)));
	}
	return removeStream;
}

PACKAGE_LIST DependencyTracker::get_dependant_packages(PACKAGE *package)
{
	PACKAGE_LIST dependantPackages;
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

void DependencyTracker::muxStreams(PACKAGE_LIST installStream, PACKAGE_LIST removeStream)
{
	PACKAGE_LIST install_list;
	PACKAGE_LIST remove_list;
	PACKAGE_LIST conflict_list;
	PACKAGE_LIST installQueuedList;
	PACKAGE_LIST removeQueuedList;
	SQLRecord sqlSearch;

	sqlSearch.clear();
	sqlSearch.addField("package_action", ST_INSTALL);
	db->get_packagelist(&sqlSearch, &installQueuedList);
#ifdef EXTRACHECK_REMOVE_QUEUE
	sqlSearch.clear();
	sqlSearch.setSearchMode(SEARCH_IN);
	sqlSearch.addField("package_action", IntToStr(ST_REMOVE));
	sqlSearch.addField("package_action", IntToStr(ST_PURGE));
	db->get_packagelist(&sqlSearch, &removeQueuedList);
#endif
	bool found;
	// What we should do?
	// 1. Remove from removeStream all items who are in installStream.
	// 2. Add to remove_list resulting removeStream;
	mDebug("Stage 1: removing items from removeStream which is required by installStream");
	for (int i=0; i<removeStream.size(); i++)
	{
		found=false;
		for (int t=0;t<installStream.size();t++)
		{
			if (installStream.get_package(t)->equalTo(removeStream.get_package(i))) //FIXME: Ооооочень странное место... явно кривое
			{
				found=true;
				break;
			}
		}
		if (!found) remove_list.add(removeStream.get_package(i));
	}
	mDebug("Add to remove_stream all conflicting packages");
	// 3. Add to remove_list all installed packages who conflicts with installStream (means have the same name and other md5)
	PACKAGE_LIST proxyinstalledList = installedPackages;
       proxyinstalledList.add(&installQueuedList);
	for (int i=0; i<installStream.size(); i++)
	{
		found=false;
		for (int t=0; t<proxyinstalledList.size(); t++)
		{
			if (proxyinstalledList.get_package(t)->installed() && \
					*proxyinstalledList.get_package(t)->get_name() == *installStream.get_package(i)->get_name() && \
					*proxyinstalledList.get_package(t)->get_md5() != *installStream.get_package(i)->get_md5() \
			   )
			{
				proxyinstalledList.get_package(t)->set_action(ST_REMOVE);
				conflict_list.add(proxyinstalledList.get_package(t));
				break;
			}
		}
	}
	mDebug("Filtering conflict list");
	// 3.1 Filter conflict_list. Search for packages who required anything in conflict_list and it cannot be replaced by anything in installStream.
	remove_list.add(&conflict_list);
#ifdef BACKTRACE_DEPS
	mDebug("Backtracing dependencies");
	PACKAGE_LIST removeCandidates;
	PACKAGE_LIST removeQueue2;
	PACKAGE_LIST willInstalled = installStream + installedPackages;
	removeCandidates = conflict_list;
	for (int i=0; i<conflict_list.size(); i++)
	{
		removeCandidates = get_dependant_packages(conflict_list.get_package(i));
		for (int t=0; t<removeCandidates.size(); t++)
		{
			if (checkBrokenDeps(removeCandidates.get_package(t), willInstalled)) removeQueue2.add(removeCandidates.get_package(t));
		}
	}
	removeQueue2 = renderRemoveQueue (&removeQueue2);
	remove_list += removeQueue2;
#endif
	mDebug("Putting install_list");
	// 4. Put in install_list all installStream
	install_list = installStream;
	// 5. Return results.
	installList = install_list;
	removeList = remove_list;
	mDebug("Done");
}

bool DependencyTracker::checkBrokenDeps(PACKAGE *pkg, PACKAGE_LIST searchList) // Tests if all items in searchList can be installed without pkg
{
	bool passed;
	// Step 1. Check if it conflicts with someone in searchlist
	for (int i=0; i<searchList.size(); i++)
	{
		if (*pkg->get_name() == *searchList.get_package(i)->get_name()) return true; // Temp solution - we should check if a version change can broke something.
	}

	for (unsigned int i=0; i<pkg->get_dependencies()->size(); i++)
	{
		passed=false;
		for (int t=0; t<searchList.size(); t++)
		{
			if (*pkg->get_dependencies()->at(i).get_package_name() == *searchList.get_package(t)->get_name() && \
					meetVersion(pkg->get_dependencies()->at(i).get_version_data(), searchList.get_package(t)->get_version()))
			{
				passed=true;
				break;
			}
		}
		if (!passed) return false;
	}
	return true;
	
}


bool DependencyTracker::commitToDb()
{
	if (installList.size()>0) say("Committing %d packages to install:\n", installList.size());
	else say ("No packages to install\n");
	int iC=1;
	for (int i=0; i<installList.size(); i++)
	{
		if (!installList.get_package(i)->installed())
		{
			say("  [%d] %s %s\n", iC, installList.get_package(i)->get_name()->c_str(), installList.get_package(i)->get_fullversion().c_str());
			iC++;
			db->set_action(installList.get_package(i)->get_id(), ST_INSTALL);
		}
	}
	if (removeList.size()>0) say("Committing %d packages to remove\n", removeList.size());
	else say ("No packages to remove\n");
	int rC=1;
	for (int i=0; i<removeList.size(); i++)
	{
		if (removeList.get_package(i)->configexist())
		{
			say("  [%d] %s %s\n", rC, removeList.get_package(i)->get_name()->c_str(), removeList.get_package(i)->get_fullversion().c_str());
			rC++;
			db->set_action(removeList.get_package(i)->get_id(), removeList.get_package(i)->action());
		}
	}
	say("Total %d new actions to proceed, committing...\n\n", iC-1+rC-1);

	return true;
}

DependencyTracker::DependencyTracker(mpkgDatabase *mpkgDB)
{
	db=mpkgDB;
}
DependencyTracker::~DependencyTracker(){}

