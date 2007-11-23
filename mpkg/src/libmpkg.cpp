/*********************************************************************
 * MOPSLinux packaging system: library interface
 * $Id: libmpkg.cpp,v 1.68 2007/11/23 01:01:46 i27249 Exp $
 * ******************************************************************/

#include "libmpkg.h"
#include <iostream>
mpkg::mpkg(bool _loadDatabase)
{
	if (!consoleMode && !dialogMode) initErrorManager(EMODE_QT);
	if (consoleMode && dialogMode) initErrorManager(EMODE_DIALOG);
	if (consoleMode && !dialogMode) initErrorManager(EMODE_CONSOLE);

	mDebug("creating core");
	currentStatus=_("Loading database...");
	loadGlobalConfig();
	db=NULL;
	DepTracker=NULL;
	if (_loadDatabase)
	{
		mDebug("Loading database");
		db = new mpkgDatabase();
		DepTracker = new DependencyTracker(db);
	}
	init_ok=true;
	currentStatus = _("Database loaded");
}

mpkg::~mpkg()
{
	if (DepTracker!=NULL) delete DepTracker;
	if (db!=NULL) delete db;
	delete_tmp_files();
	mpkgSys::clean_cache(true);

}

string mpkg::current_status()
{
	return currentStatus;
}

int mpkg::clean_queue()
{
	return mpkgSys::clean_queue(db);
}

int mpkg::unqueue(int package_id)
{
	return mpkgSys::unqueue(package_id, db);
}

// Package building
int mpkg::build_package(string out_directory, bool source)
{
	return mpkgSys::build_package(out_directory, source);
}
int mpkg::convert_directory(string output_dir)
{
	return mpkgSys::convert_directory(output_dir);
}

void mpkg::get_available_tags(vector<string>* output)
{
	db->get_available_tags(output);
}
	
// Packages installation
int mpkg::install(vector<string> fname, vector<string> *p_version, vector<string> *p_build)
{
	//printf("fname size = %d\n", fname.size());
	//printf("p_version size = %d\n", p_version->size());
	
	/*for (unsigned int i=0; i<fname.size(); i++) {
		printf("fname[%d]: %s\n", i, fname[i].c_str());
	}*/
	int ret=0;
	string version, build;
	for (unsigned int i = 0; i < fname.size(); i++)
	{
	//	printf("adding version %d %s\n", i, p_version->at(i).c_str());
		if (p_version!=NULL) version=p_version->at(i);
	//	printf("adding buidl %d\n", i);
		if (p_build!=NULL) build=p_build->at(i);
	//	printf("committing %d\n", i);

		currentStatus = _("Building queue: ")+IntToStr(i) + "/" +IntToStr(fname.size()) +" ["+fname[i]+"]";
		if (mpkgSys::requestInstall(fname[i], version, build, db, DepTracker)!=0) ret--;
	}
	//currentStatus = "Installation complete";
	//printf("install done\n");
	return ret;
}
int mpkg::installGroups(vector<string> groupName)
{
	for (unsigned int i=0; i<groupName.size(); i++)
	{
		mpkgSys::requestInstallGroup(groupName[i], db, DepTracker);
	}
	return 0;
}
int mpkg::install(string fname)
{
	currentStatus = _("Building queue: ")+fname;
	return mpkgSys::requestInstall(fname, (string) "",(string) "", db, DepTracker);
}

int mpkg::install(PACKAGE *pkg)
{
	return mpkgSys::requestInstall(pkg, db, DepTracker);
}

int mpkg::install(int package_id)
{
	return mpkgSys::requestInstall(package_id, db, DepTracker);
}

int mpkg::install(PACKAGE_LIST *pkgList)
{
	int ret=0;
	for (int i=0; i<pkgList->size(); i++)
	{
		if (mpkgSys::requestInstall(pkgList->get_package(i), db, DepTracker)!=0) ret--;
	}
	return ret;
}
// Packages removing
int mpkg::uninstall(vector<string> pkg_name)
{
	int ret=0;
	for (unsigned int i = 0; i < pkg_name.size(); i++)
	{
		currentStatus = _("Building queue: ")+IntToStr(i) + "/" +IntToStr(pkg_name.size()) +" ["+pkg_name[i]+"]";
		if (mpkgSys::requestUninstall(pkg_name[i], db, DepTracker)!=0) ret--;
	}
	return ret;
}

