/**
 *	MOPSLinux packaging system    
 *	    installpkg-ng
 *	New generation of installpkg :-)
 *	This tool ONLY can install concrete local file, but in real it can do more :-) 
 *	
 *	$Id: installpkg-ng2.cpp,v 1.3 2007/03/26 14:32:32 i27249 Exp $
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
int list(mpkg *core, vector<string> search);
void ShowBanner();
int list_rep(mpkg *core);

void ShowBanner()
{
	char *version="0.1 beta 3 (shared lib)";
	char *copyright="\(c) 2006-2007 RPUNet (http://www.rpunet.ru)";
	printf("MOPSLinux packaging system v.%s\n%s\n--\n", version, copyright);
}

int main (int argc, char **argv)
{
	mpkg core;
	if (!core.init_ok)
	{
		printf("Error initializing CORE\n");
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

#ifdef DEBUG
	printf("DEBUG: uid = %i, euid = %i\n", uid, euid);
#endif

	if ( uid != 0 ) {
		fprintf(stderr, _("You must login as root to run this program\n"));
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


#ifdef DEBUG
	printf("DEBUG: last optind = %i\n", optind);
	printf("DEBUG: argc = %i\n", argc);
#endif

	if ( optind < argc ) {
		if ( check_action( argv[optind++] ) == -1 )
				print_usage(stderr, 1);
		
		action = setup_action( argv[optind-1] );
	}

	if ( action == ACT_NONE )
			print_usage(stderr, 1);
	
	vector<string> fname;
	vector<string> pname;

	if (action == ACT_INSTALL)
	{
		printf("install\n");
		for (int i = optind; i < argc; i++)
		{
			fname.push_back((string) argv[i]);
		}
		printf("fname size = %d\n", fname.size());
		core.install(fname);
		printf("core committing\n");
		core.commit();
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
		return 0;
	}

	if ( action == ACT_SEARCH ) {
		vector<string> list_search;
		for (int i = optind; i < argc; i++)
		{
			list_search.push_back((string) argv[i]);
		}
		list(&core, list_search);
		return 0;
	}
	if (action == ACT_UPGRADE ) {
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		core.upgrade(pname);
		return 0;
	}

	if ( action == ACT_CONVERT  ) {
		for (int i = optind; i < argc; i++)
		{
			printf("temporarily unimplemented\n");
			//core.convert_package((string) argv[i], "/root/development/converted/");
		}
		return 0;
	
	}

	if ( action == ACT_TAG )
	{
		printf("argc = %d\nargv[2] = %s\nargv[3] = %s\n", argc, argv[2], argv[3]);
		if (argc == 4)
		{
			printf("tagging...\n");
			tag_package(argv[3], argv[2]);
		}
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
			printf("Please define output directory\n");
		}
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
		return 0;
		
	}

	if ( action == ACT_CLEAN ) {
		core.clean_cache();	
		return 0;
	
	}

	if ( action == ACT_INDEX ) {
		if (optind < argc)
		{
			core.rep.build_index(argv[optind]);
		}
		else
		{
			core.rep.build_index();
		}
		return 0;
	}

	if ( action == ACT_PURGE ) {
		for (int i = optind; i < argc; i++)
		{
			pname.push_back((string) argv[i]);
		}
		core.purge(pname);
		core.commit();
		return 0;
	}

	if ( action == ACT_LIST_REP ) {
		list_rep(&core);
		return 0;
	}

	// ACTION SUMMARY - NEED TO FIX!
	if (!core.DepTracker->get_install_list()->IsEmpty())
	{
		printf(_("Next packages will be installed:\n"));
		for (int i=0;i<core.DepTracker->get_install_list()->size();i++)
		{
			printf("%s\n", core.DepTracker->get_install_list()->get_package(i)->get_name(false).c_str());
		}
	}

	if (!core.DepTracker->get_remove_list()->IsEmpty())
	{
		printf(_("\nNext packages will be removed:\n"));
		for (int i=0;i<core.DepTracker->get_remove_list()->size();i++)
		{
			printf("%s\n", core.DepTracker->get_remove_list()->get_package(i)->get_name(false).c_str());
		}
	}

	if (!core.DepTracker->get_failure_list()->IsEmpty())
	{

		printf(_("Next packages is failed to install:\n"));
		for (int i=0;i<core.DepTracker->get_failure_list()->size();i++)
		{
			printf("%s: ", core.DepTracker->get_failure_list()->get_package(i)->get_name(false).c_str());
			core.DepTracker->PrintFailure(core.DepTracker->get_failure_list()->get_package(i));
		}
	}
	printf("\n");

	return 0;
}


void print_usage(FILE* stream, int exit_code)
{
	fprintf(stream, _("Usage: %s [options] action package [package ...]\n"), program_name);
	fprintf(stream,_("Options:\n"));
	fprintf(stream,_("\t-h    --help       show this help\n"));
	fprintf(stream,_("\t-v    --verbose    be verbose\n"));
	fprintf(stream,_("\t-p    --purge      purge package\n\n"));
	fprintf(stream,_("Actions:\n"));
	fprintf(stream,_("\tinstall    install packages\n"));
	fprintf(stream,_("\tupgrade    upgrade selected package or full system if no package selected\n"));
	fprintf(stream,_("\tremove     remove selected package\n"));
	fprintf(stream,_("\tpurge      purge selected package\n"));
	fprintf(stream,_("\tupdate     update packages info\n"));
	fprintf(stream,_("\tlist       list installed packages\n"));
	fprintf(stream,_("\tlist_rep   list enabled repositories\n"));

	fprintf(stream,_("\tsearch     search package\n"));
	fprintf(stream,_("\tclean      remove all packages from cache\n"));
	fprintf(stream,_("Repository maintaining functions:\n"));
	fprintf(stream,_("\tindex [server_url]     create a repository index file \"packages.xml\"\n"));
	fprintf(stream,_("\tconvert [package]      convert specified packages from Slackware to MPKG format\n"));
	fprintf(stream,_("\tconvert_dir [outp_dir] convert whole directory (including sub-dirs) to MPKG format\n"));


	exit(exit_code);
}

int list_rep(mpkg *core)
{
	printf("Repository list:\n");
	vector <string> rlist=core->get_repositorylist();
	for (unsigned int i=0; i<rlist.size(); i++)
	{
		printf("[%d] %s\n", i+1, rlist[i].c_str());
	}
	return 0;
}

int list(mpkg *core, vector<string> search)
{
	PACKAGE_LIST pkglist;
	SQLRecord sqlSearch;
	if (!search.empty())
	{
		sqlSearch.setEqMode(EQ_LIKE);
		for (unsigned int i=0; i<search.size(); i++)
		{
			sqlSearch.addField("package_name", search[i]);
		}
	}
	core->get_packagelist(sqlSearch, &pkglist, false);
	if (pkglist.IsEmpty())
	{
		printf(_("Package database empty\n"));
		return 0;
	}

	for (int i=0; i<pkglist.size(); i++)
	{
		printf("[%s]\t%s-%s-%s-%s\t(%s)\n", \
				pkglist.get_package(i)->get_vstatus().c_str(), \
				pkglist.get_package(i)->get_name().c_str(), \
				pkglist.get_package(i)->get_version().c_str(), \
				pkglist.get_package(i)->get_arch().c_str(), \
				pkglist.get_package(i)->get_build().c_str(), \
				pkglist.get_package(i)->get_short_description().c_str());
	}
	return 0;
}

int check_action(char* act)
{
	std::string _act(act);	
	int res = 0;

#ifdef DEBUG
	printf("DEBUG: action = %s\n", act);
#endif

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
	  	&& _act != "clean" ) {
		res = -1;
	}

#ifdef DEBUG
	printf("DEBUG: res = %i\n", res);
#endif

	return res;
}

int setup_action(char* act)
{
	std::string _act(act);

#ifdef DEBUG
	printf("[setup_action] DEBUG: action = %s\n", act);
#endif

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

	return ACT_NONE;
}


