/***********************************************************************
 * 	$Id: mpkg.cpp,v 1.132 2007/12/11 05:38:29 i27249 Exp $
 * 	MOPSLinux packaging system
 * ********************************************************************/
#include "mpkg.h"
#include "syscommands.h"
#include "DownloadManager.h"
#include <iostream>
#include "dialog.h"
#include "package.h"
int emerge_package(string file_url, string *package_name, string march, string mtune, string olevel, string *builddir_name_ret)
{

	string PACKAGE_OUTPUT = mConfig.getValue("package_output");
	if (PACKAGE_OUTPUT.empty()) {
		mConfig.setValue("package_output", "/var/mpkg/packages");
		PACKAGE_OUTPUT = mConfig.getValue("package_output");
	}
	*package_name="";
	SourcePackage spkg;
	if (!spkg.setInputFile(file_url))
	{
		mError("Source build: file not found");
		return -1;
	}
	if (!spkg.unpackFile())
	{
		mError("Source build: error extracting package");
		return -2;
	}

	PackageConfig p(spkg.getDataFilename());
	if (!p.parseOk) {
		mError("Source build: invalid XML data");
		return -3;
	}

	bool canCustomize = p.getBuildOptimizationCustomizable();
	bool useCflags = p.getBuildUseCflags();
	string build_system = p.getBuildSystem();

	if (canCustomize || build_system == "script") {
		// If we didn't receive an overrided architecture instructions, let's set defaults!
		if (march.empty() || mConfig.getValue("override_buildflags")=="yes") {
			march = mConfig.getValue("march");
			if (march.empty()) {
				march="i486";
				mConfig.setValue("march", "i486");
			}
		}
		if (mtune.empty() || mConfig.getValue("override_buildflags")=="yes") {
			mtune = mConfig.getValue("mtune");
			if (mtune.empty()) {
				mtune="i686";
				mConfig.setValue("mtune", "i686");
			}
		}

		if (olevel.empty() || mConfig.getValue("override_buildflags")=="yes") {
			olevel = mConfig.getValue("olevel");
			if (olevel.empty()) {
				olevel="O2";
				mConfig.setValue("olevel", "O2");
			}
		}
	}

	printf("Build flags:\nArchitecture: %s, tuned for: %s, optimization level: %s\n", march.c_str(), mtune.c_str(), olevel.c_str());
	string gcc_options = p.getBuildOptimizationCustomGccOptions();
	string configure_cmd = p.getBuildCmdConfigure();
	string make_cmd = p.getBuildCmdMake();
	string make_install_cmd = p.getBuildCmdMakeInstall();
	string script_cmd;

	// Setting input filename
	
	string filename, ext, tar_args;
	string cflags;
	if (!canCustomize) {
		if (!march.empty() || !mtune.empty() || !olevel.empty()) mWarning(_("Custom tuning options is not allowed for this package"));
	}
	if (!canCustomize || march.empty()) march=p.getBuildOptimizationMarch();
	if (!canCustomize || mtune.empty()) mtune=p.getBuildOptimizationMtune();
	if (!canCustomize || olevel.empty()) olevel=p.getBuildOptimizationLevel();
	if (useCflags)
	{
		cflags="'";
		if (!march.empty()) cflags += " -march="+march;
		if (!mtune.empty()) cflags += " -mtune=" +mtune;
		if (!olevel.empty()) cflags +=" -" + olevel;
		if (!gcc_options.empty()) cflags += " " + gcc_options;
		cflags +="'";
		if (cflags.length()<4) cflags.clear();
		else cflags = "CFLAGS=" + cflags + " CXXFLAGS=" + cflags;
	}
	cflags = p.getBuildConfigureEnvOptions() + " " + cflags;
	printf("CFLAGS: %s\n", cflags.c_str());

	string extractCommand;
	string url=p.getBuildUrl();
	say(_("Source url: [%s]\n"), url.c_str());
	if (url.find("cvs ")!=0 && url.find("svn ")!=0 && url.find("git-clone ")!=0) {
		filename=getFilename(url);
		ext = getExtension(url);
	
		if (ext=="bz2") tar_args="jxvf";
		if (ext=="gz" || ext == "tgz") tar_args="zxvf";
		if (ext!="bz2" && ext!="gz" && ext!="tgz" && ext!="zip" && ext!="rar") {
			mError("Unknown file extension " + ext);
			return 2;
		}
		if (ext=="zip") extractCommand = "unzip";
		if (ext=="rar") extractCommand = "unrar e";
		if (ext=="bz2" || ext=="gz" || ext=="tgz") extractCommand = "tar " + tar_args;
	}
	// Directories
	string currentdir=get_current_dir_name();	
	string pkgdir = "/tmp/package-"+p.getName();
	string dldir=spkg.getExtractedPath();
	// Setting source directory
	bool noSubfolder = p.getBuildNoSubfolder();

	// Setting output package name
	string pkg_name = p.getName()+"-"+p.getVersion()+"-"+p.getArch()+"-"+p.getBuild()+".tgz";
	string dl_command;
	if (url.find("cvs")==0 || url.find("svn")==0 || url.find("git")==0) dl_command = url;
	if (url.find("http://")==0 || url.find("ftp://")==0) dl_command = "wget " + url;
	if (dl_command.empty()) {
		if (url.find("/")==0) {
			if (FileExists(url)) dl_command="cp -v " + url + " .";
			else {
				mError(_("Source file doesn't exist, aborting"));
				return -5;
			}
		}
		else {
			if (FileExists(dldir+"/"+url)) dl_command="";
			else {
				mError(_("Source file doesn't exist"));
				return -5;
			}
		}
	}
	vector<string> key_names = p.getBuildKeyNames();
	vector<string> key_values = p.getBuildKeyValues();

	int max_numjobs = atoi(p.getBuildMaxNumjobs().c_str());
	string numjobs="4";
	if (max_numjobs<4 && max_numjobs !=0) numjobs = IntToStr(max_numjobs); // Temp solution


	string configure_options;
	for (unsigned int i=0; i<key_names.size(); i++)
	{
		configure_options += " " + key_names[i];
		if (!key_values[i].empty()) {
			configure_options += "=" + key_values[i];
		}
	}
	// Preparing environment. Clearing old directories
	system("rm -rf " + pkgdir);	
	system("mkdir " + pkgdir + "; cp -R " + dldir+"/install " + pkgdir+"/");
	if (march != p.getArch() && p.getArch()!="noarch") {
		// Writing new XML data (fixing arch)
		string xml_path = pkgdir+"/install/data.xml";
		string xmldata = ReadFile(xml_path);
		if (xmldata.find("<arch>")==std::string::npos || xmldata.find("</arch>")==std::string::npos) {
			mError("parse error");
			system("echo " + p.getName() + " >> /tmp/failed_list");
			return -1;
		}
		xmldata = xmldata.substr(0,xmldata.find("<arch>")+strlen("<arch>")) + march + xmldata.substr(xmldata.find("</arch>"));
		WriteFile(xml_path, xmldata);
		//printf("%s\n", xmldata.c_str());
		xmldata.clear();
	}

	
	

	string srcCacheDir = mConfig.getValue("source_cache_dir");
	if (srcCacheDir.empty()) {
		mConfig.setValue("source_cache_dir", "/var/mpkg/source_cache/");
		srcCacheDir = mConfig.getValue("source_cache_dir");
	}

	srcCacheDir = srcCacheDir+"/" + pkg_name;
	if (FileExists(dldir+"/"+getFilename(url))) dl_command.clear();
	if (!useBuildCache) system("rm -rf " + srcCacheDir);
	if (useBuildCache && FileExists(srcCacheDir+"/"+getFilename(url))) dl_command.clear();
	// Downloading/copying sources
	// First, download in cache.
	system("mkdir -p " + srcCacheDir);
	if (!dl_command.empty()) {
		if (system("(cd " + srcCacheDir+"; " + dl_command+")")!=0) {
			mError("Error retrieving sources, build failed");
			system("rm -rf " + srcCacheDir);
			return -6;
		}
	}
	if (!FileExists(dldir+"/"+getFilename(url))) {
		// Copy everything to build directory
		if (system("cp -R " + srcCacheDir+"/* " + dldir+"/")!=0) {
			mError("Error getting sources from cache, failure");
			return -112;
		}
	}
	string extraCmd;
	if (noSubfolder)
	{
		extraCmd = "mkdir -p extracted_source; cd extracted_source; ";
	}
	// Extracting the sources, if need so
	if (url.find("cvs ")!=0 && url.find("svn ")!=0 && url.find("git-clone ")!=0) {
		if (system("(cd " + dldir+"; " + extraCmd + extractCommand + " " + filename+")")!=0) {
			mError(_("Tar was failed to extract the received source package"));
			return -7;
		}
	}

	string srcdir;
	if (!noSubfolder)
	{

		srcdir=p.getBuildSourceRoot(); // Trying to get source directory from config
		if (srcdir.empty()) srcdir = spkg.getSourceDirectory(); // Trying to auto-detect directory by analyzing directory tree
		if (srcdir.empty()) { // If all of above was failed, try to determine directory name by package name
			if (ext=="bz2") srcdir=dldir+"/"+filename.substr(0,filename.length()-strlen(".tar.bz2"));
			if (ext=="gz") srcdir=dldir+"/"+filename.substr(0,filename.length()-strlen(".tar.gz"));
			if (ext=="tgz") srcdir=dldir+"/"+filename.substr(0,filename.length()-strlen(".tgz"));
			if (ext=="zip") srcdir=dldir+"/"+filename.substr(0,filename.length()-strlen(".zip"));
			if (ext=="rar") srcdir=dldir+"/"+filename.substr(0,filename.length()-strlen(".rar"));
		}
		else srcdir = dldir+"/"+srcdir;
	}
	else srcdir = dldir+"/extracted_source/";

	if (builddir_name_ret!=NULL) *builddir_name_ret = srcdir;

	if (FileExists(dldir+"/build_data/build.sh")) {

		script_cmd = "VERSION=" + p.getVersion()+ " DATADIR=" + dldir+"/build_data/" + " PKG="+pkgdir + " SRC=" + srcdir + " sh " + dldir+"/build_data/build.sh " + srcdir + " " + pkgdir + " " + march + " " + mtune + " " + olevel;
	}

	if (build_system=="autotools")
	{
		configure_cmd="./configure " + configure_options;
		make_cmd = "make";
		make_install_cmd="make install DESTDIR=$DESTDIR";
	}
	if (build_system=="cmake")
	{
		configure_cmd = "cmake .. " + configure_options;
		make_cmd = "make";
		make_install_cmd="make install DESTDIR=$DESTDIR";
	}
	if (build_system=="scons")
	{
		configure_cmd = "scons " + configure_options;
		make_cmd = "make";
		make_install_cmd = "make install DESTDIR=$DESTDIR";
	}
	if (build_system=="custom")
	{
		if (configure_cmd.find("$OPTIONS")!=std::string::npos) 
			configure_cmd=configure_cmd.substr(0,configure_cmd.find("$OPTIONS"))+ " " +configure_options + " " + configure_cmd.substr(configure_cmd.find("$OPTIONS")+strlen("$OPTIONS"));
		if (configure_cmd.find("$ENV")!=std::string::npos) 
			configure_cmd=configure_cmd.substr(0,configure_cmd.find("$ENV"))+ " " +cflags + " " + configure_cmd.substr(configure_cmd.find("$ENV")+strlen("$ENV"));

		printf("Using custom commands");
	}
	if (build_system=="script")
	{
		printf("Running script\n");
		configure_cmd.clear();
		make_cmd.clear();
		make_install_cmd.clear();
	}

	if (!make_cmd.find("make")!=std::string::npos && !numjobs.empty()) make_cmd += " -j" + numjobs;
	if (!script_cmd.empty()) {
		if (!make_cmd.empty()) {
			make_cmd += " && " + script_cmd;
		}
		else make_cmd = script_cmd;
	}
	

	while (make_install_cmd.find("$DESTDIR")!=std::string::npos)
	{
		make_install_cmd=make_install_cmd.substr(0,make_install_cmd.find("$DESTDIR"))+ pkgdir+ make_install_cmd.substr(make_install_cmd.find("$DESTDIR")+strlen("$DESTDIR"));
	}
	while (make_install_cmd.find("$SRCDIR")!=std::string::npos)
	{
		make_install_cmd=make_install_cmd.substr(0,make_install_cmd.find("$SRCDIR"))+ srcdir+ make_install_cmd.substr(make_install_cmd.find("$SRCDIR")+strlen("$SRCDIR"));
	}
	while (make_install_cmd.find("$DATADIR")!=std::string::npos)
	{
		make_install_cmd=make_install_cmd.substr(0,make_install_cmd.find("$DATADIR")) + dldir+"/build_data/"+ make_install_cmd.substr(make_install_cmd.find("$DATADIR")+strlen("$DATADIR"));
	}


	// Fixing permissions (mozgmertv resistance)
	say(_("\nChecking and fixing permissions...\n"));
	system("(cd " + srcdir+"; chown -R root:root .;	find . -perm 666 -exec chmod 644 {} \\;; find . -perm 664 -exec chmod 644 {} \\;; find . -perm 600 -exec chmod 644 {} \\;; find . -perm 444 -exec chmod 644 {} \\;; find . -perm 400 -exec chmod 644 {} \\;; find . -perm 440 -exec chmod 644 {} \\;; find . -perm 777 -exec chmod 755 {} \\;; find . -perm 775 -exec chmod 755 {} \\;; find . -perm 511 -exec chmod 755 {} \\;; find . -perm 711 -exec chmod 755 {} \\;; find . -perm 555 -exec chmod 755 {} \\;)");
	

	vector<string> patchList = p.getBuildPatchList();

	string compile_cmd;
	if (build_system=="script") make_cmd = cflags + " " + make_cmd;
	bool was_prev=false;
	if (build_system!="cmake")
	{
		compile_cmd = "(cd " + srcdir + "; ";
		if (!configure_cmd.empty()) 
		{
			if (build_system!="custom") compile_cmd += cflags + " " + configure_cmd;
			else compile_cmd += configure_cmd;
			was_prev=true;
		}
	}
	else {
		compile_cmd = "(cd " + srcdir + "; mkdir -p build; cd build; "+cflags + " cmake ..; ";
	}
	if (!make_cmd.empty())	{
		if (was_prev) compile_cmd += " && ";
		compile_cmd += make_cmd;
		was_prev=true;
	}
	if (!make_install_cmd.empty()) 
	{
		if (was_prev) compile_cmd += " && ";
		compile_cmd += make_install_cmd;
		was_prev=true;
	}
	compile_cmd+=")";

	// Patching if any
	for (unsigned int i=0; i<patchList.size(); i++)
	{
		system("(cd " + srcdir + "; zcat ../patches/" + patchList[i] + " | patch -p1 --verbose)");
	}
	// Compiling
	printf("Compilation command: %s\n", compile_cmd.c_str());
	if (system(compile_cmd)!=0) {
		mError("Build failed. Check the build config");
		return -7;
	}
	// Man compression and binary stripping
	system("( cd " + pkgdir + "; find . | xargs file | grep \"executable\" | grep ELF | cut -f 1 -d : | xargs strip --strip-unneeded 2> /dev/null; find . | xargs file | grep \"shared object\" | grep ELF | cut -f 1 -d : | xargs strip --strip-unneeded 2> /dev/null; if [ -d usr/man ]; then gzip -9 usr/man/man?/*; fi )");
	// Fixing permissions
	system("(cd " + pkgdir+"; mkdir -p " + (string) PACKAGE_OUTPUT + "; buildpkg " + (string) PACKAGE_OUTPUT +"/)");
	if (p.getArch()=="noarch") march="noarch";
	pkg_name = p.getName()+"-"+p.getVersion()+"-"+march+"-"+p.getBuild()+".tgz";
	*package_name=(string) PACKAGE_OUTPUT+"/"+pkg_name;
	if (autogenDepsMode == ADMODE_MOZGMERTV) generateDeps(*package_name, true);
	// If all OK, clean up
	spkg.clean();
	return 0;
}