// Packages purging
int mpkg::purge(vector<string> pkg_name)
{
	int ret=0;
	for (unsigned int i = 0; i < pkg_name.size(); i++)
	{
		currentStatus = _("Building queue: ")+IntToStr(i) + "/" +IntToStr(pkg_name.size()) +" ["+pkg_name[i]+"]";
		if (mpkgSys::requestUninstall(pkg_name[i], db, DepTracker, true)!=0) ret--;
	}
	return ret;
}
// Repository data updating

void mpkg::cleanCdromCache()
{
	system("rm -rf " + SYS_ROOT + "/var/mpkg/index_cache/*");
}
int mpkg::update_repository_data()
{
	if (mpkgSys::update_repository_data(db) == 0 && db->sqlFlush()==0)
	{
		cleanCdromCache();
		currentStatus = _("Repository data updated");
		return 0;
	}
	else 
	{
		currentStatus = _("Repository data update failed");
		return -1;
	}
}

// Cache cleaning
int mpkg::clean_cache(bool clean_symlinks)
{
	currentStatus = _("Cleaning cache...");
	int ret = mpkgSys::clean_cache(clean_symlinks);
	if (ret == 0) currentStatus = _("Cache cleanup complete");
	else currentStatus = _("Error cleaning cache!");
	return ret;
}

// Package list retrieving
int mpkg::get_packagelist(SQLRecord *sqlSearch, PACKAGE_LIST *packagelist, bool ultraFast)
{
	currentStatus = _("Retrieving package list...");
	int ret = db->get_packagelist(sqlSearch, packagelist, ultraFast);
	if (ret == 0) currentStatus = _("Retriveal complete");
	else currentStatus = _("Failed retrieving package list!");
	return ret;
}

// Configuration and settings: retrieving
vector<string> mpkg::get_repositorylist()
{
	return mpkgconfig::get_repositorylist();
}
vector<string> mpkg::get_disabled_repositorylist()
{
	return mpkgconfig::get_disabled_repositorylist();
}

string mpkg::get_sysroot()
{
	return mpkgconfig::get_sysroot();
}
string mpkg::get_syscache()
{
	return mpkgconfig::get_syscache();
}
string mpkg::get_dburl()
{
	return mpkgconfig::get_dburl();
}

string mpkg::get_cdromdevice()
{
	return mpkgconfig::get_cdromdevice();
}

string mpkg::get_cdrommountpoint()
{
	return mpkgconfig::get_cdrommountpoint();
}

string mpkg::get_scriptsdir()
{
	return mpkgconfig::get_scriptsdir();
}
bool mpkg::get_runscripts()
{
	return mpkgconfig::get_runscripts();
}

unsigned int mpkg::get_checkFiles()
{
	return mpkgconfig::get_checkFiles();
}

int mpkg::set_checkFiles(unsigned int value)
{
	return mpkgconfig::set_checkFiles(value);
}

int mpkg::add_repository(string repository_url)
{
	vector<string> enabledRepositories, disabledRepositories, n1;
	enabledRepositories = get_repositorylist();
	disabledRepositories = get_disabled_repositorylist();
	// Check if it is already there
	for (unsigned int i=0; i<enabledRepositories.size(); i++) {
		if (enabledRepositories[i]==repository_url) {
			say(_("Repository %s is already in list\n"), repository_url.c_str());
			return 0; // Already there
		}
	}
	for (unsigned int i=0; i<disabledRepositories.size(); i++) {
		if (disabledRepositories[i]==repository_url) {
			enabledRepositories.push_back(repository_url);
			for (unsigned int t=0; t<disabledRepositories.size(); t++) {
				if (t!=i) n1.push_back(disabledRepositories[i]);
			}
			set_repositorylist(enabledRepositories, n1);
			return 0;
		}
	}
	enabledRepositories.push_back(repository_url);
	return set_repositorylist(enabledRepositories, disabledRepositories);
}

