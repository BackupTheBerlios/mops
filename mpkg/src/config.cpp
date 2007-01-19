/******************************************************
 * MOPSLinux packaging system - global configuration
 * $Id: config.cpp,v 1.1 2007/01/19 06:13:59 i27249 Exp $
 *
 * ***************************************************/

#include "config.h"
#include "xmlParser.h"

bool DO_NOT_RUN_SCRIPTS;
string SYS_ROOT;
string SYS_CACHE;
string SCRIPTS_DIR;
unsigned int DATABASE;
string DB_FILENAME;
vector<string> REPOSITORY_LIST;

int loadGlobalConfig(string config_file)
{
	string run_scripts="yes";
	string sys_root="/root/development/sys_root/";
	string sys_cache="/root/development/sys_cache/";
	string db_url="sqlite://var/log/mpkg/packages.db";
	string scripts_dir="/var/log/mpkg/scripts";
	string sql_type;
	vector<string> repository_list;
	XMLNode config=XMLNode::openFileHelper(config_file.c_str(), "mpkgconfig");
	if (config.nChildNode("run_scripts")!=0)
		run_scripts=(string) config.getChildNode("run_scripts").getText();
	if (config.nChildNode("sys_root")!=0)
		sys_root=(string) config.getChildNode("sys_root").getText();
	if (config.nChildNode("sys_cache")!=0)
		sys_cache=(string) config.getChildNode("sys_cache").getText();
	if (config.nChildNode("database_url")!=0)
		db_url=(string) config.getChildNode("database_url").getText();
	if (config.nChildNode("repository_list")!=0)
	{
		for (int i=0;i<config.getChildNode("repository_list").nChildNode("repository");i++)
		{
			repository_list.push_back((string) config.getChildNode("repository_list").getChildNode("repository",i).getText());
		}
	}
	if (config.nChildNode("scripts_dir")!=0)
	{
		scripts_dir = (string) config.getChildNode("scripts_dir").getText();
	}

	// parsing results
	
	// run_scripts
	if (run_scripts=="yes")
		DO_NOT_RUN_SCRIPTS=false;
	if (run_scripts=="no")
		DO_NOT_RUN_SCRIPTS=true;
	if (run_scripts!="yes" && run_scripts!="no")
	{
		fprintf(stderr, "Error in config file, option <run_scripts> should be \"yes\" or \"no\", without quotes\n");
		return -1; // CONFIG ERROR in scripts
	}
	
	// sys_root
	SYS_ROOT=sys_root;
	//sys_cache
	SYS_CACHE=sys_cache;
	SCRIPTS_DIR=scripts_dir;	
	if (db_url.find("sqlite://")!=std::string::npos)
	{
		sql_type="sqlite://";

		DATABASE=DB_SQLITE_LOCAL;
		db_url.erase(0,sql_type.length()-1);
		DB_FILENAME=db_url;
	}
	REPOSITORY_LIST=repository_list;
#ifdef DEBUG
	printf("System configuration:\n\trun_scripts: %s\n\tsys_root: %s\n\tsys_cache: %s\n\tSQL type: %s\n\tSQL filename: %s\n", \
			run_scripts.c_str(), sys_root.c_str(), sys_cache.c_str(), sql_type.c_str(), db_url.c_str());
#endif

	return 0;
}
