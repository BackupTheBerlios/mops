/**
 *	MOPSLinux packaging system    
 *	CLI interface
 *	
 *	$Id: installpkg-ng2.cpp,v 1.84 2007/11/23 01:01:46 i27249 Exp $
 */
#include "libmpkg.h"
#include "converter.h"
#include "dialog.h"
#include "mpkgsys.h"
const char* program_name;
extern char* optarg;
extern int optind, opterr, optopt;
//string output_dir;
//static LoggerPtr rootLogger;
void show_package_info(mpkg *core, string name);
bool repair_damaged=false;
int setup_action(char* act);
int check_action(char* act);
int print_usage(FILE* stream=stdout, int exit_code=0);
int list(mpkg *core, vector<string> search, bool onlyQueue=false);
void ShowBanner();
int list_rep(mpkg *core);
void list_pkglist(PACKAGE_LIST *pkglist);
void searchByFile(mpkg *core, string filename, bool strict=false);
bool showOnlyAvailable=false;
bool showOnlyInstalled=false;
bool showFilelist=false;
void ShowBanner()
{
	char *copyright="\(c) 2006-2007 RPUNet (http://www.rpunet.ru)";
	say("MOPSLinux packaging system v.%s (build %s)\n%s\n--\n", mpkgVersion, mpkgBuild, copyright);
}
void cleanDebugFile()
{
	struct stat s;
	string file = log_directory + "/mpkg-debug.log";
	if (stat(file.c_str(), &s)==0)
	{
		if (s.st_size >= 4000) unlink(file.c_str());
	}
	file = log_directory + "/mpkg-errors.log";
	if (stat(file.c_str(), &s)==0)
	{
		if (s.st_size >= 4000) unlink(file.c_str());
	}

}