int mpkg::remove_repository(int repository_num)
{
	if (repository_num==0) {
		mError("No such repository"); 
		return -1;
	}
	repository_num--;
	vector<string> enabledRepositories, disabledRepositories, n1;
	enabledRepositories = get_repositorylist();
	disabledRepositories = get_disabled_repositorylist();
	if (repository_num >= (int) enabledRepositories.size())
	{
		repository_num = repository_num - enabledRepositories.size();
		if (repository_num>=(int) disabledRepositories.size()) {
			say(_("No such repository\n"));
			return -1;
		}
		for (unsigned int i=0; i<disabledRepositories.size(); i++)
		{
			if (i!=(unsigned int) repository_num) n1.push_back(disabledRepositories[i]);
		}
		return set_repositorylist(enabledRepositories, n1);
	}
	else
	{
		for (unsigned int i=0; i<enabledRepositories.size(); i++)
		{
			if (i!=(unsigned int) repository_num) n1.push_back(enabledRepositories[i]);
		}
		return set_repositorylist(n1, disabledRepositories);
	}
}
int mpkg::enable_repository(vector<int> rep_nums)
{
	vector<string> enabledRepositories, disabledRepositories, n1;
	enabledRepositories = get_repositorylist();
	disabledRepositories = get_disabled_repositorylist();
	for (unsigned int i=0; i<rep_nums.size(); i++)
	{
		if (rep_nums[i]==0 || rep_nums[i]>(int) enabledRepositories.size()+(int) disabledRepositories.size()) {
			mError("No such disabled repository number " + IntToStr(rep_nums[i]));
			return -1;
		}
		rep_nums[i]--;
	}
	for (unsigned int i=0; i<rep_nums.size(); i++)
	{
		enabledRepositories.push_back(disabledRepositories[rep_nums[i]-enabledRepositories.size()]);
	}
	bool skipIt=false;
	for (unsigned int i=0; i<disabledRepositories.size(); i++)
	{
		skipIt=false;
		for (unsigned int t=0; t<rep_nums.size(); t++) 
		{
			if (i==rep_nums[t]-enabledRepositories.size()+1) skipIt=true;
		}
		if (!skipIt) n1.push_back(disabledRepositories[i]);
	}
	set_repositorylist(enabledRepositories, n1);



	return 0;
}

int mpkg::disable_repository(vector<int> rep_nums)
{
	vector<string> enabledRepositories, disabledRepositories, n1;
	enabledRepositories = get_repositorylist();
	disabledRepositories = get_disabled_repositorylist();
	for (unsigned int i=0; i<rep_nums.size(); i++)
	{
		if (rep_nums[i]==0 || rep_nums[i]>(int) enabledRepositories.size()+(int) disabledRepositories.size()) {
			mError("No such enabled repository number" + IntToStr(rep_nums[i]));
			return -1;
		}
		rep_nums[i]--;
	}
	for (unsigned int i=0; i<rep_nums.size(); i++)
	{
		disabledRepositories.push_back(enabledRepositories[rep_nums[i]]);
	}
	bool skipIt=false;
	for (unsigned int i=0; i<enabledRepositories.size(); i++)
	{
		skipIt=false;
		for (unsigned int t=0; t<rep_nums.size(); t++) 
		{
			if ((int) i==rep_nums[t]) skipIt=true;
		}
		if (!skipIt) n1.push_back(enabledRepositories[i]);
	}
	set_repositorylist(n1, disabledRepositories);

	return 0;
}
// Configuration and settings: setting
int mpkg::set_repositorylist(vector<string> newrepositorylist, vector<string> drList)
{
	return mpkgconfig::set_repositorylist(newrepositorylist, drList);
}
int mpkg::set_sysroot(string newsysroot)
{
	return mpkgconfig::set_sysroot(newsysroot);
}

int mpkg::set_syscache(string newsyscache)
{
	return mpkgconfig::set_syscache(newsyscache);
}
int mpkg::set_dburl(string newdburl)
{
	return mpkgconfig::set_dburl(newdburl);
}

