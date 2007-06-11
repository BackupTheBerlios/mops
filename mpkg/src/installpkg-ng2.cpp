/**
 *	MOPSLinux packaging system    
 *	    installpkg-ng
 *	New generation of installpkg :-)
 *	This tool ONLY can install concrete local file, but in real it can do more :-) 
 *	
 *	$Id: installpkg-ng2.cpp,v 1.27 2007/06/11 03:56:39 i27249 Exp $
 */

#include "libmpkg.h"
#include "converter.h"
#include "dialog.h"
const char* program_name;
extern char* optarg;
extern int optind, opterr, optopt;
//string output_dir;
//static LoggerPtr rootLogger;
void show_package_info(mpkg *core, string name);

int setup_action(char* act);
int check_action(char* act);
void print_usage(FILE* stream, int exit_code);
int list(mpkg *core, vector<string> search, bool onlyQueue=false);
void ShowBanner();
int list_rep(mpkg *core);

void ShowBanner()
{
	char *version="0.9 beta 1 (public preview state)";
	char *copyright="\(c) 2006-2007 RPUNet (http://www.rpunet.ru)";
	say("MOPSLinux packaging system v.%s\n%s\n--\n", version, copyright);
}

int main (int argc, char **argv)
{
	mpkg core;
	if (!core.init_ok)
	{
		mError("Error initializing CORE\n");
		abort();
	}
	if ((string) argv[0] == "buildpkg")
	{
	    core.build_package();
	    return 0;
	}
	
	ShowBanner();
	setlocale(LC_ALL, "C");
	bindtextdomain( "installpkg-ng", "/usr/share/locale");
	textdomain("installpkg-ng");


	int verbose = 0;

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

	if ( uid != 0 ) {
		mError("You must login as root to run this program");
		exit(1);
	}
	

	int ich;
	const char* short_opt = "hvp";
	const struct option long_options[] =  {
		{ "help",	0,	NULL, 'h'},
		{ "verbose", 0, NULL, 'v'},
		{ "purge", 0, NULL, 'p'},
		{ NULL, 0, NULL, 0}
	};

	program_name = argv[0];

	do {
		ich = getopt_long(argc, argv, short_opt, long_options, NULL);
		

		switch (ich) {
			case 'h':
					print_usage(stdout, 0);

			case 'v':
					verbose = 1;
					break;

			case 'p':
					do_purge = 1;
					break;
					
			case '?':
					print_usage(stderr, 1);

			case -1:
					break;

			default:
					abort();
		}
	
	}  while ( ich != -1 );


	if ( optind < argc ) {
		if ( check_action( argv[optind++] ) == -1 )
				print_usage(stderr, 1);
		
		action = setup_action( argv[optind-1] );
	}

	if ( action == ACT_NONE )
			print_usage(stderr, 1);
	if ( action == ACT_SHOW)
	{
		show_package_info(&core, argv[optind]);
		// show info about package
	}
	vector<string> fname;
	vector<string> pname;
	if (action == ACT_COMMIT)
	{
		core.commit();
		return 0;
	}

	if (action == ACT_SHOWQUEUE)
	{
		vector<string> list_empty;
		list(&core, list_empty,true);
		delete_tmp_files();
		return 0;
	}

	if (action == ACT_RESETQUEUE)
	{
		PACKAGE_LIST tmp;
		SQLRecord sqlSearch;
		sqlSearch.setSearchMode(SEARCH_OR);
		sqlSearch.addField("package_action", ST_INSTALL);
		sqlSearch.addField("package_action", ST_REMOVE);
		sqlSearch.addField("package_action", ST_PURGE);

		core.get_packagelist(&sqlSearch, &tmp);
		for (int i=0; i<tmp.size(); i++)
		{
			core.unqueue(tmp.get_package(i)->get_id());
		}
		//core.commit();
		delete_tmp_files();
		return 0;
	}

	if (action == ACT_PACKAGEMENU)
	{
		Dialog dialogItem;
		vector<TagPair> pkgList;
		SQLRecord sqlSearch;
		PACKAGE_LIST packageList;
		core.get_packagelist(&sqlSearch, &packageList);
		for (int i=0; i<packageList.size(); i++)
		{
			pkgList.push_back(TagPair(*packageList.get_package(i)->get_name(),\
					       *packageList.get_package(i)->get_short_description(),\
					       packageList.get_package(i)->installed()));
		}
		vector<TagPair> oldState = pkgList;
		if (dialogItem.execCheckList("Отредактируйте состояние пакетов", 0,0,0, &pkgList))
		{
			vector<string> removeList;
			vector<string> installList;
			for (unsigned int i=0; i<oldState.size(); i++)
			{
				if (oldState[i].checkState!=pkgList[i].checkState)
				{
					if (pkgList[i].checkState) installList.push_back(pkgList[i].tag);
					else removeList.push_back(pkgList[i].tag);
				}
			}
			printf("Install list:\n");
			for (unsigned int i=0; i<installList.size(); i++)
			{
				printf("INSTALL: %s\n", installList[i].c_str());
			}
			for (unsigned int i=0; i<removeList.size(); i++)
			{
				printf("REMOVE: %s\n", removeList[i].c_str());
			}

			core.uninstall(removeList);
			core.install(installList);
			core.commit();
		}
		else printf("Cancelled\n");
		return 0;
	}

	if (action == ACT_INSTALLFROMLIST)
	{
		if (argc>2)
		{
			// Read the file
			FILE *install_list = fopen(argv[optind], "r");
			if (!install_list)
			{
				perror("cannot open installation list");
				exit(-1);
			}
			char membuff[2000];
			vector<string> installQuery;
			memset(&membuff, 0, sizeof(membuff));
			string tmp;
			while (fscanf(install_list, "%s", &membuff)!=EOF)
			{
				tmp = (string) membuff;
				installQuery.push_back(tmp.substr(1, tmp.size()-2));
			}
			fclose(install_list);
			for (unsigned int i=0; i<installQuery.size(); i++)
			{
				mDebug("Installing from list " + installQuery[i]);
			}
			core.install(installQuery);
			core.commit();
			delete_tmp_files();
			return 0;
		}
		else
		{
			mError("Please select input file\n");
			exit(-1);
		}
	}				

	if (action == ACT_INSTALL)
	{
		for (int i = optind; i < argc; i++)
		{
			fname.push_back((string) argv[i]);
		}
		core.install(fname);
		core.commit();
		delete_tmp_files();
		return 0;
	}
	if (action == ACT_TEST)
	{
		printf("No test for today\n");
		return 0;

		/*

		TagPair tmp;
		vector<TagPair> optList;
		tmp.tag="1";tmp.value="Run gauge";
		optList.push_back(tmp);
		tmp.tag="2";
		tmp.value="Print a message";
		optList.push_back(tmp);
		string act = dialogItem.execMenu("Choose an action", 0,0,0, optList);
		if (act.empty())
		{
			printf("cancelled\n");
			delete_tmp_files();
			return 0;
		}
		if (act=="2")
		{
			printf("Message choosed\n");
			delete_tmp_files();
			return 0;
		}
		if (act=="1")
		{
		for (unsigned int i=0; i<=100; i++)
		{
			dialogItem.execGauge("Testing gauge engine", 8,60,i);
		//	sleep(1);
		}
		printf("Developing console dialog class\n");
		return 0;
		}
		printf("unknown return %s\n", act.c_str());
		*/


	}
	if (action == ACT_REMOVE)
	{
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		if (do_purge==0) core.uninstall(pname);
		else core.purge(pname);
		core.commit();
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_SEARCH ) {
		vector<string> list_search;
		for (int i = optind; i < argc; i++)
		{
			list_search.push_back((string) argv[i]);
		}
		list(&core, list_search);
		delete_tmp_files();
		return 0;
	}
	if (action == ACT_UPGRADE ) {
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		core.install(pname);
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
		//mDebug("argc = %d\nargv[2] = %s\nargv[3] = %s\n", argc, argv[2], argv[3]);
		if (argc == 4)
		{
			say("tagging...\n");
			tag_package(argv[3], argv[2]);
		}
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_CONVERT_DIR ) {
		if (optind < argc )
		{
			//output_dir=(string) argv[optind];
			core.convert_directory((string) argv[optind]);
		}
		else 
		{
			mError("Please define output directory");
		}
		delete_tmp_files();

		return 0;
	}

	if ( action == ACT_LIST ) {
		vector<string> list_empty;
		list(&core, list_empty);
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_UPDATE ) {
		core.update_repository_data();
		delete_tmp_files();
		return 0;
		
	}

	if ( action == ACT_CLEAN ) {
		core.clean_cache();	
		return 0;
	
	}

	if ( action == ACT_INDEX ) {
		if (argc == 4)
		{
			
			say("Building repository index\nURL: %s\nName: %s\n", argv[optind], argv[optind+1]);
			core.rep.build_index(argv[optind], argv[optind+1]);
		}
		else
		{
			if (FileExists("packages.xml.gz"))
			{
				core.rep.build_index("", "", true);
			}
			else
			{
				mError("To few arguments to index\n");
				print_usage(stderr, 1);
			}
		}
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_PURGE ) {
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		core.purge(pname);
		core.commit();
		delete_tmp_files();
		return 0;
	}

	if ( action == ACT_LIST_REP ) {
		list_rep(&core);
		delete_tmp_files();
		return 0;
	}

	// ACTION SUMMARY - NEED TO FIX!
	if (!core.DepTracker->get_install_list()->IsEmpty())
	{
		say("Next packages will be installed:\n");
		for (int i=0;i<core.DepTracker->get_install_list()->size();i++)
		{
			say("%s\n", core.DepTracker->get_install_list()->get_package(i)->get_name()->c_str());
		}
	}

	if (!core.DepTracker->get_remove_list()->IsEmpty())
	{
		say("\nNext packages will be removed:\n");
		for (int i=0;i<core.DepTracker->get_remove_list()->size();i++)
		{
			say("%s\n", core.DepTracker->get_remove_list()->get_package(i)->get_name()->c_str());
		}
	}

	if (!core.DepTracker->get_failure_list()->IsEmpty())
	{

		say("Next packages is failed to install:\n");
		for (int i=0;i<core.DepTracker->get_failure_list()->size();i++)
		{
			say("%s: ", core.DepTracker->get_failure_list()->get_package(i)->get_name()->c_str());
			//core.DepTracker->PrintFailure(core.DepTracker->get_failure_list()->get_package(i));
		}
	}
	say("\n");

	return 0;
}


