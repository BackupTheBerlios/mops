/* Dependency tracking
$Id: dependencies.cpp,v 1.44 2007/08/25 20:33:30 i27249 Exp $
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

void DependencyTracker::createPackageCache()
{

	SQLRecord sqlSearch;
	db->get_packagelist(&sqlSearch, &packageCache);
	cacheCreated=true;
}

void DependencyTracker::fillInstalledPackages()
{
	if (!cacheCreated) createPackageCache();
	installedPackages.clear();
	for (int i=0; i<packageCache.size(); i++)
	{
		if (packageCache.get_package(i)->installed()) installedPackages.add(packageCache.get_package(i));
	}
}

int DependencyTracker::renderData()
{
	createPackageCache();
	fillInstalledPackages();
	/*
	printf("Retrieving data from SQL\n");
	// Retrieving common package list from database - we will use C++ logic.
	SQLRecord sqlSearch;
	sqlSearch.addField("package_installed", 1);
	db->get__packagelist(&sqlSearch, &installedPackages);
	*/

	mDebug("Rendering installations");
	int failureCounter = 0;
	printf("rendering required list\n");
	PACKAGE_LIST installStream = renderRequiredList(&installQueryList);

	mDebug("Rendering removing");
	printf("Rendering remove queue\n");
	PACKAGE_LIST removeStream = renderRemoveQueue(&removeQueryList);
	mDebug("Filtering dupes: install");
	currentStatus=_("Checking dependencies: filtering (stage 1: installation queue dupes");
	printf("Filtering dupes\n");
	filterDupes(&installStream);
	currentStatus=_("Checking dependencies: filtering (stage 2: remove queue dupes");
	mDebug("Filtering dupes: remove");
	filterDupes(&removeStream);
	printf("preparing rollback\n");
	currentStatus=_("Checking dependencies: filtering (stage 3: rollback)");
	mDebug("Rolling back the items who was dropped on update");
	PACKAGE_LIST fullWillBeList = installedPackages;
	fullWillBeList.add(&installStream);
	bool requested=false;
	printf("Starting a loop of rollbacking\n");
	for (int i=0; i<removeStream.size(); i++)
	{
		requested=false;
		for (int t=0; t<removeQueryList.size(); t++)
		{
			if (removeStream.get_package(i)->equalTo(removeQueryList.get_package(t)))
			{
				// Package is requested to remove by user, we shouldn't roll back
				requested=true;
			}
		}
		if (!requested && checkBrokenDeps(removeStream.get_package(i), fullWillBeList))
		{
			// If package:
			//    - isn't requested to remove by user, and
			//    - some of packages requested to install depends on it
			// then we should cancel it's removal

			//printf("rolling back %s\n", removeStream.get_package(i)->get_name()->c_str());
			mDebug("Rolling back " + *removeStream.get_package(i)->get_name());
			installStream.add(removeStream.get_package(i));
		}
	}
	printf("rollback end\n");
	// END OF ROLLBACK MECHANISM

	mDebug("Muxing streams");
	printf("muxing streams\n");
	currentStatus=_("Checking dependencies: muxing queues");
	muxStreams(installStream, removeStream);
	currentStatus=_("Checking dependencies: searching for broken packages");
	mDebug("Searching for broken packages");
	printf("Searching for broken packages\n");
	failureCounter = findBrokenPackages(installList, &failure_list);
	mDebug("done");
	currentStatus=_("Dependency check completed, error count: ") + IntToStr(failureCounter);
	printf("renderData complete\n");
	if (!force_dep) return failureCounter;
	else return 0;
}
// Tree
PACKAGE_LIST DependencyTracker::renderRequiredList(PACKAGE_LIST *installationQueue)
{
	currentStatus=_("Checking dependencies: rendering requirements");
	mDebug("Rendering required list\n");
	// installationQueue - user-composed request for installation
	// outStream - result, including all required packages.
	PACKAGE_LIST outStream;
	PACKAGE_LIST req;
	outStream.add(installationQueue);
	bool skipThis;
	for (int i=0; i<outStream.size(); i++)
	{
		currentStatus = _("Checking dependencies: rendering requirements") + (string) " (" + IntToStr(i) + "/"+IntToStr(outStream.size()) + ")";
		//printf("%s: cycle %d\n",__func__, i);
		req=get_required_packages(outStream.get_package(i));
		//printf("Package was get\n");
		// Check if this package is already in stream
		for (int t=0; t<req.size(); t++)
		{
			skipThis=false;
			//printf("Cheking cycle\n");
			// Will check and add by one
			for (int c=0; c<outStream.size(); c++)
			{
				if (req.get_package(t)->installed() || req.get_package(t)->get_id()==outStream.get_package(c)->get_id()) 
				{
					skipThis=true;
					break;
				}
			}
			//printf("cycle complter\n");
			if (!skipThis) outStream.add(req.get_package(t));
		}
		//outStream.add(&req);
	}
	//printf("end\n");
	return outStream;
}