mpkgDatabase::mpkgDatabase()
{
	hasFileList=false;
}
mpkgDatabase::~mpkgDatabase(){}

int mpkgDatabase::sqlFlush()
{
	return db.sqlFlush();
}

PACKAGE mpkgDatabase::get_installed_package(string* pkg_name)
{
	PACKAGE_LIST packagelist;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_name", pkg_name);
	sqlSearch.addField("package_installed", ST_INSTALLED);

	get_packagelist(&sqlSearch, &packagelist);
	// We do NOT allow multiple packages with same name to be installed, so, we simply get first package of list.
	
	if (packagelist.size()>0)
		return *packagelist.get_package(0);
	else
	{
		PACKAGE ret;
		return ret;
	}
}
	

int mpkgDatabase::emerge_to_db(PACKAGE *package)
{
	mDebug("mpkgDatabase::emerge_to_db()");
	int pkg_id;
	pkg_id=get_package_id(package);
	if (pkg_id==0)
	{
		// New package, adding
		add_package_record(package);
		return 0;
	}
	if (pkg_id<0)
	{
		// Query error
		return MPKGERROR_CRITICAL;
	}
	// Раз пакет уже в базе (и в единственном числе - а иначе и быть не должно), сравниваем данные.
	// В случае необходимости, добавляем location.
	PACKAGE db_package;
	vector<LOCATION> new_locations;
	get_package(pkg_id, &db_package);
	package->set_id(pkg_id);

	for (unsigned int j=0; j<package->get_locations()->size(); j++)
	{
		for (unsigned int i=0; i<db_package.get_locations()->size(); i++)
		{
			if (!package->get_locations()->at(j).equalTo(&db_package.get_locations()->at(i)))
			{
				new_locations.push_back(package->get_locations()->at(j));
			}
		}
	}
	if (!new_locations.empty()) add_locationlist_record(pkg_id, &new_locations);
	return 0;
}


