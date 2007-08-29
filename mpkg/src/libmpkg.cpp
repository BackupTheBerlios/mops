/*********************************************************************
 * MOPSLinux packaging system: library interface
 * $Id: libmpkg.cpp,v 1.53 2007/08/29 22:33:13 i27249 Exp $
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
int mpkg::build_package()
{
	return mpkgSys::build_package();
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
int mpkg::install(vector<string> fname)
{
	int ret=0;
	for (unsigned int i = 0; i < fname.size(); i++)
	{
		currentStatus = _("Building queue: ")+IntToStr(i) + "/" +IntToStr(fname.size()) +" ["+fname[i]+"]";
		if (mpkgSys::requestInstall(fname[i], db, DepTracker)!=0) ret--;
	}
	//currentStatus = "Installation complete";
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
	return mpkgSys::requestInstall(fname, db, DepTracker);
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
int mpkg::update_repository_data()
{
	if (mpkgSys::update_repository_data(db) == 0 && db->sqlFlush() == 0)
	{
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
int mpkg::get_packagelist(SQLRecord *sqlSearch, PACKAGE_LIST *packagelist)
{
	currentStatus = _("Retrieving package list...");
	int ret = db->get_packagelist(sqlSearch, packagelist);
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
		//mpkgErrorReturn errRet = waitResponce(MPKG_DEPENDENCY_ERROR);
		
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
	PACKAGE_LIST allPackages;
	SQLRecord sqlSearch;
//	sqlSearch.addField("package_installed", 1);
	get_packagelist(&sqlSearch, &allPackages);
	PACKAGE *p;
	mstring data;
	say("Received %d packages\n",allPackages.size());
	for (int i=0; i<allPackages.size(); i++)
	{
		data.clear();
		p = allPackages.get_package(i);
		say("[%d/%d] Exporting package %s\n",i+1,allPackages.size(),p->get_name()->c_str());
		data = "PACKAGE NAME:\t" + *p->get_name() +"-"+*p->get_version()+"-"+*p->get_arch()+"-"+*p->get_build() +\
			"\nCOMPRESSED PACKAGE SIZE:\t"+*p->get_compressed_size()+ \
			"\nUNCOMPRESSED PACKAGE SIZE:\t"+*p->get_installed_size()+\
			"\nPACKAGE LOCATION:\t/var/log/mount/"+*p->get_filename()+\
			"\nPACKAGE DESCRIPTION:\n" + *p->get_name() + ":  " + *p->get_short_description()+\
			"\nFILE LIST:\n";
		db->get_filelist(p->get_id(), p->get_files());
		for (unsigned int f=0; f<p->get_files()->size(); f++)
		{
			data+=*p->get_files()->at(f).get_name()+"\n";
		}
		data+="\n";
		WriteFile(output_dir+"/"+*p->get_name()+"-"+*p->get_version()+"-"+*p->get_arch()+"-"+*p->get_build(), data.s_str());
	}
}

void dumpPackage(PACKAGE *p, string filename)
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
	node.writeToFile(filename.c_str());
}

void generateDeps(string tgz_filename)
{
	say("Generating dependencies for %s\n",tgz_filename.c_str());
	string current_dir = (string) get_current_dir_name();
	// Create a temporary directory
	string tmpdir = get_tmp_file();
	string dep_out = get_tmp_file();
	//say("Creating temp directory in %s\n", tmpdir.c_str());
	unlink(tmpdir.c_str());
	system("mkdir -p " + tmpdir);
	say("Extracting\n");
	system("tar zxf " + tgz_filename + " -C " + tmpdir);
	//say("Importing data\n");
	PackageConfig p(tmpdir+"/install/data.xml");
	PACKAGE pkg;
	if (p.parseOk) xml2package(p.getXMLNode(), &pkg);
	say("Building dependencies\n");
	
	system("requiredbuilder -n -v " + tgz_filename + " > "+ dep_out);
	//say("Parsing\n");
	vector<string> data = ReadFileStrings(dep_out);
	
	string tmp;
	string tail;
	DEPENDENCY d;
	pkg.get_dependencies()->clear();
	string condptr;
	for (unsigned int i=0; i<data.size(); i++)
	{
	//	printf("parse cycle %d start\n",i);
		tmp = data[i].substr(0,data[i].find_first_of(" "));
		tail = data[i].substr(tmp.length()+1);
	//	printf("dep name = [%s]\n",tmp.c_str());
		d.set_package_name(&tmp);

		tmp = tail.substr(0, tail.find_first_of(" "));
	//	printf("tmp = [%s]\n", tmp.c_str());
		tail = tail.substr(tmp.length()+1);
	//	printf("dep condition = [%s]\n", tmp.c_str());
		condptr=IntToStr(condition2int(hcondition2xml(tmp)));
		d.set_condition(&condptr);

		tmp = tail.substr(0,tail.find_first_of("-"));
	//	printf("dep version = [%s]\n", tmp.c_str());
		d.set_package_version(&tmp);
		pkg.get_dependencies()->push_back(d);
	}
	dumpPackage(&pkg, tmpdir+"/install/data.xml");
	system ("cd " + tmpdir + "; buildpkg; mv *.tgz " + current_dir + "/" +tgz_filename );
	system("rm -rf " + tmpdir);
	delete_tmp_files();
}

