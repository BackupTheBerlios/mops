/* Dependency tracking
$Id: dependencies.cpp,v 1.53 2007/11/23 01:01:46 i27249 Exp $
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
	//printf("found %d broken packages\n", counter);
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

int DependencyTracker::renderDependenciesInPackageList(PACKAGE_LIST *pkgList)
{
	printf("%s\n", __func__);
	// A *very* special case: need to compute dependencies within single package list, with no access to database
	packageCache = *pkgList;
	cacheCreated=true;
	fillInstalledPackages();
	int failureCounter = 0;
	// Forming pseudo-queue
	for (int i=0; i<packageCache.size(); i++)
	{
		if (packageCache.get_package(i)->action()==ST_INSTALL) installQueryList.add(packageCache.get_package(i));
		if (packageCache.get_package(i)->action()==ST_REMOVE || \
				packageCache.get_package(i)->action()==ST_PURGE) removeQueryList.add(packageCache.get_package(i));
	}
	PACKAGE_LIST installStream;
        _tmpInstallStream = &installStream;
	installStream = renderRequiredList(&installQueryList);
	PACKAGE_LIST removeStream;
        _tmpRemoveStream = &removeStream;
	removeStream = renderRemoveQueue(&removeQueryList);
	filterDupes(&installStream);
	filterDupes(&removeStream);
	PACKAGE_LIST fullWillBeList = installedPackages;
	fullWillBeList.add(&installStream);
	bool requested = false;
	for (int i=0; i<removeStream.size(); i++)
	{
		requested=false;
		for (int t=0; t<removeQueryList.size(); t++)
		{
			if (removeStream.get_package(i)->equalTo(removeQueryList.get_package(t)))
			{
				requested=true;
				break;
			}
		}
		if (!requested && checkBrokenDeps(removeStream.get_package(i), fullWillBeList))
		{
			mDebug("Rolling back " + *removeStream.get_package(i)->get_name());
			installStream.add(removeStream.get_package(i));
		}
	}
	muxStreams(installStream, removeStream);

	failureCounter = findBrokenPackages(installList, &failure_list);

	// Summarize and push back
	
	// Reset
	for (int i=0; i<pkgList->size(); i++)
	{
		pkgList->get_package(i)->set_action(ST_NONE);
	}
	int iC=0;
	vector<int> i_ids;
	bool alreadyThere;
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
					break;
				}
			}
			if (!alreadyThere)
			{
				iC++;
				pkgList->getPackageByID(installList.get_package(i)->get_id())->set_action(ST_INSTALL);
				//db->set_action(installList.get_package(i)->get_id(), ST_INSTALL);

				i_ids.push_back(installList.get_package(i)->get_id());
			}
		}
	}
	int rC=0;
	vector<int> r_ids;
	
	//bool essentialUpdating, essentialFound=false;
	for (int i=0; i<removeList.size(); i++)
	{
		/*essentialUpdating=false;
		
		if (removeList.get_package(i)->isRemoveBlacklisted())
		{
			for (int c=0; c<installList.size(); c++)
			{
				if (*installList.get_package(c)->get_name() == *removeList.get_package(i)->get_name())
				{
					essentialUpdating = true;
					break;
				}
			}
			if (!essentialUpdating)
			{
				if (!force_essential_remove) mError(_("Cannot remove package ") + \
						*removeList.get_package(i)->get_name() + \
						_(", because it is an important system component."));
				else mWarning(_("Removing essential package ") + \
						*removeList.get_package(i)->get_name());
				if (!force_essential_remove) essentialFound=true;
			}
		}*/
		if (removeList.get_package(i)->configexist())
		{
			alreadyThere=false;
			for (unsigned int v=0; v<r_ids.size(); v++)
			{
				if (r_ids[v]==removeList.get_package(i)->get_id()) {
					alreadyThere=true;
					break;
				}
			}
			if (!alreadyThere)
			{
				rC++;
				pkgList->getPackageByID(removeList.get_package(i)->get_id())->set_action(ST_REMOVE);

				//db->set_action(removeList.get_package(i)->get_id(), removeList.get_package(i)->action());
				r_ids.push_back(removeList.get_package(i)->get_id());
			}
		}
	}
	_tmpRemoveStream = NULL;
	_tmpInstallStream = NULL;
	if (!force_dep) return failureCounter;
	else return 0;

}