bool mpkgDatabase::check_cache(PACKAGE *package, bool clear_wrong)
{

	string fname = SYS_CACHE + "/" + *package->get_filename();
	if (package->usedSource.find("cdrom://")!=std::string::npos && FileExists(fname)) return true;
	string got_md5 = get_file_md5(SYS_CACHE + "/" + *package->get_filename());
	if (FileExists(fname) && *package->get_md5() == got_md5)
		return true;
	else
	{
		if (clear_wrong) unlink(fname.c_str());
		return false;
	}
}


int mpkgDatabase::commit_actions()
{
	mDebug("Committing");
	Dialog dialogItem;
	delete_tmp_files();
	sqlFlush();
	// Zero: purging required packages
	// First: removing required packages
	unsigned int installFailures=0;
	unsigned int removeFailures=0;
	PACKAGE_LIST remove_list;
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", ST_REMOVE);
	sqlSearch.addField("package_action", ST_PURGE);
	if (get_packagelist(&sqlSearch, &remove_list)!=0) return MPKGERROR_SQLQUERYERROR;
	if (!remove_list.IsEmpty()) remove_list.sortByPriority(true);
	PACKAGE_LIST install_list;
	sqlSearch.clear();
	sqlSearch.setSearchMode(SEARCH_OR);
	sqlSearch.addField("package_action", ST_INSTALL);
	sqlSearch.addField("package_action", ST_REPAIR);
	if (get_packagelist(&sqlSearch, &install_list)!=0) return MPKGERROR_SQLQUERYERROR;
	if (!install_list.IsEmpty()) install_list.sortByPriority();
	if (dialogMode) dialogItem.execInfoBox("Подготовка к установке пакетов");
	install_list.sortByLocations();
	// Checking available space
	//printf("checking available space\n");
	long double rem_size=0;
	long double ins_size=0;
	
	for (int i=0; i<remove_list.size(); i++)
	{
		remove_list.get_package(i)->itemID = pData.addItem(*remove_list.get_package(i)->get_name(), 10);
		rem_size+=strtod(remove_list.get_package(i)->get_installed_size()->c_str(), NULL);
		// Also, checking for update
		for (int t=0; t<install_list.size(); t++) {
			if (*install_list.get_package(t)->get_name() == *remove_list.get_package(t)->get_name()) {
				remove_list.get_package(t)->set_action(ST_UPDATE);
				remove_list.get_package(t)->updatingBy=install_list.get_package(t);
			}
		}
	}
	// From now on, all packages in remove group who will be updated, has action ST_UPDATE
	for (int i=0; i<install_list.size(); i++)
	{
		install_list.get_package(i)->itemID = pData.addItem(*install_list.get_package(i)->get_name(), atoi(install_list.get_package(i)->get_compressed_size()->c_str()));
		ins_size += strtod(install_list.get_package(i)->get_installed_size()->c_str(), NULL);
	}
	long double freespace = get_disk_freespace(SYS_ROOT);
	mDebug("Freespace is " + IntToStr((long long) freespace));
	
	if (freespace < (ins_size - rem_size))
	{
		mDebug("Not enough free space, show warning");
		if (dialogMode)
		{
			Dialog dialogItem;
			if (!dialogItem.execYesNo("Судя по всему, для установки места на диске не хватит.\nНа корневой файловой системе " + SYS_ROOT+" имеется " + humanizeSize(freespace) + "\nДля установки требуется: " + humanizeSize(ins_size - rem_size) + "\nВсе равно продолжить?"))
			{
				return MPKGERROR_COMMITERROR;
			}
		}
		else mError(_("Seems that free space is not enough to install. Required: ") + humanizeSize(ins_size - rem_size) + _(", available: ") + humanizeSize(freespace));
	}
	else mDebug("Freespace OK, proceeding");

	// Let's show the summary for console and dialog users and ask for confirmation
	if (consoleMode && !dialogMode)
	{
		unsigned int installCount = 0, removeCount = 0, purgeCount = 0, repairCount = 0, updateCount = 0;
		say("Action summary:\n");

		// Install
		for (int i=0; i<install_list.size(); i++) {
			if (install_list.get_package(i)->action()==ST_INSTALL) {
				if (installCount==0) say(_("Will be installed:\n"));
				installCount++;
				say("  [%d] %s %s\n", installCount, \
						install_list.get_package(i)->get_name()->c_str(), \
						install_list.get_package(i)->get_fullversion().c_str());
			}
		}
		// Remove
		for (int i=0; i<remove_list.size(); i++) {
			if (remove_list.get_package(i)->action()==ST_REMOVE) {
				if (removeCount==0) say(_("Will be removed:\n"));
				removeCount++;
				say("  [%d] %s %s\n", removeCount, \
						remove_list.get_package(i)->get_name()->c_str(), \
						remove_list.get_package(i)->get_fullversion().c_str());
			}
		}
		// Purge
		for (int i=0; i<remove_list.size(); i++) {
			if (remove_list.get_package(i)->action()==ST_PURGE) {
				if (purgeCount==0) say(_("Will be purged:\n"));
				purgeCount++;
				say("  [%d] %s %s\n", purgeCount, \
						remove_list.get_package(i)->get_name()->c_str(), \
						remove_list.get_package(i)->get_fullversion().c_str());
			}
		}
		// Update
		for (int i=0; i<remove_list.size(); i++) {
			if (remove_list.get_package(i)->action()==ST_UPDATE) {
				if (updateCount==0) say(_("Will be updated:\n"));
				updateCount++;
				say("  [%d] %s %s\n", updateCount, \
						remove_list.get_package(i)->get_name()->c_str(), \
						remove_list.get_package(i)->get_fullversion().c_str());
			}
		}
		// Repair
		for (int i=0; i<install_list.size(); i++) {
			if (install_list.get_package(i)->action()==ST_REPAIR) {
				if (repairCount==0) say(_("Will be repaired:\n"));
				repairCount++;
				say("  [%d] %s %s\n", repairCount, \
						install_list.get_package(i)->get_name()->c_str(), \
						install_list.get_package(i)->get_fullversion().c_str());
			}
		}

		if (install_list.size()>0 || remove_list.size()>0)
		{
			if (interactive_mode)
			{
				say(_("Continue? [Y/n]\n"));
				string input;
i_actInput:
				input=cin.get();
				if (input=="n" || input=="N" || input == "no") { return MPKGERROR_ABORTED; }
				if (input!="y" && input!="Y" && input!="yes" && input!="\n") {
					say(_("Please answer Y (yes) or N (no)\n"));
					goto i_actInput;
				}
			}
		}
		else 
		{
			say (_("Nothing to do\n"));
			return 0;
		}

	}

	// Building action list
	mDebug("Building action list");
	actionBus.clear();
	if (install_list.size()>0)
	{
		actionBus.addAction(ACTIONID_CACHECHECK);
		actionBus.setSkippable(ACTIONID_CACHECHECK, true);
		actionBus.setActionProgressMaximum(ACTIONID_CACHECHECK, install_list.size());
		actionBus.addAction(ACTIONID_DOWNLOAD);
		actionBus.setActionProgressMaximum(ACTIONID_DOWNLOAD, install_list.size());
		actionBus.addAction(ACTIONID_MD5CHECK);
		actionBus.setSkippable(ACTIONID_MD5CHECK, true);
		actionBus.setActionProgressMaximum(ACTIONID_MD5CHECK, install_list.size());
	}
	if (remove_list.size()>0)
	{
		actionBus.addAction(ACTIONID_REMOVE);
		actionBus.setActionProgressMaximum(ACTIONID_REMOVE, remove_list.size());
	}
	if (install_list.size()>0)
	{
		actionBus.addAction(ACTIONID_INSTALL);
		actionBus.setActionProgressMaximum(ACTIONID_INSTALL, install_list.size());

	}
	// Done

	currentStatus = _("Looking for install queue");

	if (install_list.size()>0)
	{
		actionBus.setCurrentAction(ACTIONID_CACHECHECK);
		// Building download queue
		currentStatus = _("Looking for package locations...");
		DownloadsList downloadQueue;
		DownloadItem tmpDownloadItem;
		vector<string> itemLocations;
		//double totalDownloadSize=0;
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
		pData.setCurrentAction(_("Checking cache"));
		bool skip=false;
		if (dialogMode)
		{
			dialogItem.execGauge(_("Проверка пакетов"), 10,80,0);
		}

		for (int i=0; i<install_list.size(); i++)
		{
			if (dialogMode)
			{
				dialogItem.setGaugeValue((unsigned int) round((double) (i)/((double) (install_list.size())/(double) (100))));
			}
			delete_tmp_files();

			actionBus.setActionProgress(ACTIONID_CACHECHECK, i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				return MPKGERROR_ABORTED;
			}
			if (actionBus.skipped(ACTIONID_CACHECHECK))
			{
				skip=true;
			}

			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Checking package cache"));
			pData.setItemProgressMaximum(install_list.get_package(i)->itemID, 1);
			pData.setItemProgress(install_list.get_package(i)->itemID, 0);
	
			currentStatus = _("Checking cache and building download queue: ") + *install_list.get_package(i)->get_name();
	
	
			if (skip || !check_cache(install_list.get_package(i), false))
			{
				if (!skip) pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("not cached"));
				else pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("check skipped"));

				itemLocations.clear();
				
				tmpDownloadItem.expectedSize=strtod(install_list.get_package(i)->get_compressed_size()->c_str(), NULL);
				tmpDownloadItem.file = SYS_CACHE + *install_list.get_package(i)->get_filename();
				tmpDownloadItem.name = *install_list.get_package(i)->get_name();
				tmpDownloadItem.priority = 0;
				tmpDownloadItem.status = DL_STATUS_WAIT;
				tmpDownloadItem.itemID = install_list.get_package(i)->itemID;
				tmpDownloadItem.usedSource = &install_list.get_package(i)->usedSource;
	
				install_list.get_package(i)->sortLocations();
				for (unsigned int k = 0; k < install_list.get_package(i)->get_locations()->size(); k++)
				{
					itemLocations.push_back(*install_list.get_package(i)->get_locations()->at(k).get_server_url() \
						     + *install_list.get_package(i)->get_locations()->at(k).get_path() \
						     + *install_list.get_package(i)->get_filename());
	
				}
				tmpDownloadItem.url_list = itemLocations;
				downloadQueue.push_back(tmpDownloadItem);
			}
			else pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("cached"));
	
			pData.increaseItemProgress(install_list.get_package(i)->itemID);
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
		}
		if (dialogMode) dialogItem.closeGauge();
		actionBus.setActionState(ACTIONID_CACHECHECK);
		actionBus.setCurrentAction(ACTIONID_DOWNLOAD);
		mpkgErrorReturn errRet;
		bool do_download = true;
	
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
		while(do_download)
		{
			do_download = false;
			pData.downloadAction=true;

			if (CommonGetFileEx(downloadQueue, &currentItem) == DOWNLOAD_ERROR)
			{
				mError(_("Download failed"));
				if (!actionBus._abortActions) {
					errRet = waitResponce (MPKG_DOWNLOAD_ERROR);
					switch(errRet)
					{
						case MPKG_RETURN_IGNORE:
							say(_("Download errors ignored, continue installing\n"));
							goto installProcess;
							break;
				
						case MPKG_RETURN_RETRY:
							say(_("retrying...\n"));
							do_download = true;
							break;
						case MPKG_RETURN_ABORT:
							say(_("aborting...\n"));
							return MPKGERROR_ABORTED;
							break;
						default:
							mError(_("Unknown value, don't know what to do, aborting"));
							return MPKGERROR_ABORTED;
					}
				}
					
			}
		
		}
		actionBus.setActionState(ACTIONID_DOWNLOAD);
		pData.downloadAction=false;
		if (download_only) {
			say(_("Downloaded packages are stored in %s\n"), SYS_CACHE.c_str());
			mpkgSys::clean_queue(this);
			return 0;
		}