int mpkg::set_cdromdevice(string cdromDevice)
{
	return mpkgconfig::set_cdromdevice(cdromDevice);
}

int mpkg::set_cdrommountpoint(string cdromMountPoint)
{
	return mpkgconfig::set_cdrommountpoint(cdromMountPoint);
}


int mpkg::set_scriptsdir(string newscriptsdir)
{
	return mpkgconfig::set_scriptsdir(newscriptsdir);
}
int mpkg::set_runscripts(bool dorun)
{
	return mpkgconfig::set_runscripts(dorun);
}

// Finalizing
int mpkg::commit()
{
	Dialog d;
	mDebug("committing");
	if (!dialogMode) say(_("Building dependency tree\n"));
	else d.execInfoBox("Построение дерева зависимостей",3,60); // LANG_GETTEXT

	//currentStatus = _("Checking dependencies...");
	int errorCount;
       errorCount = DepTracker->renderData();
	//printf("RenderData complete\n");
	if (errorCount==0)
	{
		if (!dialogMode) say(_("Building queue\n"));
		else d.execInfoBox("Построение очереди действий",3,40);
		mDebug("Tracking deps");
		
		if (!DepTracker->commitToDb()) return MPKGERROR_IMPOSSIBLE;

		if (!dialogMode) say(_("Committing...\n"));
		else d.execInfoBox("Выполнение...");
		currentStatus = _("Committing changes...");
		unsigned int ret = db->commit_actions();
		if (ret==0) {
			if (!dialogMode) say(_("Complete successfully\n"));
			else d.execInfoBox("Запрошенные операции успешно завершены",3,60);
		}
		else 
		{
			switch(ret)
			{
				case MPKGERROR_ABORTED: say (_("Aborted\n"));
							break;
				default:
					mError(_("Commit failed, error code: ") + IntToStr(ret));
					if (dialogMode) d.execInfoBox("Запрошенные операции выполнить не удалось",3,60);
			}
		}
		currentStatus = _("Complete.");
		return ret;
	}
	else
	{
		mError(_("Error in dependencies: ") + IntToStr(errorCount) + _(" failures"));
		if (dialogMode) d.execMsgBox(_("Error in dependencies: ") + IntToStr(errorCount) + _(" failures"));
		currentStatus = _("Failed - depencency errors");
		return MPKGERROR_UNRESOLVEDDEPS;
	}
}

bool mpkg::checkPackageIntegrity(string pkgName)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", &pkgName);
	sqlSearch.addField("package_installed", ST_INSTALLED);
	PACKAGE_LIST table;
	get_packagelist(&sqlSearch, &table);
	if (table.size()==0)
	{
		mError(_("No package \"") + pkgName + _("\" is installed"));
		return true;
	}
	if (table.size()!=1)
	{
		mError(_("Received ") + IntToStr(table.size()) + _(" packages, ambiguity!"));
		return false;
	}
	return checkPackageIntegrity(table.get_package(0));
}

bool mpkg::checkPackageIntegrity(PACKAGE *package)
{

	db->get_filelist(package->get_id(), package->get_files());
	// Function description: 
	// 	1. Check files for exist
	// 	2. Check files for integrity (md5 verify)
	// 	3. Access rights check (hm...)
	bool integrity_ok = true;
	bool broken_sym = false;
	for (unsigned int i=0; i<package->get_files()->size(); i++)
	{
		if (!FileExists(SYS_ROOT + *package->get_files()->at(i).get_name(), &broken_sym))
		{
			if (integrity_ok) 
				mError(_("Package ") + (string) CL_YELLOW + *package->get_name() + (string) CL_WHITE + _(" has broken files or symlinks:"));
			integrity_ok = false;
			if (!broken_sym) 
				say(_("%s%s%s: /%s (file doesn't exist)\n"), CL_YELLOW, package->get_name()->c_str(),CL_WHITE, package->get_files()->at(i).get_name()->c_str());
			else
				say(_("%s%s%s: /%s (broken symlink)\n"), CL_YELLOW, package->get_name()->c_str(),CL_WHITE, package->get_files()->at(i).get_name()->c_str());


		}
	}
	return integrity_ok;
}