void print_usage(FILE* stream, int exit_code)
{
	fprintf(stream, _("\nUsage: %s [options] action package [package ...]\n"), program_name);
	fprintf(stream,_("Options:\n"));
	fprintf(stream,_("\t-h    --help       show this help\n"));
	fprintf(stream,_("\t-v    --verbose    be verbose\n"));
	fprintf(stream,_("\nActions:\n"));
	fprintf(stream,_("\tinstall    install packages\n"));
	fprintf(stream,_("\tupgrade    upgrade selected package or full system if no package selected\n"));
	fprintf(stream,_("\tremove     remove selected package\n"));
	fprintf(stream,_("\tpurge      purge selected package\n"));
	fprintf(stream,_("\tshow       show info about package\n"));
	fprintf(stream,_("\tshow_queue show actions queue\n"));
	fprintf(stream,_("\tupdate     update packages info\n"));
	fprintf(stream,_("\tlist       list installed packages\n"));
	fprintf(stream,_("\tlist_rep   list enabled repositories\n"));
	fprintf(stream,_("\tinstallfromlist install using file with list of items\n"));
	fprintf(stream,_("\treset      reset queue\n"));
	fprintf(stream,_("\tshow_queue show queue\n"));
	fprintf(stream,_("\tcommit     commit queued actions\n"));
	fprintf(stream,_("\tsearch     search package by name\n"));
	fprintf(stream,_("\tclean      remove all packages from cache\n"));
	fprintf(stream,_("\nRepository maintaining functions:\n"));
	fprintf(stream,_("\tindex <server_url> <file_url>     create a repository index file \"packages.xml.gz\"\n"));
	fprintf(stream,_("\tconvert <package>      convert specified packages from Slackware to MPKG format\n"));
	fprintf(stream,_("\tconvert_dir <outp_dir> convert whole directory (including sub-dirs) to MPKG format\n"));
	fprintf(stream, "\n");


	exit(exit_code);
}