installProcess:
	
		actionBus.setCurrentAction(ACTIONID_MD5CHECK);
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
	
		//bool hasErrors=false;
		skip=false;
		currentStatus = _("Checking files (comparing MD5):");
		pData.setCurrentAction(_("Checking md5"));
		if (dialogMode)
		{
			dialogItem.execGauge(_("Checking packages integrity"), 10,80,0);
		}
		for (int i=0; i<install_list.size(); i++)
		{
			actionBus.setActionProgress(ACTIONID_MD5CHECK, i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_MD5CHECK, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			if (actionBus.skipped(ACTIONID_MD5CHECK) || forceSkipLinkMD5Checks) break;

			if (dialogMode)
			{
				dialogItem.setGaugeValue((unsigned int) round(((double)(i)/(double) ((double) (install_list.size())/(double) (100)))));

			}
			else say(_("Checking MD5 for %s\n"), install_list.get_package(i)->get_filename()->c_str());
			currentStatus = _("Checking md5 of downloaded files: ") + *install_list.get_package(i)->get_name();
	
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Checking md5"));
			pData.setItemProgressMaximum(install_list.get_package(i)->itemID, 1);
			pData.setItemProgress(install_list.get_package(i)->itemID, 0);
	
			if (!check_cache(install_list.get_package(i), true))
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("md5 incorrect"));
				pData.increaseItemProgress(install_list.get_package(i)->itemID);
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FAILED);
	
				errRet = waitResponce(MPKG_DOWNLOAD_ERROR);
				switch(errRet)
				{
					case MPKG_RETURN_IGNORE:
						say(_("Wrong checksum ignored, continuing...\n"));
						break;
					case MPKG_RETURN_RETRY:
						say(_("Re-downloading...\n"));
						break;
					case MPKG_RETURN_ABORT:
						say(_("Aborting installation\n"));
						return MPKGERROR_ABORTED;
						break;
					default:
						mError(_("Unknown reply, aborting"));
						return MPKGERROR_ABORTED;
						break;
				}
			}
			else
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, "MD5 OK");
				pData.increaseItemProgress(install_list.get_package(i)->itemID);
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}
		}
		if (dialogMode) dialogItem.closeGauge();
		actionBus.setActionState(ACTIONID_MD5CHECK);
	}

	mDebug("Building remove queue");
	if (remove_list.size()>0)
	{
		actionBus.setCurrentAction(ACTIONID_REMOVE);

		currentStatus = _("Looking for remove queue");
			mDebug ("Calling REMOVE for "+IntToStr(remove_list.size())+" packages");
		currentStatus = _("Removing ") + IntToStr(remove_list.size()) + _(" packages");
		pData.setCurrentAction("Removing packages");
	
		int removeItemID=0;
		for (int i=0; i<remove_list.size(); i++)
		{
			removeItemID=remove_list.get_package(i)->itemID;
			pData.setItemState(removeItemID, ITEMSTATE_WAIT);
			pData.setItemCurrentAction(removeItemID, _("Waiting"));
			pData.resetIdleTime(removeItemID);
			pData.setItemProgress(removeItemID, 0);
			pData.setItemProgressMaximum(removeItemID,8);
		}
		string _actionName;
		for (int i=0;i<remove_list.size();i++)
		{
			if (remove_list.get_package(i)->action()==ST_UPDATE) _actionName = _("Updating package");
			else _actionName = _("Removing package");
			
			if (dialogMode)
			{
				dialogItem.execGauge("[" + IntToStr(i+1) + "/" + IntToStr(remove_list.size()) + "] " + _actionName + " " + \
						*remove_list.get_package(i)->get_name() + "-" + \
						remove_list.get_package(i)->get_fullversion(), 10,80, \
						(unsigned int) round((double)(i)/(double)((double)(remove_list.size())/(double) (100))));
			}	
			delete_tmp_files();
			actionBus.setActionProgress(ACTIONID_REMOVE,i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_REMOVE, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);

			currentStatus = _actionName+" " + *remove_list.get_package(i)->get_name();
			if (remove_package(remove_list.get_package(i))!=0)
			{
				removeFailures++;
				pData.setItemCurrentAction(remove_list.get_package(i)->itemID, _("Remove failed"));
				pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_FAILED);
			}
			else
			{
				pData.setItemCurrentAction(remove_list.get_package(i)->itemID, _("Removed"));
				pData.setItemState(remove_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}

			if (dialogMode) 
				dialogItem.closeGauge();
		
		}
		sqlSearch.clear();

		clean_backup_directory();
		actionBus.setActionState(ACTIONID_REMOVE);
	} // Done removing packages


	if (install_list.size()>0)
	{	
		// Actually installing

		actionBus.setCurrentAction(ACTIONID_INSTALL);
		pData.resetItems(_("waiting"), 0, 1, ITEMSTATE_WAIT);
	
		pData.setCurrentAction(_("Installing packages"));
		int installItemID;
		for (int i=0; i<install_list.size(); i++)
		{
			installItemID=install_list.get_package(i)->itemID;
			pData.setItemState(installItemID, ITEMSTATE_WAIT);
			pData.setItemCurrentAction(installItemID, _("Waiting"));
			pData.resetIdleTime(installItemID);
			pData.setItemProgress(installItemID, 0);
			pData.setItemProgressMaximum(installItemID,8);
		}
		
		for (int i=0;i<install_list.size();i++)
		{
			
			actionBus.setActionProgress(ACTIONID_INSTALL, i);
			if (actionBus._abortActions)
			{
				sqlFlush();
				actionBus._abortComplete=true;
				actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
				return MPKGERROR_ABORTED;
			}
			pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_INPROGRESS);
			currentStatus = _("Installing package ") + *install_list.get_package(i)->get_name();
			if (dialogMode)
			{
				dialogItem.execGauge("[" + IntToStr(i+1) + "/" + IntToStr(install_list.size()) + _("] Installing: ") + \
						*install_list.get_package(i)->get_name() + "-" + \
						install_list.get_package(i)->get_fullversion(), 10,80, \
						(unsigned int) round((double) (i)/(double) ((double) (install_list.size())/(double) (100))));
			}
			if (install_package(install_list.get_package(i))!=0)
			{
				installFailures++;
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Installation failed"));
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FAILED);
				
				// change in 0.12.9: will stop installation if previous brokes...
				mError("Failed to install. Due to possible dependency issues, the installation procedure will stop now.");
				return MPKGERROR_COMMITERROR;
			}
			else
			{
				pData.setItemCurrentAction(install_list.get_package(i)->itemID, _("Installed"));
				pData.setItemState(install_list.get_package(i)->itemID, ITEMSTATE_FINISHED);
			}
		}
		currentStatus = _("Installation complete.");
		actionBus.setActionState(ACTIONID_INSTALL);
	}
	if (removeFailures!=0 && installFailures!=0) return MPKGERROR_COMMITERROR;
	actionBus.clear();
	if (install_list.size()>0) 
	{
		if (!dialogMode) say(_("Executing ldconfig\n"));
		system("ldconfig 2> /dev/null");
		say(_("Syncing disks...\n"));
		system("sync");
	}
	return 0;
}