bool mpkg::repair(string fname)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", &fname);
	sqlSearch.addField("package_installed", 1);
	PACKAGE_LIST p;
	get_packagelist(&sqlSearch, &p);
	if (p.size()==1)
	{
		return repair(p.get_package(0));
	}
	else {
		say(_("Cannot repair or reinstall package %s: it is not installed\n"), fname.c_str());
		return false;
	}
}
bool mpkg::repair(PACKAGE *package)
{
	if (!package->available())
	{
		mError(_("Cannot repair ") + *package->get_name() + _(": package is unavailable"));
		return false;
	}
	db->set_action(package->get_id(), ST_REPAIR);
	return true;
}
void mpkg::exportBase(string output_dir)
{
	say("Exporting data to %s directory\n",output_dir.c_str());
	// First of all, clear previous contents
	system("rm -rf " + output_dir+"; mkdir -p " + output_dir);
	PACKAGE_LIST allPackages;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_installed", 1);
	get_packagelist(&sqlSearch, &allPackages);
	say("Received %d packages\n",allPackages.size());
	for (int i=0; i<allPackages.size(); i++)
	{
		say("[%d/%d] Exporting package %s\n",i+1,allPackages.size(),allPackages.get_package(i)->get_name()->c_str());
		db->exportPackage(output_dir, allPackages.get_package(i));
	}
}

