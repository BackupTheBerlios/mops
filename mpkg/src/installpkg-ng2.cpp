/**
 *	MOPSLinux packaging system    
 *	    installpkg-ng
 *	New generation of installpkg :-)
 *	This tool ONLY can install concrete local file, but in real it can do more :-) 
 *	
 *	$Id: installpkg-ng2.cpp,v 1.22 2007/05/21 23:52:14 i27249 Exp $
 */

#include "libmpkg.h"
#include "converter.h"

const char* program_name;
extern char* optarg;
extern int optind, opterr, optopt;
//string output_dir;
//static LoggerPtr rootLogger;

int setup_action(char* act);
int check_action(char* act);
void print_usage(FILE* stream, int exit_code);
int list(mpkg *core, vector<string> search, bool onlyQueue=false);
void ShowBanner();
int list_rep(mpkg *core);

void ShowBanner()
{
	char *version="0.1 beta N (debug state)";
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

		core.get_packagelist(&sqlSearch, &tmp, false, true);
		for (int i=0; i<tmp.size(); i++)
		{
			core.unqueue(tmp.get_package(i)->get_id());
		}
		//core.commit();
		delete_tmp_files();
		return 0;
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
		get_disk_freespace("/");
		delete_tmp_files();
		return 0;
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
	fprintf(stream,_("\tupdate     update packages info\n"));
	fprintf(stream,_("\tlist       list installed packages\n"));
	fprintf(stream,_("\tlist_rep   list enabled repositories\n"));
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

	core->get_packagelist(&sqlSearch, &pkglist, false, false);
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
	  	&& _act != "test" ) {
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

	return ACT_NONE;
}