int mpkgDatabase::install_package(PACKAGE* package)
{
	string sys_cache=SYS_CACHE;
	string sys_root=SYS_ROOT;

	pData.setItemCurrentAction(package->itemID, _("installing"));
	if (!dialogMode) say(_("Installing %s %s\n"),package->get_name()->c_str(), package->get_fullversion().c_str());
	string statusHeader = "["+IntToStr((int) actionBus.progress())+"/"+IntToStr((int)actionBus.progressMaximum())+"] "+_("Installing package ") + *package->get_name()+": ";
	currentStatus = statusHeader + _("initialization");
	// Checking if it is a symlink. If it is broken, and package installs from CD, ask to insert and mount
	bool broken_sym=false;
	mpkgErrorReturn errRet;
	//printf("Checking file existance\n");
	if (!FileExists(sys_cache + *package->get_filename(), &broken_sym) || broken_sym)
	{
		//printf("Link is broken. Looking for the package source\n");
		// Let's see what source is used
		if (package->usedSource.find("cdrom://")!=std::string::npos)
		{
			//printf("Yes, we are installing package from CD. Ejecting, mounting and checking the volume\n");
			system("eject " + CDROM_DEVICE);
			// Yeah, we used CD and there are symlink. Let's ask for appropriate disc
			// First, determine Volume ID
			string source;
		        source = package->usedSource.substr(strlen("cdrom://"));
			//printf("source created\n");
			string cdromVolName;
		        cdromVolName = source.substr(0,source.find_first_of("/"));
			//printf("cdromVolName created\n");
			bool mountedOk=false, abortMount=false;
			string recv_volname;
			while (!mountedOk)
			{
				//printf("Trying to mount\n");
				if (dialogMode) 
				{
					Dialog d("Установка пакета " + *package->get_name() + ": монтирование CD/DVD");
					if (d.execYesNo("Вставьте диск с меткой " + cdromVolName + " в привод " + CDROM_DEVICENAME))
					{
						system("umount " + CDROM_MOUNTPOINT);
						system("mount " + CDROM_DEVICE + " " + CDROM_MOUNTPOINT);
						recv_volname = getCdromVolname();
						if (recv_volname == cdromVolName)
						{
							mountedOk = true;
						}
						else d.execMsgBox("Вы вставили не тот диск.\nТребуется: ["+cdromVolName+"]\nВ дисководе: [" + recv_volname + "]\n");
					}
					else abortMount = true;
				}
				else
				{
					errRet = waitResponce(MPKG_CDROM_MOUNT_ERROR);
					if (errRet == MPKG_RETURN_ABORT)
					{
						abortMount=true;
					}
				}
				if (abortMount) mountedOk=true;
			}
			if (abortMount) return MPKGERROR_ABORTED;
		}
		else {
			mError(_("Installation error: file not found"));
			mError("Filename was: " + SYS_CACHE + *package->get_filename());
			return MPKGERROR_FILEOPERATIONS;
		}
	}

	// NEW (04.10.2007): Check if package is source, and build if needed. Also, import to database the output binary package and prepare to install it.
	if (package->get_type()==PKGTYPE_SOURCE)
	{
		say(_("Package %s is source-based. Building...\n"), package->get_name()->c_str());
		string binary_out;
		if (emerge_package(SYS_CACHE + *package->get_filename(), &binary_out)!=0) {
			mError("Failed to build. Aborting...");
			return -45;
		}
		say(_("Package was built. Filename: %s\n"), binary_out.c_str());
		// Now we have a new binary package with filename stored in variable binary_out. Import him into database and create a link to cache.
		
		if (!copyFile(binary_out, SYS_CACHE + getFilename(binary_out))) {
			mError("Error copying package, aborting...");
			return -45;
		}
		//printf("File copied to cache");
		say(_("Importing to database\n"));
		LocalPackage binpkg(SYS_CACHE + getFilename(binary_out));
		if (binpkg.injectFile()!=0) {
			mError("Error injecting binary package, cannot continue");
			return -45;
		}
		PACKAGE binary_package = binpkg.data;
		emerge_to_db(&binary_package);
		say(_("Processing to install binary\n"));
		return install_package(&binary_package);
	}

	// First of all: EXTRACT file list and scripts!!!
	LocalPackage lp(SYS_CACHE + *package->get_filename());
	bool no_purge=true;
	vector<FILES> old_config_files;
	mDebug("purge check");
	int purge_id=0;
	if (package->action()!=ST_REPAIR) purge_id=get_purge(package->get_name()); // returns package id if this previous package config files are not removed, or 0 if purged.
	mDebug("purge check complete");
	mDebug("purge_id="+IntToStr(purge_id));
	if (purge_id==0)
	{
		no_purge=false;
	}
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	currentStatus = statusHeader + _("extracting installation scripts");
	pData.increaseItemProgress(package->itemID);
	lp.fill_scripts(package);
	currentStatus = statusHeader + _("extracting file list");
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}
	if (package->get_files()->empty()) lp.fill_filelist(package);
	currentStatus = statusHeader + _("detecting configuration files");
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	lp.fill_configfiles(package);
	mDebug("fill ended");
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	if (fileConflictChecking==CHECKFILES_PREINSTALL) 
	{
		currentStatus = statusHeader + _("checking file conflicts");
		pData.increaseItemProgress(package->itemID);

	}
	mDebug("Checking file conflicts\n");
	if (!force_skip_conflictcheck)
	{
		if (fileConflictChecking == CHECKFILES_PREINSTALL && check_file_conflicts(package)!=0)
		{
			mDebug("Check failed (dupes present)");
			currentStatus = _("Error: Unresolved file conflict on package ") + *package->get_name();
			mError(_("Unresolved file conflict on package ") + *package->get_name() + _(", it will be skipped!"));
			return -5;
		}
		mDebug("Check conflicts ok");
	}
	else mDebug("Conflict check skipped");
	
	currentStatus = statusHeader + _("installing...");
	pData.increaseItemProgress(package->itemID);