int verbose = 0;
int main (int argc, char **argv)
{
	cleanDebugFile();
	// Check database lock
	mpkg core;
	if (!core.init_ok)
	{
		mError(_("Error initializing CORE"));
		abort();
	}
	if ((string) argv[0] == "buildpkg")
	{
		if (argc==2) core.build_package((string) argv[1],false);
		if (argc==1) core.build_package((string) "", false);
		if (argc>2) {
			fprintf(stderr,_("Too many arguments\n"));
			fprintf(stderr,_("Usage: buildpkg [output_directory]\n"));
			return -1;
		}
	    	return 0;
	}
	if ((string) argv[0] == "buildsrcpkg")
	{
		if (argc==2) core.build_package((string) argv[1],true);
		if (argc==1) core.build_package((string) "", true);
		if (argc>2) {
			fprintf(stderr,_("Too many arguments\n"));
			fprintf(stderr,_("Usage: buildsrcpkg [output_directory]\n"));
			return -1;
		}
	    	return 0;
	}

	ShowBanner();
	setlocale(LC_ALL, "");
	bindtextdomain( "installpkg-ng", "/usr/share/locale");
	textdomain("installpkg-ng");



	/**
	 * remove everything?
	 */
	int do_purge = 0;

	/**
	 * selected action
	 * by default - ACT_NONE
	 */
	int action = ACT_NONE;

	uid_t uid, euid;

	uid = getuid();
	euid = geteuid();

		
	bool do_reset=true;
	int ich;
	const char* short_opt = "hvpdzfmkDrailgyq!cw";
	const struct option long_options[] =  {
		{ "help",		0, NULL,	'h'},
		{ "verbose", 		0, NULL,	'v'},
		{ "purge", 		0, NULL,	'p'},
		{ "force-dep", 		0, NULL,	'd'},
		{ "no-dep",		0, NULL,	'z'},
		{ "force-conflicts",	0, NULL,	'f'},
		{ "no-md5",		0, NULL,	'm'},
		{ "force-essential",	0, NULL,	'k'},
		//{ "simulate",		0, NULL,	's'},
		{ "download-only",	0, NULL,	'D'},
		{ "repair",		0, NULL,	'r'},
		{ "available",		0, NULL,	'a'},
		{ "installed",		0, NULL,	'i'},
		{ "filelist",		0, NULL,	'l'},
		{ "dialog",		0, NULL,	'g'},
		{ "noconfirm",		0, NULL,	'y'},
		{ "noreset",		0, NULL,	'q'},
		{ "nodepgen",		0, NULL,	'!'},
		{ "resume",		0, NULL,	'c'},
		{ NULL, 		0, NULL, 	0}
	};

	program_name = argv[0];

	if (!dialogMode) interactive_mode=true;
	do {
		ich = getopt_long(argc, argv, short_opt, long_options, NULL);
		

		switch (ich) {
			case 'c':
					enableDownloadResume=false;
					break;
			case 'q':
					do_reset=false;
					break;
			case 'h':
					print_usage(stdout, 0);
					return 0;

			case 'v':
					verbose = 1;
					break;

			case 'p':
					do_purge = 1;
					break;

			case 'd':
					force_dep = true;
					break;
			case 'f':
					force_skip_conflictcheck = true;
					break;

			case 'm':
					forceSkipLinkMD5Checks=true;
					break;

			case 'k':
					force_essential_remove=true;
					break;
			/*case 's':
					simulate=true;
					break;*/
			case 'i':
					showOnlyInstalled=true;
					break;
			case 'a':
					showOnlyAvailable=true;
					break;
			case 'D':
					download_only=true;
					break;

			case 'r':
					repair_damaged=true;
					break;

			case 'l':
					showFilelist=true;
					break;
			case 'g':
					dialogMode=true;
					break;
	
			case 'z':
					ignoreDeps=true;
					break;
			case 'y':
					interactive_mode=false;
					break;
			case '!':
					autogenDepsMode=ADMODE_OFF;
					break;
			case 'w':
					useBuildCache = false;
					break;
			case '?':
					return print_usage(stderr, 1);

			case -1:
					break;
					

			default:
					abort();
		}
	
	}  while ( ich != -1 );

	if ( optind < argc ) {
		if ( check_action( argv[optind++] ) == -1 )
		{
			return print_usage(stderr, 1);
		}
		
		action = setup_action( argv[optind-1] );
	}
	if (action==ACT_SHOW || \
			action == ACT_SEARCH || \
			action == ACT_SHOWQUEUE || \
			action == ACT_CONVERT || \
			action == ACT_CONVERT_DIR || \
			action == ACT_TAG || \
			action == ACT_LIST || \
			action == ACT_INDEX || \
			action == ACT_GENDEPS || \
			action == ACT_NONE) 
		require_root=false;

	if (require_root && uid != 0 ) {
		mError(_("You must login as root to run this program"));
		exit(1);
	}
	if (require_root && isDatabaseLocked())
	{
		mError(_("Error: database is locked. Please close all other programs that use this"));
		exit(1);
	}

	if (action == ACT_INSTALL ||
			action == ACT_PACKAGEMENU ||
			action == ACT_PURGE ||
			action == ACT_REMOVE ||
			action == ACT_INSTALLGROUP ||
			action == ACT_REMOVEGROUP)
	{
		if (do_reset) core.clean_queue();
	}

	if ( action == ACT_NONE )
			return print_usage(stderr, 1);


	if ( action == ACT_SHOW)
	{
		if (argc<=optind) return print_usage(stderr,1);
		show_package_info(&core, argv[optind]);
	}
	vector<string> pname;
	if (action == ACT_COMMIT)
	{
		if (argc!=optind) return print_usage(stderr,1);

		core.commit();
		unlockDatabase();
		return 0;
	}

	if (action == ACT_SHOWQUEUE)
	{
		if (argc!=optind) return print_usage(stderr,1);

		vector<string> list_empty;
		list(&core, list_empty,true);
		delete_tmp_files();
		return 0;
	}

	if (action == ACT_RESETQUEUE)
	{
		if (argc!=optind) return print_usage(stderr,1);

		lockDatabase();
		core.clean_queue();
		delete_tmp_files();
		unlockDatabase();
		return 0;
	}

	if (action == ACT_PACKAGEMENU)
	{
		if (argc!=optind) return print_usage(stderr,1);

		lockDatabase();
		Dialog dialogItem;
		vector<TagPair> pkgList;
		SQLRecord sqlSearch;
		PACKAGE_LIST packageList;
		core.get_packagelist(&sqlSearch, &packageList);
		for (int i=0; i<packageList.size(); i++)
		{
			pkgList.push_back(TagPair(*packageList.get_package(i)->get_name()+"="+*packageList.get_package(i)->get_version()+"="+*packageList.get_package(i)->get_build(),\
					       *packageList.get_package(i)->get_short_description(),\
					       packageList.get_package(i)->installed()));
		}
		vector<TagPair> oldState = pkgList;
		if (dialogItem.execCheckList("Отредактируйте состояние пакетов", 0,0,0, &pkgList))
		{
			vector<string> removeList;
			vector<string> installList_names, installList_versions, installList_builds, fname, p_version, p_build;
			string name, version, build;
			for (unsigned int i = 0; i < oldState.size(); i++)
			{
				name = pkgList[i].tag;
				version.clear();
				build.clear();
				if (name.find_first_of("=")!=std::string::npos) {
					version = name.substr(name.find_first_of("="));
					if (version.length()>1) version=version.substr(1);
					name = name.substr(0, name.find_first_of("="));
					if (version.find_first_of("=")!=std::string::npos) {
						build = version.substr(version.find_first_of("="));
						if (build.length()>1) build=build.substr(1);
					}
				}
				fname.push_back(name);
				p_version.push_back(name);
				p_build.push_back(build);
			}

			for (unsigned int i=0; i<oldState.size(); i++)
			{
				if (oldState[i].checkState!=pkgList[i].checkState)
				{
					if (pkgList[i].checkState) {
						installList_names.push_back(fname[i]);
						installList_versions.push_back(p_version[i]);
						installList_builds.push_back(p_build[i]);
					}
					else removeList.push_back(fname[i]);
				}
			}
			/*printf("Install list:\n");
			for (unsigned int i=0; i<installList.size(); i++)
			{
				printf("INSTALL: %s\n", installList[i].c_str());
			}
			for (unsigned int i=0; i<removeList.size(); i++)
			{
				printf("REMOVE: %s\n", removeList[i].c_str());
			}*/
			
			core.uninstall(removeList);
			core.install(installList_names, &installList_versions, &installList_builds);
			core.commit();
		}
		else say(_("Cancelled\n"));
		unlockDatabase();
		return 0;
	}

	if (action == ACT_INSTALLFROMLIST)
	{
		if (argc<=optind) return print_usage(stderr,1);

		// Read the file
		FILE *install_list = fopen(argv[optind], "r");
		if (!install_list)
		{
			perror(_("cannot open installation list"));
			exit(-1);
		}
		char *membuff=(char *) malloc(2000);
		vector<string> installQuery;
		memset(membuff, 0, 2000);
		string tmp;
		while (fscanf(install_list, "%s", membuff)!=EOF)
		{
			tmp = (string) membuff;
			installQuery.push_back(tmp.substr(1, tmp.size()-2));
		}
		free(membuff);
		lockDatabase();
		fclose(install_list);
		for (unsigned int i=0; i<installQuery.size(); i++)
		{
			mDebug("Installing from list " + installQuery[i]);
		}
		core.install(installQuery);
		core.commit();
		delete_tmp_files();
		unlockDatabase();
		return 0;
	}				
	
	/*if (action == ACT_UPGRADE ) {
		if (argc<=optind) return print_usage(stderr,1);

		lockDatabase();
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		core.install(pname);
		delete_tmp_files();
		unlockDatabase();
		return 0;
	}*/

	if (action == ACT_UPDATEALL)
	{
		lockDatabase();
		PACKAGE_LIST pkgList;
		vector<string> updates;
		SQLRecord sqlSearch;
		core.get_packagelist(&sqlSearch, &pkgList);
		bool thisIsUpdate, already;
		// Searching which has updates
		for (int i=0; i<pkgList.size(); i++)
		{
			for (int t=0; t<pkgList.size(); t++)
			{
				thisIsUpdate=false;
				if (i!=t && \
						pkgList.get_package(i)->installed() && \
						pkgList.get_package(t)->available() && \
						*pkgList.get_package(i)->get_name() == *pkgList.get_package(t)->get_name())
				{
					if (strverscmp(pkgList.get_package(i)->get_version()->c_str(),pkgList.get_package(t)->get_version()->c_str())<0)
					{
						// Update (version)
						thisIsUpdate=true;
					}
					else {
						if (strverscmp(pkgList.get_package(i)->get_version()->c_str(),pkgList.get_package(t)->get_version()->c_str())==0 && \
								strverscmp(pkgList.get_package(i)->get_build()->c_str(),pkgList.get_package(t)->get_build()->c_str())<0)
						{
							// Update (build)
							thisIsUpdate=true;
						}
					}
				}
				if (thisIsUpdate) {
					already=false;
					for (unsigned int k=0; k<updates.size(); k++)
					{
						if (updates[k]==*pkgList.get_package(t)->get_name())
						{
							already = true;
							break;
						}
					}
					if (!already) updates.push_back(*pkgList.get_package(t)->get_name());
				}
			}
		}
		if (updates.size()!=0)
		{
			core.install(updates);
			core.commit();
			core.clean_queue();
			delete_tmp_files();
			unlockDatabase();
		}
		else say(_("No updates available\n"));
		return 0;
	}


	if (action == ACT_BUILD)
	{
		if (argc<=optind) return print_usage(stderr,1);
		string s_pkg;
		string march, mtune, olevel;
		for (int i=optind; i < argc; i++)
		{
			s_pkg = argv[i];
			if (s_pkg.find("march=")==0) {
				march=s_pkg.substr(strlen("march="));
				continue;
			}
			if (s_pkg.find("mtune=")==0) {
				mtune=s_pkg.substr(strlen("mtune="));
				continue;
			}

			if (s_pkg.find("olevel=")==0) {
				olevel=s_pkg.substr(strlen("olevel="));
				continue;
			}


			
			if (emerge_package(s_pkg, &s_pkg, march, mtune, olevel)!=0) {
				mError("One of the packages failed to build, stopping");
				delete_tmp_files();
				return 0;
			}
		}
		delete_tmp_files();
		return 0;
	}

	if (action == ACT_INSTALL || action == ACT_UPGRADE || action == ACT_REINSTALL)
	{

		vector<string> r_name, fname, p_version, p_build;
		string name, version, build;
		if (argc<=optind) return print_usage(stderr,1);
		lockDatabase();
		for (int i = optind; i < argc; i++) {
			r_name.push_back((string) argv[i]);
		}

		// Check for wildcards
		bool hasWilds=false;
		for (unsigned int i=0; i<r_name.size(); i++)
		{
			if (r_name[i].find("*")!=std::string::npos)
			{
				hasWilds=true;
				break;
			}
		}
		if (hasWilds)
		{
			SQLTable pl;
			SQLRecord sqlr, sqlfields;
			sqlr.setEqMode(EQ_CUSTOMLIKE);
			vector<string> srch=r_name;
			sqlfields.addField("package_name");
			for (unsigned int i=0; i<srch.size(); i++)
			{
				while(srch[i].find_first_of("*")!=std::string::npos)
				{
					srch[i][srch[i].find_first_of("*")]='%';
				}
				sqlr.addField("package_name", &srch[i]);
			}
			core.db->get_sql_vtable(&pl, sqlfields, (string) "packages", sqlr);
			r_name.clear();
			for (int i=0; i<pl.size(); i++)
			{
				r_name.push_back(*pl.getValue(i,"package_name"));
			}
		}

		for (int i = 0; i < r_name.size(); i++)
		{
			name = r_name[i];
			version.clear();
			build.clear();
			if (name.find_first_of("=")!=std::string::npos) {
				version = name.substr(name.find_first_of("="));
				if (version.length()>1) version=version.substr(1);
				name = name.substr(0, name.find_first_of("="));
				if (version.find_first_of("=")!=std::string::npos) {
					build = version.substr(version.find_first_of("="));
					if (build.length()>1) build=build.substr(1);
				}
			}
			//printf("adding [%s]-[%s]-[%s]\n", name.c_str(), version.c_str(), build.c_str());
			fname.push_back(name);
			p_version.push_back(version);
			p_build.push_back(build);
		}
		
		if (action != ACT_REINSTALL) core.install(fname, &p_version, &p_build);
		else {
			for (unsigned int i=0; i<fname.size(); i++)
			{
				//printf("TODO: repair package %s\n", fname[i].c_str());
				core.repair(fname[i]);
			}
		}
		core.commit();
		core.clean_queue();
		delete_tmp_files();

		if (usedCdromMount) system("umount " + CDROM_MOUNTPOINT);
		unlockDatabase();
		return 0;
	}

	if (action == ACT_CONFIG)
	{
		if (argc!=optind) return print_usage(stderr,1);
		say(_("Current configuration:\n"));
		say(_("System root: %s\n"), SYS_ROOT.c_str());
		say(_("Package cache: %s\n"), SYS_CACHE.c_str());
		say(_("Run scripts: %d\n"), !DO_NOT_RUN_SCRIPTS);
		say(_("Database: %s\n"), DB_FILENAME.c_str());
		say(_("CD-ROM device: %s\n"), CDROM_DEVICE.c_str());
		say(_("CD-ROM mountpoint: %s\n"), CDROM_MOUNTPOINT.c_str());
		say(_("Scripts directory: %s\n"), SCRIPTS_DIR.c_str());
		list_rep(&core);
		return 0;
	}
	if (action == ACT_EXPORT)
	{
		string dest_dir=SYS_ROOT+"/"+legacyPkgDir;
		if (argc>optind) dest_dir=argv[optind];

		core.exportBase(dest_dir);
		return 0;
	}
	if (action == ACT_LISTGROUPS)
	{
		vector<string> availableTags;
		core.get_available_tags(&availableTags);
		for (unsigned int i=0; i<availableTags.size(); i++)
		{
			say("%s\n", availableTags[i].c_str());
		}
		return 0;
	}
	if (action == ACT_TEST)
	{
		printf("march=%s\n", mConfig.getValue("march").c_str());
		return 0;
#ifdef RELEASE
		return print_usage(stderr,1);
#else
		return 0;
#endif
	}
	if (action == ACT_LISTDEPENDANTS)
	{
		if (argc<=optind) return print_usage(stderr, 1);
		say(_("Searching for packages which depends on %s\n"),argv[optind]);
		PACKAGE_LIST tList, dependant;
		PACKAGE *pkg = new PACKAGE;
		SQLRecord sqlSearch;
		string search = argv[optind];
		core.get_packagelist(&sqlSearch, &tList);
	
		for (int i=0; i<tList.size(); i++)
		{
			if (*tList.get_package(i)->get_name()==search)
			{
				if (!showOnlyInstalled || tList.get_package(i)->installed ())
				{
					*pkg = *tList.get_package(i);
				}
			}
		}
		dependant = core.DepTracker->get_dependant_packages(pkg);
		if (dependant.size()==0)
		{
			if (showOnlyInstalled) say(_("No installed packages depends on %s\n"), argv[optind]);
			else say(_("No package depends on %s\n"), argv[optind]);
			delete pkg;
			return 0;
		}
		else say(_("Next packages depends on %s: \n"),argv[optind]);
		for (int i=0; i<dependant.size(); i++)
		{
			say("%s\n", dependant.get_package(i)->get_name()->c_str());
		}
		delete pkg;
		return 0;
	}

	if (action == ACT_LISTGROUP)
	{
		if (argc<=optind) return print_usage(stderr,1);
		string group=argv[optind];
		PACKAGE_LIST pkgList1;
		PACKAGE_LIST pkgList2;
		SQLRecord sqlSearch;
		core.get_packagelist(&sqlSearch, &pkgList1);
		for (int i=0; i<pkgList1.size(); i++)
		{
			if (pkgList1.get_package(i)->isTaggedBy(group)) pkgList2.add(pkgList1.get_package(i));
		}
		list_pkglist(&pkgList2);
		return 0;
	}

	if (action == ACT_INSTALLGROUP)
	{
		if (argc<=optind) return print_usage(stderr,1);
		lockDatabase();
		string group=argv[optind];
		PACKAGE_LIST pkgList1;
		vector<string> queue;
		SQLRecord sqlSearch;
		core.get_packagelist(&sqlSearch, &pkgList1);
		for (int i=0; i<pkgList1.size(); i++)
		{
			if (pkgList1.get_package(i)->isTaggedBy(group) && !pkgList1.get_package(i)->installed()) queue.push_back(*pkgList1.get_package(i)->get_name());
		}
		core.install(queue);
		core.commit();
		core.clean_queue();
		delete_tmp_files();

		unlockDatabase();
		

		return 0;
	}

	if (action == ACT_REMOVEGROUP)
	{
		if (argc<=optind) return print_usage(stderr,1);
		lockDatabase();
		string group=argv[optind];
		PACKAGE_LIST pkgList1;
		vector<string> queue;
		SQLRecord sqlSearch;
		core.get_packagelist(&sqlSearch, &pkgList1);
		for (int i=0; i<pkgList1.size(); i++)
		{
			if (pkgList1.get_package(i)->isTaggedBy(group) && pkgList1.get_package(i)->installed()) queue.push_back(*pkgList1.get_package(i)->get_name());
		}
		core.uninstall(queue);
		core.commit();
		core.clean_queue();
		delete_tmp_files();
		unlockDatabase();
		return 0;

	}

	if (action == ACT_FILESEARCH)
	{
		if (argc<=optind) return print_usage(stderr,1);
		searchByFile(&core, argv[optind]);
		return 0;
	}


	if (action == ACT_WHICH)
	{
		if (argc<=optind) return print_usage(stderr,1);
		searchByFile(&core, argv[optind],true);
		return 0;
	}
	


	if (action == ACT_GENDEPS)
	{
		if (argc<=optind) return print_usage(stderr,1);
		generateDeps(argv[optind]);
		return 0;
	}

	if (action == ACT_CHECKDAMAGE)
	{
		lockDatabase();

		PACKAGE_LIST repairList;
		PACKAGE_LIST checkList;
		SQLRecord sqlSearch;
		sqlSearch.addField("package_installed",ST_INSTALLED);
		core.get_packagelist(&sqlSearch, &checkList);

		string pkgname;
		//printf("optind = %d, argc = %d\n", optind, argc);
		if (optind>=argc)
		{
			//printf("mode 1\n");
			// Check entire system
			for (int i=0; i<checkList.size(); i++)
			{
				say("[%d/%d] ",i+1,checkList.size());
				if (core.checkPackageIntegrity(checkList.get_package(i))) say("%s: %sOK%s\n",\
					       	checkList.get_package(i)->get_name()->c_str(), CL_GREEN, CL_WHITE);
				else 
				{
					say(_("%s: %sDAMAGED%s\n"), checkList.get_package(i)->get_name()->c_str(), CL_RED, CL_WHITE);
					if (repair_damaged) 
					{
						//printf("Adding to repair queue\n");
						repairList.add(checkList.get_package(i));
					}
				}
			}
		}
		else
		{
			int pkgIndex=-1;
			for (int i = optind; i<argc; i++)
			{
				pkgname = (string) argv[i];

				for (int t = 0; t<checkList.size(); t++)
				{
					if (checkList.get_package(t)->get_name()->find(pkgname)==0) {
						pkgIndex=t;
						break;
					}
				}
				
				if (core.checkPackageIntegrity(pkgname)) say("%s: %sOK%s\n", argv[i], CL_GREEN, CL_WHITE);
				else 
				{
					say(_("%s: %sDAMAGED%s\n"), argv[i], CL_RED, CL_WHITE);
					if (repair_damaged) 
					{
						//printf("Adding\n");
						repairList.add(checkList.get_package(pkgIndex));
					}

				}
			}
		}
		for (int i=0; i<repairList.size(); i++)
		{
			//printf("repairing %d\n", i);
			core.repair(repairList.get_package(i));
		}
		if (repairList.size()>0)
		{
			say(_("\n\n----------Repairing damaged packages----------\n"));
			core.commit();
		}

		unlockDatabase();
		return 0;
	}
	
	if (action == ACT_REMOVE)
	{
		if (argc<=optind) return print_usage(stderr,1);
		lockDatabase();
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		if (do_purge==0) core.uninstall(pname);
		else core.uninstall(pname);
		core.commit();
		core.clean_queue();
		delete_tmp_files();
		unlockDatabase();
		return 0;
	}

	if ( action == ACT_SEARCH ) {
		if (argc<=optind) return print_usage(stderr,1);

		vector<string> list_search;
		for (int i = optind; i < argc; i++)
		{
			list_search.push_back((string) argv[i]);
		}
		list(&core, list_search);
		delete_tmp_files();
		return 0;
	}
	
	if ( action == ACT_CONVERT  ) {
		for (int i = optind; i < argc; i++)
		{
			mError("Feature disabled");
		}
		delete_tmp_files();
		return 0;
	
	}

	if ( action == ACT_TAG )
	{
		if (argc > optind+1)
		{
			say(_("tagging %s as %s...\n"), argv[optind+1], argv[optind]);
			tag_package(argv[optind+1], argv[optind]);
		}
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_CONVERT_DIR ) {
		if (optind < argc )
		{
			core.convert_directory((string) argv[optind]);
		}
		else 
		{
			mError(_("Please define output directory"));
		}
		delete_tmp_files();

		return 0;
	}

	if ( action == ACT_LIST ) {
		if (argc!=optind) return print_usage(stderr,1);

		vector<string> list_empty;
		list(&core, list_empty);
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_UPDATE ) {
		if (argc!=optind) return print_usage(stderr,1);

		lockDatabase();
		core.update_repository_data();
		delete_tmp_files();

		if (usedCdromMount) system("umount " + CDROM_MOUNTPOINT);
		unlockDatabase();
		return 0;
		
	}

	if ( action == ACT_CLEAN ) {
		if (argc!=optind) return print_usage(stderr,1);
		core.clean_cache();	
		return 0;
	
	}

	if ( action == ACT_INDEX ) {
		if (argc!=optind) return print_usage(stderr,1);

		core.rep.build_index("","");
		delete_tmp_files();
	
		return 0;
	}

	if ( action == ACT_PURGE ) {
		if (argc<=optind) return print_usage(stderr,1);

		lockDatabase();
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		core.purge(pname);
		core.commit();
		core.clean_queue();
		delete_tmp_files();
		unlockDatabase();
		return 0;
	}

	if ( action == ACT_LIST_REP ) {
		if (argc!=optind) return print_usage(stderr,1);

		list_rep(&core);
		delete_tmp_files();
		return 0;
	}
	if ( action == ACT_ADD_REPOSITORY ) {
		if (argc<=optind) return print_usage(stderr,1);
		for (int i=optind; i < argc; i++) {
			core.add_repository((string) argv[i]);
		}
		list_rep(&core);
		return 0;
	}
	if (action == ACT_REMOVE_REPOSITORY ) {
		if (argc<optind) return print_usage(stderr,1);
		int shift = 0;
		for (int i=optind; i<argc; i++) {
			core.remove_repository(atoi(argv[i])-shift);
			shift++;
		}
		list_rep(&core);
		return 0;
	}

	if (action == ACT_ENABLE_REPOSITORY) {
		if (argc<optind) return print_usage(stderr,1);
		vector<int> en;
		for (int i=optind; i<argc; i++) {
			en.push_back(atoi(argv[i]));
		}
		core.enable_repository(en);
		list_rep(&core);
		return 0;
	}
	if (action == ACT_DISABLE_REPOSITORY) {
		if (argc<optind) return print_usage(stderr,1);
		vector<int> en;
		for (int i=optind; i<argc; i++) {
			en.push_back(atoi(argv[i]));
		}
		core.disable_repository(en);
		list_rep(&core);
		return 0;
	}


	// ACTION SUMMARY - NEED TO FIX!
	printf("Doing action summary\n");
	if (!core.DepTracker->get_install_list()->IsEmpty())
	{
		say(_("Next packages will be installed:\n"));
		for (int i=0;i<core.DepTracker->get_install_list()->size();i++)
		{
			say("%s\n", core.DepTracker->get_install_list()->get_package(i)->get_name()->c_str());
		}
	}

	if (!core.DepTracker->get_remove_list()->IsEmpty())
	{
		say(_("\nNext packages will be removed:\n"));
		for (int i=0;i<core.DepTracker->get_remove_list()->size();i++)
		{
			say("%s\n", core.DepTracker->get_remove_list()->get_package(i)->get_name()->c_str());
		}
	}

	if (!core.DepTracker->get_failure_list()->IsEmpty())
	{

		say(_("Next packages is failed to install:\n"));
		for (int i=0;i<core.DepTracker->get_failure_list()->size();i++)
		{
			say("%s: ", core.DepTracker->get_failure_list()->get_package(i)->get_name()->c_str());
		}
	}
	say("\n");

	unlockDatabase();
	return 0;
}