void dumpPackage(PACKAGE *p, PackageConfig *pc, string filename)
{
	XMLNode node = XMLNode::createXMLTopNode("package");
	node.addChild("name");
	node.getChildNode("name").addText(p->get_name()->c_str());
	node.addChild("version");
	node.getChildNode("version").addText(p->get_version()->c_str());
	node.addChild("arch");
	node.getChildNode("arch").addText(p->get_arch()->c_str());
	node.addChild("build");
	node.getChildNode("build").addText(p->get_build()->c_str());

	node.addChild("description");
	node.getChildNode("description").addText(p->get_description()->c_str());
	node.addChild("short_description");
	node.getChildNode("short_description",0).addText(p->get_short_description()->c_str());
	
	node.addChild("dependencies");
	node.addChild("suggests");
	for (unsigned int i=0; i<p->get_dependencies()->size(); i++)
	{
		node.getChildNode("dependencies").addChild("dep");
		node.getChildNode("dependencies").getChildNode("dep", i).addChild("name");
		node.getChildNode("dependencies").getChildNode("dep", i).getChildNode("name").addText(p->get_dependencies()->at(i).get_package_name()->c_str());
		node.getChildNode("dependencies").getChildNode("dep", i).addChild("condition");
		node.getChildNode("dependencies").getChildNode("dep", i).getChildNode("condition").addText(condition2xml(*p->get_dependencies()->at(i).get_condition()).c_str());
		node.getChildNode("dependencies").getChildNode("dep", i).addChild("version");
		node.getChildNode("dependencies").getChildNode("dep", i).getChildNode("version").addText(p->get_dependencies()->at(i).get_package_version()->c_str());
	}
	node.addChild("tags");
	node.addChild("changelog");

	for (unsigned int i=0; i<p->get_tags()->size(); i++)
	{
		node.getChildNode("tags").addChild("tag");
		node.getChildNode("tags").getChildNode("tag",i).addText(p->get_tags()->at(i).c_str());
	}

	node.getChildNode("changelog").addText(p->get_changelog()->c_str());
	node.addChild("maintainer");
	node.getChildNode("maintainer").addChild("name");
	node.getChildNode("maintainer").getChildNode("name").addText(p->get_packager()->c_str());
	node.getChildNode("maintainer").addChild("email");
	node.getChildNode("maintainer").getChildNode("email").addText(p->get_packager_email()->c_str());
	
	node.addChild("configfiles");
	for (unsigned int i=0; i<p->get_config_files()->size(); i++)
	{
		node.getChildNode("configfiles").addChild("conffile");
		node.getChildNode("configfiles").getChildNode("conffile",i).addText(p->get_config_files()->at(i).get_name()->c_str());
	}
	node.addChild("tempfiles");
	for (unsigned int i=0; i<p->get_temp_files()->size(); i++)
	{
		node.getChildNode("tempfiles").addChild("tempfile");
		node.getChildNode("tempfiles").getChildNode("tempfile",i).addText(p->get_config_files()->at(i).get_name()->c_str());
	}
	node.addChild("mbuild");
	if (!pc->getBuildUrl().empty())
	{
		node.getChildNode("mbuild").addChild("url");
		node.getChildNode("mbuild").getChildNode("url").addText(pc->getBuildUrl().c_str());
	}
	if (!pc->getBuildPatchList().empty())
	{
		node.getChildNode("mbuild").addChild("patches");
		for (unsigned int i=0; i<pc->getBuildPatchList().size(); i++)
		{
			node.getChildNode("mbuild").getChildNode("patches").addChild("patch");
			node.getChildNode("mbuild").getChildNode("patches").getChildNode("patch",i).addText(pc->getBuildPatchList().at(i).c_str());
		}
	}
	if (!pc->getBuildSourceRoot().empty())
	{
		node.getChildNode("mbuild").addChild("sources_root_directory");
		node.getChildNode("mbuild").getChildNode("sources_root_directory").addText(pc->getBuildSourceRoot().c_str());
	}
	if (!pc->getBuildSystem().empty())
	{
		node.getChildNode("mbuild").addChild("build_system");
		{
			node.getChildNode("mbuild").getChildNode("build_system").addText(pc->getBuildSystem().c_str());
		}
	}
	if (!pc->getBuildMaxNumjobs().empty())
	{
		node.getChildNode("mbuild").addChild("max_numjobs");
		node.getChildNode("mbuild").getChildNode("max_numjobs").addText(pc->getBuildMaxNumjobs().c_str());
	}
		
	node.getChildNode("mbuild").addChild("optimization");
	if (!pc->getBuildOptimizationMarch().empty()) {
		node.getChildNode("mbuild").getChildNode("optimization").addChild("march");
		node.getChildNode("mbuild").getChildNode("optimization").getChildNode("march").addText(pc->getBuildOptimizationMarch().c_str());
	}
	if (!pc->getBuildOptimizationMtune().empty()) {
		node.getChildNode("mbuild").getChildNode("optimization").addChild("mtune");
		node.getChildNode("mbuild").getChildNode("optimization").getChildNode("mtune").addText(pc->getBuildOptimizationMtune().c_str());
	}
	if (!pc->getBuildOptimizationLevel().empty()) {
		node.getChildNode("mbuild").getChildNode("optimization").addChild("olevel");
		node.getChildNode("mbuild").getChildNode("optimization").getChildNode("olevel").addText(pc->getBuildOptimizationLevel().c_str());
	}
	if (!pc->getBuildOptimizationCustomGccOptions().empty()) {
		node.getChildNode("mbuild").getChildNode("optimization").addChild("custom_gcc_options");
		node.getChildNode("mbuild").getChildNode("optimization").getChildNode("custom_gcc_options").addText(pc->getBuildOptimizationCustomGccOptions().c_str());
	}

	node.getChildNode("mbuild").getChildNode("optimization").addChild("allow_change");
	if (!pc->getBuildOptimizationCustomizable()) {
		node.getChildNode("mbuild").getChildNode("optimization").getChildNode("allow_change").addText("true");
	}
	else {
		node.getChildNode("mbuild").getChildNode("optimization").getChildNode("allow_change").addText("false");
	}

	if (!pc->getBuildConfigureEnvOptions().empty())
	{
		node.getChildNode("mbuild").addChild("env_options");
		node.getChildNode("mbuild").getChildNode("env_options").addText(pc->getBuildConfigureEnvOptions().c_str());
	}
	if (!pc->getBuildKeyNames().empty())
	{
		node.getChildNode("mbuild").addChild("configuration");
	
		for (unsigned int i=0; i<pc->getBuildKeyNames().size(); i++)
		{
			node.getChildNode("mbuild").getChildNode("configuration").addChild("key");
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).addChild("name");
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).getChildNode("name").addText(pc->getBuildKeyNames().at(i).c_str());
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).addChild("value");
			node.getChildNode("mbuild").getChildNode("configuration").getChildNode("key",i).getChildNode("value").addText(pc->getBuildKeyValues().at(i).c_str());
		}
	}

	if (!pc->getBuildCmdConfigure().empty() || !pc->getBuildCmdMake().empty() || !pc->getBuildCmdMakeInstall().empty())
	{
		node.getChildNode("mbuild").addChild("custom_commands");
		if (!pc->getBuildCmdConfigure().empty())
		{
			node.getChildNode("mbuild").getChildNode("custom_commands").addChild("configure");
			node.getChildNode("mbuild").getChildNode("custom_commands").getChildNode("configure").addText(pc->getBuildCmdConfigure().c_str());
		}
		if (!pc->getBuildCmdMake().empty())
		{
			node.getChildNode("mbuild").getChildNode("custom_commands").addChild("make");
			node.getChildNode("mbuild").getChildNode("custom_commands").getChildNode("make").addText(pc->getBuildCmdMake().c_str());
		}
		if (!pc->getBuildCmdMakeInstall().empty())
		{
			node.getChildNode("mbuild").getChildNode("custom_commands").addChild("make_install");
			node.getChildNode("mbuild").getChildNode("custom_commands").getChildNode("make_install").addText(pc->getBuildCmdMakeInstall().c_str());
		}
	}


	node.writeToFile(filename.c_str());
}