// Filtering file list...
	vector<FILES> package_files;

	if (!no_purge) add_filelist_record(package->get_id(), package->get_files());
	string sys;
	mDebug("Preparing scripts");
	pData.increaseItemProgress(package->itemID);
	if (actionBus._abortActions)
	{
		sqlFlush();
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_INSTALL, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}


	if (!DO_NOT_RUN_SCRIPTS)
	{
		currentStatus = statusHeader + _("executing pre-install scripts");
		if (FileExists(package->get_scriptdir() + "preinst.sh"))
		{
			string preinst="cd " + SYS_ROOT + " ; sh "+package->get_scriptdir() + "preinst.sh";
			if (!simulate) system(preinst.c_str());
		}
	}

	// Extracting package
	currentStatus = statusHeader + _("extracting...");
	pData.increaseItemProgress(package->itemID);


	mDebug("calling extract");
	string create_root="mkdir -p "+sys_root+" 2>/dev/null";
	if (!simulate) system(create_root.c_str());
	sys="(cd "+sys_root+"; tar zxf "+sys_cache + *package->get_filename();
	//If previous version isn't purged, do not overwrite config files
	if (no_purge)
	{
		mDebug("retrieving old config files");
		get_filelist(purge_id, &old_config_files, true);
		mDebug("no_purge flag IS SET, config files count = "+IntToStr(package->get_config_files()->size()));
		for (unsigned int i=0; i<package->get_config_files()->size(); i++)
		{
			// Writing new config files, skipping old
			for (unsigned int k=0; k < old_config_files.size(); k++)
			{
				if (*package->get_config_files()->at(i).get_name()==*old_config_files[k].get_name())
				{
					mDebug("excluding file "+*package->get_config_files()->at(i).get_name());
					sys+=" --exclude '"+*package->get_config_files()->at(i).get_name()+"'"; 
					// FIXME: exclude works NOT as needed in some cases.
					// For example, if we want to exclude /install, the /bin/install will be excluded too
				}
			}
		}
		for (unsigned int i=0; i<package->get_files()->size(); i++)
		{
			for (unsigned int k=0; k <= old_config_files.size(); k++)
			{
				if (k==old_config_files.size()) 
				{
					package_files.push_back(package->get_files()->at(i));
					break;
				}
				if (*package->get_files()->at(i).get_name()==*old_config_files[k].get_name())
				{
					mDebug("Skipping file " + *package->get_files()->at(i).get_name());
					break;
				}
			}
		}
		mDebug("package_files size: "+IntToStr(package_files.size())+", package->get_files size: "+IntToStr(package->get_files()->size()));
		add_filelist_record(package->get_id(), &package_files);
	}
	package->get_files()->clear();
	package_files.clear();
	sys+=" > /dev/null)";
//#ifdef ACTUAL_EXTRACT
	if (!simulate)
	{
		if (system(sys.c_str()) == 0)
		{
			system("rm -rf " + SYS_ROOT+"/install"); // Cleanup. Be aware of placing anything important to this directory
			currentStatus = statusHeader + _("executing post-install scripts...");
		}
		else {
			currentStatus = _("Failed to extract!");
			return -10;
		}
	}
//#endif
	pData.increaseItemProgress(package->itemID);


	
	// Creating and running POST-INSTALL script
	if (!DO_NOT_RUN_SCRIPTS)
	{
		if (FileExists(package->get_scriptdir() + "doinst.sh"))
		{
			string postinst="cd " + SYS_ROOT + " ; sh "+package->get_scriptdir() + "doinst.sh";
			if (!simulate) system(postinst);
		}
	}

	set_installed(package->get_id(), ST_INSTALLED);
	set_configexist(package->get_id(), ST_CONFIGEXIST);
	set_action(package->get_id(), ST_NONE);
	if (purge_id!=0){
		set_configexist(purge_id, ST_CONFIGNOTEXIST); // Clear old purge status
		cleanFileList(purge_id);
	}
	sqlFlush();
	mDebug("*********************************************\n*        Package installed sussessfully     *\n*********************************************");
	exportPackage(SYS_ROOT+"/"+legacyPkgDir, package);

	pData.increaseItemProgress(package->itemID);
	return 0;
}	//End of install_package
void mpkgDatabase::exportPackage(string output_dir, PACKAGE *p)
{
	mstring data;
	data = "PACKAGE NAME:\t" + *p->get_name() +"-"+*p->get_version()+"-"+*p->get_arch()+"-"+*p->get_build() +\
		"\nCOMPRESSED PACKAGE SIZE:\t"+*p->get_compressed_size()+ \
		"\nUNCOMPRESSED PACKAGE SIZE:\t"+*p->get_installed_size()+\
		"\nPACKAGE LOCATION:\t/var/log/mount/"+*p->get_filename()+\
		"\nPACKAGE DESCRIPTION:\n" + *p->get_name() + ":  " + *p->get_short_description()+\
		"\nFILE LIST:\n";
	get_filelist(p->get_id(), p->get_files());
	for (unsigned int f=0; f<p->get_files()->size(); f++)
	{
		data+=*p->get_files()->at(f).get_name()+"\n";
	}
	data+="\n";
	WriteFile(output_dir+"/"+*p->get_name()+"-"+*p->get_version()+"-"+*p->get_arch()+"-"+*p->get_build(), data.s_str());
}	
void mpkgDatabase::unexportPackage(string output_dir, PACKAGE *p)
{
	string victim = output_dir+"/"+*p->get_name()+"-"+*p->get_version()+"-"+*p->get_arch()+"-"+*p->get_build();
	unlink(victim.c_str());
}