int print_usage(FILE* stream, int exit_code)
{
	fprintf(stream, _("\nUsage: %s [options] action package [package ...]\n"), program_name);
	fprintf(stream,_("Options:\n"));
	fprintf(stream,_("\t-h    --help              show this help\n"));
	fprintf(stream,_("\t-v    --verbose           be verbose\n"));
	fprintf(stream,_("\t-g    --dialog            use dialog mode UI\n"));
	fprintf(stream,_("\t-d    --force-dep         interpret dependency errors as warnings\n"));
	fprintf(stream,_("\t-z    --no-dep            totally ignore dependencies existance\n"));
	fprintf(stream,_("\t-f    --force-conflicts   do not perform file conflict checking\n"));
	fprintf(stream,_("\t-m    --no-md5            do not check package integrity on install\n"));
	fprintf(stream,_("\t-k    --force-essential   allow removing essential packages\n"));
	//fprintf(stream,_("\t-s    --simulate          just simulate all actions\n"));
	//fprintf(stream,_("\t-D    --download-only     just download packages, do not install\n"));
	fprintf(stream,_("\t-r    --repair            repair damaged packages (use with \"check\" keyword)\n"));
	fprintf(stream,_("\t-i    --installed         show only installed packages (use with \"list\" keyword)\n"));
	fprintf(stream,_("\t-a    --available         show only available packages (use with \"list\" keyword)\n"));
	fprintf(stream,_("\t-l    --filelist          show file list for package (with \"show\" keyword)\n"));
	fprintf(stream,_("\t-y    --noconfirm         don't ask confirmation\n"));
	fprintf(stream,_("\t-q    --noreset           don't reset queue at start\n"));
	fprintf(stream,_("\t-!    --nodepgen          don't generate dependencies on package building\n"));
	fprintf(stream,_("\t-w    --no_buildcache     don't use source cache when building packages\n")); 
	fprintf(stream,_("\t-c    --no_resume         disable download resuming\n"));

	
	fprintf(stream,_("\nActions:\n"));
	fprintf(stream,_("\tbuild                     build source package(s)\n"));
	fprintf(stream,_("\tinstall                   install package(s)\n"));
	fprintf(stream,_("\tupgrade                   upgrade selected package\n"));
	fprintf(stream,_("\tupgradeall                upgrade all installed packages\n"));
	fprintf(stream,_("\treinstall                 reinstall package(s)\n"));
	fprintf(stream,_("\tremove                    remove selected package(s)\n"));
	fprintf(stream,_("\tpurge                     purge selected package(s)\n"));
	fprintf(stream,_("\tinstallgroup              install all the packages from group\n"));
	fprintf(stream,_("\tremovegroup               remove all the packages from group\n"));

	fprintf(stream,_("\tshow                      show info about package\n"));
	fprintf(stream,_("\tupdate                    update packages info\n"));
	fprintf(stream,_("\tlist                      show the list of all packages in database\n"));
	fprintf(stream,_("\tlistgroup                 show the list of packages belonged to group\n"));
	fprintf(stream,_("\tlistgroups                show the list of all existing groups\n"));
	fprintf(stream,_("\twhodepend                 show what packages depends on this one\n"));
	
	fprintf(stream,_("\tfilesearch                look for owner of the file in installed packages (LIKE mode).\n"));
	fprintf(stream,_("\twhich                     look for owner of the file in installed packages (EQUAL mode).\n"));
	fprintf(stream,_("\tlist_rep                  list enabled repositories\n"));
	fprintf(stream,_("\tadd_rep                   add repository\n"));
	fprintf(stream,_("\tdelete_rep                delete Nth repository\n"));
	fprintf(stream,_("\tenable_rep                enable Nth repository\n"));
	fprintf(stream,_("\tdisable_rep               disable Nth repository\n"));
	fprintf(stream,_("\tinstallfromlist           install using file with list of items\n"));
	fprintf(stream,_("\treset                     reset queue\n"));
	fprintf(stream,_("\tshow_queue                show queue\n"));
	fprintf(stream,_("\tcommit                    commit queued actions\n"));
	fprintf(stream,_("\tsearch                    search package by name\n"));
	fprintf(stream,_("\tclean                     remove all packages from cache\n"));
	fprintf(stream,_("\tcheck                     checks installed package(s) for damaged files. Use -r flag to to repair\n"));

	fprintf(stream,_("\nInteractive options:\n"));
	fprintf(stream,_("\tmenu                      shows the package selection menu\n"));
//	fprintf(stream,_("\tedit_rep                  edit repository list with menu\n"));
	
	fprintf(stream,_("\nRepository maintaining functions:\n"));
	fprintf(stream,_("\tindex                     create a repository index file \"packages.xml.gz\"\n"));
	//fprintf(stream,_("\tconvert <package>         convert specified packages from Slackware to MPKG format\n"));
	fprintf(stream,_("\tconvert_dir <outp_dir>    convert whole directory (including sub-dirs) to MPKG format\n"));
	fprintf(stream,_("\texport [dir]              export database in slackware format to dir (by default, /var/log/packages/)\n"));
	fprintf(stream,_("\tgendeps <package>         generate dependencies and import it into package\n"));
#ifndef RELEASE
	fprintf(stream,_("\nDebug options:\n"));
	fprintf(stream,_("\ttest                      Executes unit test\n"));
#endif
	fprintf(stream,_("\nExtra options for command \"build\" (should be specified _before_ package name):\n"));
	fprintf(stream,_("\tmarch=                    CPU architecture\n"));
	fprintf(stream,_("\tmtune=                    CPU tuning\n"));
	fprintf(stream,_("\tolevel=                   Optimization level\n"));
	fprintf(stream,_("Example: mpkg build march=i686 mtune=prescott olevel=O3\n"));
	fprintf(stream, "\n");


	return exit_code;
}