PACKAGE_LIST DependencyTracker::get_required_packages(PACKAGE *package)
{
	// TODO: Need a FASTUP!!
	// Returns a list of required packages. Broken ones is marked internally
	PACKAGE_LIST requiredPackages;
	PACKAGE tmpPackage;
	for (unsigned int i=0; i<package->get_dependencies()->size(); i++)
	{
		if (get_dep_package(&package->get_dependencies()->at(i), &tmpPackage)!=0) {
			mError(_("package ") + *package->get_name() + " " + package->get_fullversion() + _(" is broken")); package->set_broken();
		}
		else requiredPackages.add(&tmpPackage);
	}
	return requiredPackages;
}

void DependencyTracker::fillByName(string *name, PACKAGE_LIST *p)
{
	if (!cacheCreated) createPackageCache();
	p->clear();
	for (int i=0; i<packageCache.size(); i++)
	{
		if (*packageCache.get_package(i)->get_name()==*name) p->add(packageCache.get_package(i));
	}
}
int DependencyTracker::get_dep_package(DEPENDENCY *dep, PACKAGE *returnPackage)
{
	returnPackage->clear();
	returnPackage->isRequirement=true;
	returnPackage->set_name(dep->get_package_name());
	returnPackage->set_broken(true);
	returnPackage->set_requiredVersion(dep->get_version_data());
	PACKAGE_LIST reachablePackages;
	
	fillByName(dep->get_package_name(), &reachablePackages);
	
	/*SQLRecord sqlSearch;
	sqlSearch.addField("package_name", dep->get_package_name());
	db->get__packagelist(&sqlSearch, &reachablePackages);*/
	
	if (reachablePackages.IsEmpty())
	{
		mError(_("Required package ") + *dep->get_package_name() + _(" not found"));
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
		mError(dep->getDepInfo() + _(" is required, but no suitable version was found"));
		return MPKGERROR_NOPACKAGE;
	}
	if (candidates.hasInstalledOnes()) *returnPackage = *candidates.getInstalledOne();
	else *returnPackage = *candidates.getMaxVersion();
	return MPKGERROR_OK;
}

PACKAGE_LIST DependencyTracker::renderRemoveQueue(PACKAGE_LIST *removeQueue)
{
	currentStatus=_("Checking dependencies: rendering remove queue");
	// removeQueue - user-composed remove queue
	// removeStream - result. Filtered.
	PACKAGE_LIST removeStream;
	PACKAGE_LIST tmp;
	removeStream.add(removeQueue);
	bool skipThis;
	for (int i=0; i<removeStream.size(); i++)
	{
		currentStatus=_("Checking dependencies: rendering remove queue") + (string) " (" + IntToStr(i) + "/" + IntToStr(removeStream.size())+")";


		tmp = get_dependant_packages(removeStream.get_package(i));
		//printf( "dependant for %s has %d items\n",removeStream.get_package(i)->get_name()->c_str(), tmp.size());
		for (int t=0; t<tmp.size(); t++)
		{
			skipThis=false;
			for (int c=0; c<removeStream.size(); c++)
			{
				if (!tmp.get_package(t)->installed() || removeStream.get_package(c)->get_id()==tmp.get_package(t)->get_id())
				{
					//printf("Skipping %s\n",removeStream.get_package(c)->get_name()->c_str());
					skipThis=true;
					break;
				}
			}
			if (!skipThis) 
			{
				//printf("adding to remove stream: %s\n", tmp.get_package(t)->get_name()->c_str());
				removeStream.add(tmp.get_package(t));
			}
		}
	}
	return removeStream;
}