int DependencyTracker::renderData()
{
	createPackageCache();
	fillInstalledPackages();

	mDebug("Rendering installations");
	int failureCounter = 0;
	//printf("rendering required list\n");
	PACKAGE_LIST installStream;
       _tmpInstallStream = &installStream;
	installStream = renderRequiredList(&installQueryList);
	

	mDebug("Rendering removing");
	//printf("Rendering remove queue\n");
	PACKAGE_LIST removeStream;
       _tmpRemoveStream = &removeStream;
	removeStream = renderRemoveQueue(&removeQueryList);

	mDebug("Filtering dupes: install");
	currentStatus=_("Checking dependencies: filtering (stage 1: installation queue dupes");
	//printf("Filtering dupes\n");
	filterDupes(&installStream);
	currentStatus=_("Checking dependencies: filtering (stage 2: remove queue dupes");
	mDebug("Filtering dupes: remove");
	filterDupes(&removeStream);
	//printf("preparing rollback\n");
	currentStatus=_("Checking dependencies: filtering (stage 3: rollback)");
	mDebug("Rolling back the items who was dropped on update");
	PACKAGE_LIST fullWillBeList = installedPackages;
	fullWillBeList.add(&installStream);
	bool requested=false;
	//printf("Starting a loop of rollbacking\n");
	for (int i=0; i<removeStream.size(); i++)
	{
		requested=false;
		for (int t=0; t<removeQueryList.size(); t++)
		{
			if (removeStream.get_package(i)->equalTo(removeQueryList.get_package(t)))
			{
				// Package is requested to remove by user, we shouldn't roll back
				requested=true;
				break;
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
	//printf("rollback end\n");
	// END OF ROLLBACK MECHANISM

	mDebug("Muxing streams");
	//printf("muxing streams\n");
	currentStatus=_("Checking dependencies: muxing queues");
	muxStreams(installStream, removeStream);
	
	// Имеем:
	// installList
	// removeList
#ifndef NO_FIND_UPDATE
	currentStatus=_("Advanced search for updates");
	// Fill in fullWillBeList
	fullWillBeList = installList;
	bool skip;
	for (int i=0; i<installedPackages.size(); i++) {
		skip=false;
		for (int t=0; t<removeList.size(); t++) {
			if (removeList.get_package(t)->get_id()==installedPackages.get_package(i)->get_id()) {
				skip=true;
				break;
			}
		}
		if (!skip) fullWillBeList.add(installedPackages.get_package(i));
	}

	PACKAGE checkPackage; // Temporary package, I don't wish to risk packageCache integrity
	for (int i=0; i<removeList.size(); i++) {
		requested=false;
		for (int t=0; t<removeQueryList.size(); t++)
		{
			if (removeList.get_package(i)->equalTo(removeQueryList.get_package(t)))
			{
				// Package is requested to remove by user, we shouldn't roll back
				requested=true;
				break;
			}
		}

		if (!requested) {
			//printf("Not requested, but broken deps: %s\n", removeList.get_package(i)->get_name()->c_str());
			// Here is a place to try to find replacement with new dependencies
			// Поищем среди доступных версии пакета, способные заменить сломанный пакет. По нахождении добавляем в installList
			for (int t=0; t<packageCache.size(); t++) {
				checkPackage = *packageCache.get_package(t);
				if (*checkPackage.get_name()==*removeList.get_package(i)->get_name()) { // Нашли пакет с таким же именем, смотрим что у него там с зависимостями
					//printf("checking package %s-%s\n", checkPackage.get_name()->c_str(), checkPackage.get_fullversion().c_str() );
					if (check_deps(&checkPackage, &fullWillBeList)) { // Подходит
						installList.add(&checkPackage);
						//printf("Added %s-%s for update broken package %s-%s\n", checkPackage.get_name()->c_str(), 
								//checkPackage.get_fullversion().c_str(),
								//removeList.get_package(i)->get_name()->c_str(), removeList.get_package(i)->get_fullversion().c_str());
						break; // Раз нашли один подходящий, нафиг что-либо еще?
					}
				}
			}
		}
	}

#endif

	currentStatus=_("Checking dependencies: searching for broken packages");
	mDebug("Searching for broken packages");
	//printf("Searching for broken packages\n");
	failureCounter = findBrokenPackages(installList, &failure_list);
	mDebug("done");
	currentStatus=_("Dependency check completed, error count: ") + IntToStr(failureCounter);
	//printf("renderData complete\n");
	_tmpRemoveStream = NULL;
	_tmpInstallStream = NULL;
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

bool DependencyTracker::check_deps(PACKAGE *package, PACKAGE_LIST *pList) // Checks if package deps can be resolved using pList
{
	// Returns a list of required packages. Broken ones is marked internally
	PACKAGE tmpPackage;
	for (unsigned int i=0; i<package->get_dependencies()->size(); i++)
	{
		//printf("%s-%s: checking dependency %s\n", package->get_name()->c_str(), package->get_fullversion().c_str(), package->get_dependencies()->at(i).getDepInfo().c_str());
		if (get_dep_package(&package->get_dependencies()->at(i), &tmpPackage, pList)!=0) {
			return false;
		}
	}
	//printf("Package %s-%s meets requirements\n", package->get_name()->c_str(), package->get_fullversion().c_str());
	return true;
}

void DependencyTracker::fillByName(string *name, PACKAGE_LIST *p, PACKAGE_LIST *testPackages)
{
	PACKAGE_LIST *list;
	if (testPackages==NULL) {
		if (!cacheCreated) createPackageCache();
		list = &packageCache;
	}
	else list = testPackages;
	p->clear();
	for (int i=0; i<list->size(); i++)
	{
		if (*list->get_package(i)->get_name()==*name) p->add(list->get_package(i));
	}
}
int DependencyTracker::get_dep_package(DEPENDENCY *dep, PACKAGE *returnPackage, PACKAGE_LIST *pList)
{
	returnPackage->clear();
	returnPackage->isRequirement=true;
	returnPackage->set_name(dep->get_package_name());
	returnPackage->set_broken(true);
	returnPackage->set_requiredVersion(dep->get_version_data());
	PACKAGE_LIST reachablePackages;
	fillByName(dep->get_package_name(), &reachablePackages, pList);
	
	/*SQLRecord sqlSearch;
	sqlSearch.addField("package_name", dep->get_package_name());
	db->get__packagelist(&sqlSearch, &reachablePackages);*/
	
	if (reachablePackages.IsEmpty())
	{
		if (pList==NULL) mError(_("Required package ") + *dep->get_package_name() + _(" not found"));
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
		if (pList==NULL) mError(dep->getDepInfo() + _(" is required, but no suitable version was found"));
		return MPKGERROR_NOPACKAGE;
	}
	if (candidates.hasInstalledOnes()) *returnPackage = *candidates.getInstalledOne();
	else *returnPackage = *candidates.getMaxVersion();
	return MPKGERROR_OK;
}

PACKAGE_LIST DependencyTracker::renderRemoveQueue(PACKAGE_LIST *removeQueue) // Построение очереди на удаление
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
	bool updating;
	for (int i=0; i<installedPackages.size(); i++)
	{
		if (installedPackages.get_package(i)->isItRequired(package))
		{
			updating=false;
			// Check if it can be replaced by any from install queue
			if (_tmpInstallStream != NULL)
			{
				for (int t=0; t<_tmpInstallStream->size(); t++)
				{
					if (*package->get_name() == *_tmpInstallStream->get_package(t)->get_name() && installedPackages.get_package(i)->isItRequired(_tmpInstallStream->get_package(t)))
					{
						updating = true;
						break;
					}
				}
			}
			if (!updating) dependantPackages.add(installedPackages.get_package(i));
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

	fillByAction(ST_INSTALL, &installQueuedList);

#ifdef EXTRACHECK_REMOVE_QUEUE
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
			if (installStream.get_package(t)->equalTo(removeStream.get_package(i))) 
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
	int iC=0;
	vector<int> i_ids;
	bool alreadyThere;
	PACKAGE_LIST iList, rList;
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
					break;
				}
			}
			if (!alreadyThere)
			{
				iC++;
				iList.add(installList.get_package(i));
				i_ids.push_back(installList.get_package(i)->get_id());
			}
		}
	}
	installList = iList;
	int rC=0;
	vector<int> r_ids;

	bool essentialFound=false;
	for (int i=0; i<removeList.size(); i++)
	{
		if (removeList.get_package(i)->configexist())
		{
			alreadyThere=false;
			for (unsigned int v=0; v<r_ids.size(); v++)
			{
				if (r_ids[v]==removeList.get_package(i)->get_id()) {
					alreadyThere=true;
					break;
				}
			}
			if (!alreadyThere)
			{
				rC++;
				rList.add(removeList.get_package(i));
				r_ids.push_back(removeList.get_package(i)->get_id());
			}
		}
	}
	removeList = rList;

	bool attemptRemoveEssential;
	// Queue are fully formed, check for essentials
	for (int i=0; i<removeList.size(); i++)
	{
		if (removeList.get_package(i)->isRemoveBlacklisted()) {
			// Checking if it is replaced by something...
			attemptRemoveEssential = true;
			for (int t=0; t<installList.size(); t++) {
				if (*installList.get_package(t)->get_name() == *removeList.get_package(i)->get_name()) {
					attemptRemoveEssential = false; // It's updating, all ok
					break;
				}
			}
			if (attemptRemoveEssential) {
				if (!force_essential_remove) {
					mError(_("Cannot remove package ") + \
						*removeList.get_package(i)->get_name() + \
						_(", because it is an important system component."));
					essentialFound = true;
				}
				else  mWarning(_("Removing essential package ") + \
						*removeList.get_package(i)->get_name());
			}
		}
	}

	if (essentialFound) {
		mError(_("Found essential packages, cannot continue"));
		return false;
	}
	// Commit actions to database if all OK
	for (int i=0; i<installList.size(); i++) db->set_action(installList.get_package(i)->get_id(), ST_INSTALL);
	for (int i=0; i<removeList.size(); i++) db->set_action(removeList.get_package(i)->get_id(), removeList.get_package(i)->action());
	mDebug("finished");
	return true;
}

DependencyTracker::DependencyTracker(mpkgDatabase *mpkgDB)
{
	_tmpRemoveStream = NULL;
	_tmpInstallStream = NULL;
	cacheCreated=false;
	db=mpkgDB;
}
DependencyTracker::~DependencyTracker(){}