void generateDeps(string tgz_filename)
{
	if (tgz_filename.empty()) {
		mError("No filename specified");
		return;
	}
	say("Generating dependencies for %s\n",tgz_filename.c_str());
	string current_dir = (string) get_current_dir_name();
	// Create a temporary directory
	string tmpdir = get_tmp_file();
	string dep_out = get_tmp_file();
	unlink(tmpdir.c_str());
	system("mkdir -p " + tmpdir);
	say("Extracting\n");
	system("tar zxf " + tgz_filename + " -C " + tmpdir);
	PackageConfig *p = new PackageConfig(tmpdir+"/install/data.xml");
	PACKAGE pkg;
	if (p->parseOk) xml2package(p->getXMLNode(), &pkg);
	delete p;
	say("Building dependencies\n");
	
	system("env LC_ALL=C requiredbuilder -n -v " + tgz_filename + " > "+ dep_out);
	vector<string> data = ReadFileStrings(dep_out);
	
	string tmp;
	string tail;
	DEPENDENCY d;
	//pkg.get_dependencies()->clear();
	string condptr;
	for (unsigned int i=0; i<data.size(); i++)
	{
		tmp = data[i].substr(0,data[i].find_first_of(" "));
		tail = data[i].substr(tmp.length()+1);
		d.set_package_name(&tmp);

		tmp = tail.substr(0, tail.find_first_of(" "));
		tail = tail.substr(tmp.length()+1);
		condptr=IntToStr(condition2int(hcondition2xml(tmp)));
		d.set_condition(&condptr);

		tmp = tail.substr(0,tail.find_first_of("-"));
		d.set_package_version(&tmp);
		if (*d.get_package_name()!=*pkg.get_name()) {
			// Checking existing dependencies
			bool added=false;
			for (unsigned int t=0; t<pkg.get_dependencies()->size(); t++) {
				if (*d.get_package_name()==*pkg.get_dependencies()->at(t).get_package_name()) {
					pkg.get_dependencies()->at(t) = d;
					added=true;
				}
			}
			if (!added) pkg.get_dependencies()->push_back(d);
		}
	}
	say(_("Got %d dependencies\n"), pkg.get_dependencies()->size());
	p = new PackageConfig(tmpdir+"/install/data.xml");
	dumpPackage(&pkg, p, tmpdir+"/install/data.xml");
	delete p;
	if (tgz_filename[0]!='/') tgz_filename = current_dir + "/"+getDirectory(tgz_filename);
	system ("cd " + tmpdir + "; buildpkg " + tgz_filename );
	system("rm -rf " + tmpdir);
	delete_tmp_files();
}