PACKAGE_LIST DependencyTracker::get_dependant_packages(PACKAGE *package)
{
	if (!cacheCreated) { fillInstalledPackages(); }
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

void DependencyTracker::fillByAction(int action, PACKAGE_LIST *p)
{
	if (!cacheCreated) createPackageCache();
	for (int i=0; i<packageCache.size(); i++)
	{
		if (packageCache.get_package(i)->action()==action) p->add(packageCache.get_package(i));
	}
}

void DependencyTracker::muxStreams(PACKAGE_LIST installStream, PACKAGE_LIST removeStream)
{
	PACKAGE_LIST install_list;
	PACKAGE_LIST remove_list;
	PACKAGE_LIST conflict_list;
	PACKAGE_LIST installQueuedList;
	PACKAGE_LIST removeQueuedList;
	/*SQLRecord sqlSearch;
	sqlSearch.clear();	
	sqlSearch.addField("package_action", ST_INSTALL);
	db->get__packagelist(&sqlSearch, &installQueuedList);*/

	fillByAction(ST_INSTALL, &installQueuedList);
#ifdef EXTRACHECK_REMOVE_QUEUE
	/*sqlSearch.clear();
	sqlSearch.setSearchMode(SEARCH_IN);
	sqlSearch.addField("package_action", IntToStr(ST_REMOVE));
	sqlSearch.addField("package_action", IntToStr(ST_PURGE));
	db->get__packagelist(&sqlSearch, &removeQueuedList);
	*/
	fillByAction(ST_REMOVE, &removeQueuedList);
	fillByAction(ST_PURGE, &removeQueuedList);
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
			if (installStream.get_package(t)->equalTo(removeStream.get_package(i))) //FIXME: Ооооочень странное место... глючное
			{
				mDebug("Rollback of " + *installStream.get_package(t)->get_name() + " is confirmed");
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
	// Returns true if it is required by someone in searchList, or it is already in searchList himself
	// False if no package depends on it
	bool hasdependant;
	// Step 1. Check if it conflicts with someone in searchlist
	/*for (int i=0; i<searchList.size(); i++)
	{
		if (pkg->equalTo(searchList.get_package(i))) 
		{
			printf("%s: equals to item in searchList, returning TRUE\n",__func__);
			return true;
		}
		//if (*pkg->get_name() == *searchList.get_package(i)->get_name()) return true; // Temp solution - we should check if a version change can broke something.
	}*/


	for (int t=0; t<searchList.size(); t++)
	{
		hasdependant=false;

		for (unsigned int i=0; i<searchList.get_package(t)->get_dependencies()->size(); i++)
		{
			if (*searchList.get_package(t)->get_dependencies()->at(i).get_package_name() == *pkg->get_name() && \
					meetVersion(searchList.get_package(t)->get_dependencies()->at(i).get_version_data(), pkg->get_version()))
			{
				hasdependant=true;
				break;
			}
		}
		if (!hasdependant)
		{
			//printf("%s: All clean, we don't remove this\n",__func__);
			return false; // We can remove it
		}
	}
	
	//printf("%s: No packages to complain\n",__func__);
	return false; // No packages to complain - it's alone one :)
	
}


bool DependencyTracker::commitToDb()
{
	mDebug("Tracking and committing to database");
	//if (installList.size()>0) say(_("Committing %d packages to install:\n"), installList.size());
	//else say (_("No packages to install\n"));
	int iC=0;
	vector<int> i_ids;
	bool alreadyThere;
	if (!dialogMode) say(_("Will be installed:\n"));
	for (int i=0; i<installList.size(); i++)
	{
		if (!installList.get_package(i)->installed())
		{
			alreadyThere=false;
			for (unsigned int v=0; v<i_ids.size(); v++)
			{
				if (i_ids[v]==installList.get_package(i)->get_id())
				{
					alreadyThere=true;
				}
			}
			if (!alreadyThere)
			{
				if (!dialogMode) say("  [%d] %s %s\n", iC, installList.get_package(i)->get_name()->c_str(), installList.get_package(i)->get_fullversion().c_str());
				iC++;
				db->set_action(installList.get_package(i)->get_id(), ST_INSTALL);
				i_ids.push_back(installList.get_package(i)->get_id());
			}
		}
	}
	//if (removeList.size()>0) say(_("Committing %d packages to remove\n"), removeList.size());
	//else say (_("No packages to remove\n"));
	int rC=0;
	vector<int> r_ids;
	if (!dialogMode) say(_("Will be removed:\n"));
	for (int i=0; i<removeList.size(); i++)
	{

		//TODO: check for essential packages
	/*	if (!package->isUpdating && package->isRemoveBlacklisted())
	{
		mError(_("Cannot remove package ") + *package->get_name() + _(", because it is an important system component."));
		set_action(package->get_id(), ST_NONE);
		return MPKGERROR_IMPOSSIBLE;
	}*/

		if (removeList.get_package(i)->configexist())
		{
			alreadyThere=false;
			for (unsigned int v=0; v<r_ids.size(); v++)
			{
				if (r_ids[v]==removeList.get_package(i)->get_id()) alreadyThere=true;
			}
			if (!alreadyThere)
			{
				if (!dialogMode) say("  [%d] %s %s\n", rC, removeList.get_package(i)->get_name()->c_str(), removeList.get_package(i)->get_fullversion().c_str());
				rC++;
				db->set_action(removeList.get_package(i)->get_id(), removeList.get_package(i)->action());
				r_ids.push_back(removeList.get_package(i)->get_id());
			}
		}
	}
	say(_("Summary: \n  to install: %d\n  to remove: %d\n"),iC, rC);
	int total_actions =  iC+rC;
	if (total_actions == 0) say(_("No actions to proceed\n"));
	else say(_("Total %d new actions to proceed\n\n"), total_actions);

	mDebug("finished");
	return true;
}

DependencyTracker::DependencyTracker(mpkgDatabase *mpkgDB)
{
	cacheCreated=false;
	db=mpkgDB;
}
DependencyTracker::~DependencyTracker(){}