int mpkgDatabase::remove_package(PACKAGE* package)
{
	//printf("remove_package is in progress\n");
	get_filelist(package->get_id(), package->get_files());
	package->sync();
	pData.setItemProgressMaximum(package->itemID, package->get_files()->size()+8);
	pData.setItemCurrentAction(package->itemID, "removing");
	if (!dialogMode)
	{
		if (package->action()==ST_UPDATE) say(_("Updating package %s %s by %s...\n"),package->get_name()->c_str(), 
				package->get_fullversion().c_str(),
				package->updatingBy->get_fullversion().c_str());
		else say(_("Removing package %s %s...\n"),package->get_name()->c_str(), package->get_fullversion().c_str());
	}

	string statusHeader = "["+IntToStr((int)actionBus.progress())+"/"+IntToStr((int)actionBus.progressMaximum()) + "] " + _("Removing package ") + *package->get_name()+": ";
	currentStatus = statusHeader + _("initialization");
	
	if (package->action()==ST_REMOVE || package->action()==ST_PURGE || package->action()==ST_UPDATE)
	{
		// Checking if package is updating, if so, get the files of new package already
		if (package->action()==ST_UPDATE) {
			LocalPackage *lp = new LocalPackage(SYS_CACHE + *package->updatingBy->get_filename());
			lp->fill_filelist(package->updatingBy);
			delete lp;
		}


		// Running pre-remove scripts
		//printf("Processing\n");
		mDebug("REMOVE PACKAGE::Preparing scripts");
		if(!DO_NOT_RUN_SCRIPTS)
		{
			if (FileExists(package->get_scriptdir() + "preremove.sh"))
			{
				currentStatus = statusHeader + _("executing pre-remove scripts");
				string prerem="cd " + SYS_ROOT + " ; sh "+package->get_scriptdir() + "preremove.sh";
				if (!simulate) system(prerem.c_str());
			}
		}
		
		pData.increaseItemProgress(package->itemID);
		
		// removing package
		mDebug("calling remove");
		string sys_cache=SYS_CACHE;
		string sys_root=SYS_ROOT;
		string fname;
		mDebug("Package has "+IntToStr(package->get_files()->size())+" files");

		// Checking backups
		vector<FILES> backups;
		get_backup_records(package, &backups);
		//printf("backups size: %d\n", backups.size());



		// Purge is now implemented here; checking all
		currentStatus = statusHeader + _("building file list");
		vector<FILES> *remove_files = package->get_files(); // Note: no need to delete remove_files, because it will be deleted together with package object
		vector<FILES> *new_files = package->updatingBy->get_files();

		currentStatus = statusHeader + _("removing files...");
		bool removeThis;
		int unlink_ret;
		for (unsigned int i=0; i<remove_files->size(); i++)
		{
			fname=sys_root + *remove_files->at(i).get_name();
			for (unsigned int t=0; t<backups.size(); t++) {
				if (*remove_files->at(i).get_name()==*backups[t].get_name()) {
					fname = (string) SYS_BACKUP+"/"+*backups[t].get_backup_file();
					//printf("backup file: %s\n", backups[t].get_backup_file()->c_str());
					delete_conflict_record(backups[t].overwriter_id, backups[t].get_backup_file());
					break;
				}
			}
		

			// Checking for: 
			// - if file is a configuration file
			// - if file will be overwritten by new package
			//
			removeThis = false;
			if (package->action()!=ST_UPDATE) {
					if (package->action()==ST_PURGE || remove_files->at(i).get_type()==FTYPE_PLAIN) removeThis = true;
			}
			else {
				if (remove_files->at(i).get_type()==FTYPE_PLAIN) { // Don't touch config files at all
					removeThis=true;
					for (unsigned int t=0; t<new_files->size(); t++) {
						if (*new_files->at(t).get_name()==*remove_files->at(i).get_name()) {
							removeThis=false;
							break;
						}
					}
				}
			}
			// Actually removing files
			if (removeThis && fname[fname.length()-1]!='/')
			{
				pData.increaseItemProgress(package->itemID);
				if (!simulate) {
					if (verbose) say("[%d] %s %s: ", i, _("Removing file"), fname.c_str());
					unlink_ret = unlink(fname.c_str());
					if (verbose) {
						if (unlink_ret==0) say("OK\n");
						else say(_("FAILED\n"));
					}
				}
			}
		}

		currentStatus = statusHeader + _("removing empty directories...");
	
		// Run 2: clearing empty directories
		vector<string>empty_dirs;
		string edir;
		
		pData.increaseItemProgress(package->itemID);
		
		for (unsigned int i=0; i<remove_files->size(); i++)
		{
			fname=sys_root + *remove_files->at(i).get_name();
			for (unsigned int d=0; d<fname.length(); d++)
			{
				edir+=fname[d];
				if (fname[d]=='/')
				{
					empty_dirs.resize(empty_dirs.size()+1);
					empty_dirs[empty_dirs.size()-1]=edir;
				}
			}

			for (int x=empty_dirs.size()-1;x>=0; x--)
			{
				if (!simulate) {
					unlink_ret = rmdir(empty_dirs[x].c_str());
					if (verbose) {
						if (unlink_ret == 0) say("[%d] %s %s", i, _("Removing empty directory"), fname.c_str());
					}

				}
			}
			edir.clear();
			empty_dirs.clear();
		}
	
		// Creating and running POST-REMOVE script
		if (!DO_NOT_RUN_SCRIPTS)
		{
			if (FileExists(package->get_scriptdir() + "postremove.sh"))
			{
				currentStatus = statusHeader + _("executing post-removal scripts");
				string postrem="cd " + SYS_ROOT + " ; sh " + package->get_scriptdir() + "postremove.sh";
				if (!simulate) system(postrem.c_str());
			}
		}
	
		// Restoring backups. NOTE: if there is an updating package, manage backups anyway. TODO: minimize file operation by processing more data
		vector<FILES>restore;
		get_conflict_records(package->get_id(), &restore);
		if (!restore.empty())
		{
			string cmd;
			string tmpName;
			for (unsigned int i=0; i<restore.size(); i++)
			{
				if (restore[i].get_name()->find_last_of("/")!=std::string::npos)
				{
					cmd = "mkdir -p ";
					cmd += SYS_ROOT + restore[i].get_name()->substr(0, restore[i].get_name()->find_last_of("/"));
					if (!simulate) system(cmd.c_str());
				}
				cmd = "mv ";
			        cmd += SYS_BACKUP+*restore[i].get_backup_file() + " ";
				tmpName = restore[i].get_backup_file()->substr(SYS_BACKUP.length());
				tmpName = tmpName.substr(tmpName.find("/"));
			        cmd += SYS_ROOT + tmpName.substr(0,tmpName.find_last_of("/"))+"/";
				if (!simulate) system(cmd);
				delete_conflict_record(package->get_id(), restore[i].get_backup_file());
			}
		}

		pData.increaseItemProgress(package->itemID);
		set_installed(package->get_id(), ST_NOTINSTALLED);
		if (package->action()==ST_PURGE) set_configexist(package->get_id(), 0);
		set_action(package->get_id(), ST_NONE);
		currentStatus = statusHeader + _("cleaning file list");
		pData.increaseItemProgress(package->itemID);
		cleanFileList(package->get_id());
		pData.increaseItemProgress(package->itemID);
		sqlFlush();
		currentStatus = statusHeader + _("remove complete");
		mDebug("Package removed sussessfully");
		package->get_files()->clear();
		unexportPackage(SYS_ROOT+"/"+legacyPkgDir, package);
		return 0;
	}
	else
	{
		mError(_("Weird status of package, i'm afraid to remove this..."));
		return -1;
	}
}	// End of remove_package

int mpkgDatabase::delete_packages(PACKAGE_LIST *pkgList)
{
	if (pkgList->IsEmpty())
	{
		return 0;
	}
	SQLRecord sqlSearch;
	sqlSearch.setSearchMode(SEARCH_OR);
	for (int i=0; i<pkgList->size(); i++)
	{
		sqlSearch.addField("package_id", pkgList->get_package(i)->get_id());
	}
	db.sql_delete("packages", sqlSearch);
	sqlSearch.clear();
	for (int i=0; i<pkgList->size(); i++)
	{
		sqlSearch.addField("packages_package_id", pkgList->get_package(i)->get_id());
	}
	db.sql_delete("tags_links", sqlSearch);
	db.sql_delete("dependencies", sqlSearch);
#ifdef ENABLE_INTERNATIONAL
	db.sql_delete("descriptions", sqlSearch);
	db.sql_delete("changelogs", sqlSearch);
	db.sql_delete("ratings", sqlSearch);
#endif

	// Removing unused tags
	sqlSearch.clear();
	SQLTable available_tags;
	SQLRecord fields;

	db.get_sql_vtable(&available_tags, sqlSearch, "tags", fields);
	SQLTable used_tags;
	db.get_sql_vtable(&used_tags, sqlSearch, "tags_links", fields);
	vector<string> toDelete;
	bool used;

	// Index
	int fAvailTags_name = available_tags.getFieldIndex("tags_name");
	int fAvailTags_id = available_tags.getFieldIndex("tags_id");
	int fUsedTags_tag_id = used_tags.getFieldIndex("tags_tag_id");
	if (available_tags.size()>0)
	{
		for (int i=0; i<available_tags.size(); i++)
		{
			used=false;
			for (int u=0; u<used_tags.size(); u++)
			{
				if (*used_tags.getValue(u, fUsedTags_tag_id)==*available_tags.getValue(i, fAvailTags_id))
				{
					used=true;
				}
			}
			
			if (!used)
			{
				say(_("Deleting tag %s as unused\n"), available_tags.getValue(i, fAvailTags_name)->c_str());
				toDelete.push_back(*available_tags.getValue(i,"tags_id"));
			}
		}
		available_tags.clear();
		sqlSearch.clear();
		sqlSearch.setSearchMode(SEARCH_OR);
		if (toDelete.size()>0)
		{
			for (unsigned int i=0; i<toDelete.size(); i++)
			{
				sqlSearch.addField("tags_id", &toDelete[i]);
			}
			db.sql_delete("tags", sqlSearch);
		}
	}
	return 0;
}



int mpkgDatabase::cleanFileList(int package_id)
{
	SQLRecord sqlSearch;
	sqlSearch.addField("packages_package_id", package_id);
	bool cexist = get_configexist(package_id);
	if (cexist)
	{
		//printf("leaving configs intact\n");
		sqlSearch.addField("file_type", FTYPE_PLAIN);
	}
	int ret = db.sql_delete("files", sqlSearch);
	if (ret!=0) return ret;
	if (cexist)
	{
		//printf("checking if it has any configs\n");
		vector<FILES> flist;
		get_filelist(package_id, &flist);
		if (flist.size()==0) 
		{
			//printf("it has no configs...\n");
			set_configexist(package_id,0);
		}
	}
	return ret;
}