int list_rep(mpkg *core)
{
	say("Repository list:\n");
	vector <string> rlist=core->get_repositorylist();
	for (unsigned int i=0; i<rlist.size(); i++)
	{
		say("[%d] %s\n", i+1, rlist[i].c_str());
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
		say("No such package\n");
	}
	for (int i=0; i<pkgList.size(); i++)
	{
		say("%sID:%s                 %d\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_id());
		say("%sName:%s               %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_name()->c_str());
		say("%sVersion:%s            %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_version()->c_str());
		say("%sArch:%s               %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_arch()->c_str());
		say("%sBuild:%s              %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_build()->c_str());
		say("%sPackage size:%s       %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_compressed_size()->c_str());
		say("%sInstalled size:%s     %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_installed_size()->c_str());
		say("%sMaintainer:%s         %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_packager()->c_str());
		say("%sMaintainer e-mail:%s  %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_packager_email()->c_str());
		say("%sStatus:%s             %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_vstatus(true).c_str());

		say("%sMD5:%s                %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_md5()->c_str());
		say("%sFilename:%s           %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_filename()->c_str());
		say("%sShort description:%s  %s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_short_description()->c_str());
		
		if (pkgList.get_package(i)->available()) {
			say("%sLocations:%s\n", CL_GREEN, CL_WHITE);
			for (unsigned int t=0; t<pkgList.get_package(i)->get_locations()->size(); t++)
			{
				say("\t\t    %s\n", pkgList.get_package(i)->get_locations()->at(t).get_full_url().c_str());
			}
		}
		else say("%sLocations:%s          %s\n", CL_GREEN, CL_WHITE, "none");
		
		say("%sTags:%s               \n", CL_GREEN, CL_WHITE);
		if (pkgList.get_package(i)->get_tags()->empty())
		{
			say("none\n");
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
			say("%sDepends on:%s\n", CL_GREEN, CL_WHITE);
			for (unsigned int t=0; t<pkgList.get_package(i)->get_dependencies()->size(); t++)
			{
				say("\t\t    %s\n", pkgList.get_package(i)->get_dependencies()->at(t).getDepInfo().c_str());
			}
		}

		say("%sDescription:%s        \n%s\n", CL_GREEN, CL_WHITE, pkgList.get_package(i)->get_description()->c_str());
	}
	//	say("%sMaintainer e-mail:%s  %s\n", CL_GREEN, CL_WHITE, pkgList->get_package(i)->get_packager_email()->c_str());
	//	say("%sMaintainer e-mail:%s  %s\n", CL_GREEN, CL_WHITE, pkgList->get_package(i)->get_packager_email()->c_str());
	//	say("%sMaintainer e-mail:%s  %s\n", CL_GREEN, CL_WHITE, pkgList->get_package(i)->get_packager_email()->c_str());



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
		sqlSearch.setSearchMode(SEARCH_OR);
		sqlSearch.addField("package_action", ST_INSTALL);
		sqlSearch.addField("package_action", ST_REMOVE);
		sqlSearch.addField("package_action", ST_PURGE);
	}

	core->get_packagelist(&sqlSearch, &pkglist);
	if (pkglist.IsEmpty())
	{
		if (!search.empty()) say("Package database is empty\n");
		else say("Nothing is found\n");
		return 0;
	}
	for (int i=0; i<pkglist.size(); i++)
	{
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
	  	&& _act != "menu" ) {
		res = -1;
	}

	mDebug("res = " + IntToStr(res));

	return res;
}

int setup_action(char* act)
{
	std::string _act(act);

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

	return ACT_NONE;
}