int list_rep(mpkg *core)
{
	say(_("Repository list:\n"));
	vector <string> rlist=core->get_repositorylist();
	vector<string> dlist = core->get_disabled_repositorylist();
	for (unsigned int i=0; i<rlist.size(); i++)
	{
		say("* [%d] %s\n", i+1, rlist[i].c_str());
	}
	for (unsigned int i=0; i<dlist.size(); i++)
	{
		say("  [%d] %s (%s)\n", i+rlist.size()+1, dlist[i].c_str(), _("DISABLED"));
	}
	
	return 0;
}

void show_package_info(mpkg *core, string name)
{
	PACKAGE_LIST pkgList;
	SQLRecord sqlSearch;
	if (!name.empty())
	{
		sqlSearch.addField("package_name", &name);
	}
	core->get_packagelist(&sqlSearch, &pkgList);
	if (pkgList.IsEmpty())
	{
		say(_("No such package\n"));
	}
	for (int i=0; i<pkgList.size(); i++)
	{
		say(_("%sID:%s                 %d\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_id());
		say(_("%sName:%s               %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_name()->c_str());
		say(_("%sVersion:%s            %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_version()->c_str());
		say(_("%sArch:%s               %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_arch()->c_str());
		say(_("%sBuild:%s              %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_build()->c_str());
		say(_("%sPackage size:%s       %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_compressed_size()->c_str());
		say(_("%sInstalled size:%s     %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_installed_size()->c_str());
		say(_("%sMaintainer:%s         %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_packager()->c_str());
		say(_("%sMaintainer e-mail:%s  %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_packager_email()->c_str());
		say(_("%sStatus:%s             %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_vstatus(true).c_str());

		say(_("%sMD5:%s                %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_md5()->c_str());
		say(_("%sFilename:%s           %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_filename()->c_str());
		say(_("%sShort description:%s  %s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_short_description()->c_str());
		
		if (pkgList.get_package(i)->available()) {
			say(_("%sLocations:%s\n"), CL_GREEN, CL_WHITE);
			for (unsigned int t=0; t<pkgList.get_package(i)->get_locations()->size(); t++)
			{
				say("\t\t    %s\n", pkgList.get_package(i)->get_locations()->at(t).get_full_url().c_str());
			}
		}
		else say(_("%sLocations:%s          %s\n"), CL_GREEN, CL_WHITE, _("none"));
		
		say(_("%sTags:%s               \n"), CL_GREEN, CL_WHITE);
		if (pkgList.get_package(i)->get_tags()->empty())
		{
			say(_("none\n"));
		}
		else
		{
			for (unsigned int t=0; t<pkgList.get_package(i)->get_tags()->size(); t++)
			{
				say ("\t\t    %s\n", pkgList.get_package(i)->get_tags()->at(t).c_str());
			}
		}
		if (!pkgList.get_package(i)->get_dependencies()->empty())
		{
			say(_("%sDepends on:%s\n"), CL_GREEN, CL_WHITE);
			for (unsigned int t=0; t<pkgList.get_package(i)->get_dependencies()->size(); t++)
			{
				say("%s", pkgList.get_package(i)->get_dependencies()->at(t).getDepInfo().c_str());
				if (t<pkgList.get_package(i)->get_dependencies()->size()-1) say(", ");
			}
			say("\n");
		}

		say(_("%sChangelog:%s          \n%s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_changelog()->c_str());
		say(_("%sDescription:%s        \n%s\n"), CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_description()->c_str());

		if (showFilelist)
		{
			say(_("%sFilelist:%s\n"), CL_GREEN, CL_WHITE);
			if (pkgList.get_package(i)->installed())
			{
				core->db->get_filelist(pkgList.get_package(i)->get_id(), pkgList.get_package(i)->get_files());
				if (pkgList.get_package(i)->get_files()->size()==0)
				{
					say(_("\tPackage contains no files\n"));
				}
				else for (unsigned int t=0; t<pkgList.get_package(i)->get_files()->size(); t++)
				{
					say ("\t    %s", pkgList.get_package(i)->get_files()->at(t).get_name()->c_str());
					if (verbose) 
					{
						if (pkgList.get_package(i)->get_files()->at(t).get_type()==FTYPE_CONFIG)
						{
							say (_(" (config file)\n"));
						}
						else 
						{
							if (pkgList.get_package(i)->get_files()->at(t).get_type()==FTYPE_SYMLINK)
							{
								say(_(" (symlink)\n"));
							}
							else say (_(" (plain file)\n"));
						}
					}
					else say ("\n");
				}
			}
			else say (_("\tPackage is not installed\n"));
		}

	say("=============================================================================\n\n");
	}



}

void searchByFile(mpkg *core, string filename, bool strict)
{
	string filename_orig=filename;
	if (filename.length()==0) {
		say(_("No filename specified\n"));
		return;
	}
	if (filename[0]=='/') filename=filename.substr(1);
	//printf("searching for [%s]\n",filename.c_str());
	SQLRecord sqlSearch, sqlFields, sqlPkgSearch;
	
	if (!strict) sqlSearch.setEqMode(EQ_LIKE);
	sqlSearch.addField("file_name", &filename);
	sqlFields.addField("packages_package_id");
	sqlFields.addField("file_name");
	SQLTable results;
	core->db->get_sql_vtable(&results, sqlFields, "files", sqlSearch);
	PACKAGE_LIST pkgList;
	if (results.size()==0) {
		say(_("File %s doesn't belong to any installed package\n"),filename.c_str());
		return;
	}
	sqlPkgSearch.setSearchMode(SEARCH_OR);
	for (int i=0; i<results.size(); i++)
	{
		//printf("%s: %s\n", results.getValue(i, "file_name")->c_str(), results.getValue(i,"packages_package_id")->c_str());
		sqlPkgSearch.addField("package_id", results.getValue(i,"packages_package_id"));
	}
	core->get_packagelist(&sqlPkgSearch, &pkgList);
	if (pkgList.size()==0) {
		say(_("Hm... no package was returned. Seems to be malformed SQL query\n"));
		return;
	}
	say(_("File %s found in %d package(s):\n"),filename_orig.c_str(), pkgList.size());
	string pattern;
	bool showIt;
	vector<int> usedIDs;
	for (int i=0; i<results.size(); i++) {
		for (int t=0; t<pkgList.size(); t++) {
			if (*results.getValue(i,"packages_package_id")==IntToStr(pkgList.get_package(t)->get_id())) {
				if (verbose) {
					pattern=*results.getValue(i,"file_name");
					say(_("/%s: %s-%s\n"), pattern.c_str(), pkgList.get_package(t)->get_name()->c_str(), \
					pkgList.get_package(t)->get_fullversion().c_str());
				}
				else {
					showIt=true;
					for (unsigned int u = 0; u < usedIDs.size(); u++) {
						if (usedIDs[u]==pkgList.get_package(t)->get_id()) { 
							showIt=false;
							break;
						}
					}
					if (showIt) say(_("%s-%s\n"), pkgList.get_package(t)->get_name()->c_str(), \
					pkgList.get_package(t)->get_fullversion().c_str());

				}
				usedIDs.push_back(pkgList.get_package(t)->get_id());

			}
		}
	}


}

void list_pkglist(PACKAGE_LIST *pkglist)
{
	for (int i=0; i<pkglist->size(); i++)
	{
		say("[ %s ]\t", pkglist->get_package(i)->get_vstatus(true).c_str());
		say("%s-%s-%s-%s\t(%s)\n", \
			pkglist->get_package(i)->get_name()->c_str(), \
			pkglist->get_package(i)->get_version()->c_str(), \
			pkglist->get_package(i)->get_arch()->c_str(), \
			pkglist->get_package(i)->get_build()->c_str(), \
			pkglist->get_package(i)->get_short_description()->c_str());
	}
}




int list(mpkg *core, vector<string> search, bool onlyQueue)
{
	PACKAGE_LIST pkglist;
	SQLRecord sqlSearch;
	if (!search.empty())
	{
		sqlSearch.setEqMode(EQ_LIKE);
		for (unsigned int i=0; i<search.size(); i++)
		{
			sqlSearch.addField("package_name", &search[i]);
		}
	}
	if (onlyQueue)
	{
		sqlSearch.setSearchMode(SEARCH_IN);
		sqlSearch.addField("package_action", ST_INSTALL);
		sqlSearch.addField("package_action", ST_REMOVE);
		sqlSearch.addField("package_action", ST_PURGE);
		sqlSearch.addField("package_action", ST_REPAIR);
		sqlSearch.addField("package_action", ST_UPDATE);
	}
	say(_("Querying database...\n"));
	core->get_packagelist(&sqlSearch, &pkglist, true);
	if (pkglist.IsEmpty())
	{
		say(_("Search attempt has no results\n"));
		return 0;
	}
	bool showThis;
	for (int i=0; i<pkglist.size(); i++)
	{
		showThis=true;
		if (showOnlyAvailable && !pkglist.get_package(i)->available()) showThis=false;
		if (showOnlyInstalled && !pkglist.get_package(i)->installed()) showThis=false;
		if (showThis)
		{
			if (pkglist.get_package(i)->isRemoveBlacklisted()) say("*");
			else say(" ");
			if (!onlyQueue || pkglist.get_package(i)->action()!=ST_NONE)
			{
				say("[ %s ]\t", pkglist.get_package(i)->get_vstatus(true).c_str());
				say("%s-%s-%s-%s\t(%s)\n", \
					pkglist.get_package(i)->get_name()->c_str(), \
					pkglist.get_package(i)->get_version()->c_str(), \
					pkglist.get_package(i)->get_arch()->c_str(), \
					pkglist.get_package(i)->get_build()->c_str(), \
					pkglist.get_package(i)->get_short_description()->c_str());
			}
		}
	}
	return 0;
}

int check_action(char* act)
{
	std::string _act(act);	
	int res = 0;

	mDebug((string) "action = " + act);


	if ( _act != "install"
		&& _act != "remove"
	  	&& _act != "update"
	  	&& _act != "upgrade"
	  	&& _act != "list"
	  	&& _act != "search"
		&& _act != "index"
		&& _act != "purge"
		&& _act != "convert"
		&& _act != "convert_dir"
		&& _act != "list_rep"
		&& _act != "tag"
	  	&& _act != "clean"
		&& _act != "reset"
		&& _act != "commit"
		&& _act != "show_queue"
	  	&& _act != "show"
		&& _act != "installfromlist"
		&& _act != "test"
		&& _act != "check"
	  	&& _act != "menu"
	        && _act != "config"
		&& _act != "export"
		&& _act != "gendeps"
		&& _act != "filesearch"
		&& _act != "which"
		&& _act != "listgroup"
		&& _act != "installgroup"
		&& _act != "removegroup"
		&& _act != "whodepend"
		&& _act != "reinstall"
		&& _act != "upgradeall"
		&& _act != "listgroups"
		&& _act != "build"
		&& _act != "add_rep"
		&& _act != "delete_rep"
		&& _act != "enable_rep"
		&& _act != "disable_rep"
		//&& _act != "edit_rep"
		) {
		res = -1;
	}

	mDebug("res = " + IntToStr(res));

	return res;
}

int setup_action(char* act)
{
	std::string _act(act);
	if ( _act == "listgroups")
		return ACT_LISTGROUPS;
	if ( _act == "upgradeall")
		return ACT_UPDATEALL;
	if ( _act == "reinstall")
			return ACT_REINSTALL;
	if ( _act == "whodepend")
			return ACT_LISTDEPENDANTS;
	if ( _act == "check")
			return ACT_CHECKDAMAGE;
	if ( _act == "test" )
			return ACT_TEST;

	if ( _act == "installfromlist" )
			return ACT_INSTALLFROMLIST;

	if ( _act == "install" )
			return ACT_INSTALL;

	if ( _act == "remove" )
			return ACT_REMOVE;

	if ( _act == "search" )
		   return ACT_SEARCH;

	if ( _act == "list")
		return 	ACT_LIST;

	if ( _act == "update" )
		return ACT_UPDATE;

	if ( _act == "upgrade" )
		return ACT_UPGRADE;

	if ( _act == "clean" )
		return ACT_CLEAN;

	if (_act == "index" )
		return ACT_INDEX;

	if (_act == "purge" )
		return ACT_PURGE;

	if (_act == "convert")
		return ACT_CONVERT;

	if (_act == "tag")
		return ACT_TAG;

	if (_act == "convert_dir")
		return ACT_CONVERT_DIR;

	if (_act == "list_rep")
		return ACT_LIST_REP;

	if (_act == "reset")
		return ACT_RESETQUEUE;

	if (_act == "show_queue")
		return ACT_SHOWQUEUE;

	if (_act == "commit")
		return ACT_COMMIT;
	if (_act == "show")
		return ACT_SHOW;
	if (_act == "menu")
		return ACT_PACKAGEMENU;
	if (_act == "config")
		return ACT_CONFIG;
	if (_act == "export")
		return ACT_EXPORT;
	if (_act == "gendeps")
		return ACT_GENDEPS;
	if (_act == "filesearch")
		return ACT_FILESEARCH;
	if (_act == "which")
		return ACT_WHICH;
	if (_act == "listgroup")
		return ACT_LISTGROUP;
	if (_act == "installgroup")
		return ACT_INSTALLGROUP;
	if (_act == "removegroup")
		return ACT_REMOVEGROUP;
	if (_act == "build")
		return ACT_BUILD;
	if (_act == "add_rep") return ACT_ADD_REPOSITORY;
	if (_act == "delete_rep") return ACT_REMOVE_REPOSITORY;
	if (_act == "enable_rep") return ACT_ENABLE_REPOSITORY;
	if (_act == "disable_rep") return ACT_DISABLE_REPOSITORY;
	//if (_act == "edit_rep") return ACT_EDIT_REPOSITORY;

	return ACT_NONE;
}