int mpkgDatabase::update_package_data(int package_id, PACKAGE *package)
{
	PACKAGE old_package;
	if (get_package(package_id, &old_package)!=0)
	{
		mDebug("mpkg.cpp: update_package_data(): get_package error: no package or error while querying database");
		return -1;
	}
	
	SQLRecord sqlUpdate;
	SQLRecord sqlSearch;
	sqlSearch.addField("package_id", package_id);

	mDebug("mpkg.cpp: update_package_data(): updating direct package data");
	// 1. Updating direct package data
	if (*package->get_md5()!=*old_package.get_md5())
	{
		mDebug("mpkg.cpp: update_package_data(): md5 mismatch, updating description fields");
		sqlUpdate.addField("package_description", package->get_description());
		sqlUpdate.addField("package_short_description", package->get_short_description());
		sqlUpdate.addField("package_compressed_size", package->get_compressed_size());
		sqlUpdate.addField("package_installed_size", package->get_installed_size());
		sqlUpdate.addField("package_changelog", package->get_changelog());
		sqlUpdate.addField("package_packager", package->get_packager());
		sqlUpdate.addField("package_packager_email", package->get_packager_email());
		sqlUpdate.addField("package_md5", package->get_md5());
	}

	// 2. Updating filename
	if (*package->get_filename()!=*old_package.get_filename())
	{
		mDebug("mpkg.cpp: update_package_data(): filename mismatch, updating");
		sqlUpdate.addField("package_filename", package->get_filename());
	}

	// 3. Updating status. Seems that somewhere here is an error causing double scan is required
	sqlUpdate.addField("package_available", package->available());


	// 4. Updating locations
	
	// Note about method used for updating locations:
	// If locations are not identical:
	// 	Step 1. Remove all existing package locations from "locations" table. Servers are untouched.
	// 	Step 2. Add new locations.
	// Note: after end of updating procedure for all packages, it will be good to do servers cleanup - delete all servers who has no locations.
	mDebug("mpkg.cpp: update_package_data(): checking locations");	
	if (!package->locationsEqualTo(&old_package))
	{
		mDebug("mpkg.cpp: update_package_data(): locations mismatch, cleanup");
		mDebug("mpkg.cpp: update_package_data(): old has "+IntToStr(old_package.get_locations()->size())+" locations, but new has "+\
				IntToStr(package->get_locations()->size())+" ones");
		SQLRecord loc_sqlDelete;
		loc_sqlDelete.addField("packages_package_id", package_id);
		mDebug("mpkg.cpp: update_package_data(): deleting old locations relating this package");
		int sql_del=db.sql_delete("locations", loc_sqlDelete);
		if (sql_del!=0)
		{
			mDebug("mpkg.cpp: update_package_data(): unable to delete old locations: SQL error "+IntToStr(sql_del));
			return -2;
		}
		if (add_locationlist_record(package_id, package->get_locations())!=0)
		{
			mDebug("mpkg.cpp: update_package_data(): unable to add new locations: error in add_locationlist_record()");
			return -3;
		}
		if (package->get_locations()->empty())
		{
			sqlUpdate.addField("package_available", ST_NOTAVAILABLE);
		}
	}

	// 5. Updating tags
	if (!package->tagsEqualTo(&old_package))
	{
		SQLRecord taglink_sqlDelete;
		taglink_sqlDelete.addField("packages_package_id", package_id);

		if (db.sql_delete("tags_links", taglink_sqlDelete)!=0) return -4;
		if (add_taglist_record(package_id, package->get_tags())!=0) return -5;
	}

	// 6. Updating dependencies
	if (!package->depsEqualTo(&old_package))
	{
		SQLRecord dep_sqlDelete;
		dep_sqlDelete.addField("packages_package_id", package_id);

		if(db.sql_delete("dependencies", dep_sqlDelete)!=0) return -6;
		if (add_dependencylist_record(package_id, package->get_dependencies())!=0) return -7;
	}

	// 7, 8 - update scripts and file list. It is skipped for now, because we don't need this here (at least, for now).
	if (!sqlUpdate.empty())
	{
		if (db.sql_update("packages", sqlUpdate, sqlSearch)!=0)
		{
			mDebug("mpkg.cpp: update_package_data(): db.sql_update failed");
			return -8;
		}
	}
	else mDebug("mpkg.cpp: update_package_data(): sqlUpdate empty, nothing to update");
	mDebug("mpkg.cpp: update_package_data(): successful end");
	return 0;
}




int mpkgDatabase::updateRepositoryData(PACKAGE_LIST *newPackages)
{
	// Одна из самых страшных функций в этой программе.
	// Попытаемся применить принципиально новый алгоритм.
	// 
	// Для этого введем некоторые тезисы:
	// 1. Пакет однозначно идентифицируется по его контрольной сумме.
	// 2. Пакет без контрольной суммы - не пакет а мусор, выкидываем такое нахрен.
	// 
	// Алгоритм:
	// 1. Стираем все записи о locations и servers в базе.
	// 2. Забираем из базы весь список пакетов. Этот список и будет рабочим.
	// 3. Для каждого пакета из нового списка ищем соответствие в старой базе.
	// 	3а. В случае если такое соответствие найдено, вписываем в список записи о locations. Остальное остается неизменным, ибо MD5 та же.
	// 	3б. В случае если соответствие не найдено, пакет добавляется в конец рабочего списка с флагом new = true
	// 4. Вызывается синхронизация рабочего списка и данных в базе (это уже отдельная тема).
	//
	// ////////////////////////////////////////////////
	mDebug("Retrieving current package list, clearing tables");	
	// Стираем locations и servers
	db.clear_table("locations");
	

	// Забираем текущий список пакетов
	PACKAGE_LIST *pkgList = new PACKAGE_LIST;
	SQLRecord sqlSearch;
	get_packagelist(&sqlSearch, pkgList);
	
	//say("Merging data\n");
	// Ищем соответствия
	int pkgNumber;
	for (int i=0; i<newPackages->size(); i++)
	{
		pkgNumber = pkgList->getPackageNumberByMD5(newPackages->get_package(i)->get_md5());
		
		if (pkgNumber!=-1)	// Если соответствие найдено...
		{
			pkgList->get_package(pkgNumber)->set_locations(newPackages->get_package(i)->get_locations());	// Записываем locations
		}
		else			// Если соответствие НЕ найдено...
		{
			newPackages->get_package(i)->newPackage=true;
			pkgList->add(newPackages->get_package(i));
		}
	}

	//say("Clean up...\n");
	// Вызываем синхронизацию данных.
	// Вообще говоря, ее можно было бы делать прямо здесь, но пусть таки будет универсальность.
	delete newPackages;//->clear();
	syncronize_data(pkgList);
	return 0;
}
int mpkgDatabase::syncronize_data(PACKAGE_LIST *pkgList)
{
	// Идея:
	// Добавить в базу пакеты, у которых флаг newPackage
	// Добавить locations к тем пакетам, которые такого флага не имеют
	// 
	// Алгоритм:
	// Бежим по списку пакетов.
	// 	Если пакет имеет влаг newPackage, то сразу добавляем его в базу функцией add_package_record()
	//	Если флага нету, то сразу добавляем ему locations функцией add_locationlist_record()
	// 
	for (int i=0; i<pkgList->size(); i++)
	{
		if (pkgList->get_package(i)->newPackage) add_package_record(pkgList->get_package(i));
		else add_locationlist_record(pkgList->get_package(i)->get_id(), pkgList->get_package(i)->get_locations());
	}
	delete pkgList;
	mDebug("Cleanup\n");

	// Дополнение от 10 мая 2007 года: сносим нафиг все недоступные пакеты, которые не установлены. Нечего им болтаться в базе.
	PACKAGE_LIST *allList = new PACKAGE_LIST;
	SQLRecord sqlSearch;
	mDebug("retrievin pkglist");
	get_packagelist(&sqlSearch, allList);
	mDebug("retrieved package list");
	PACKAGE_LIST deleteQueue;
	for (int i=0; i<allList->size(); i++)
	{
		if (!allList->get_package(i)->reachable(true))
		{
			deleteQueue.add(allList->get_package(i));
		}
	}
	mDebug("built delete queue");
	if (!deleteQueue.IsEmpty()) delete_packages(&deleteQueue);
	mDebug("removed wrong packages");
	delete allList;
	mDebug("deleted object, returning");
	return 0;

}

