/******************************************************
 * MOPSLinux packaging system - global configuration
 * $Id: config.cpp,v 1.11 2007/03/22 16:40:10 i27249 Exp $
 *
 * ***************************************************/

#include "config.h"
#include "xmlParser.h"

bool DO_NOT_RUN_SCRIPTS;
unsigned int fileConflictChecking = CHECKFILES_PREINSTALL;
string currentStatus;
int currentProgress;
unsigned int progressMax;
bool progressEnabled = false;
string SYS_ROOT;
string SYS_CACHE;
string SCRIPTS_DIR;
unsigned int DATABASE;
string DB_FILENAME;
vector<string> REPOSITORY_LIST;
vector<string> DISABLED_REPOSITORY_LIST;
//vector<CDROM_DEVICE> CDROMS;
int loadGlobalConfig(string config_file)
{
	currentStatus = "Loading configuration...";
	string run_scripts="yes";
	string check_files = "preinstall";
	string sys_root="/root/development/sys_root/";
	string sys_cache="/root/development/sys_cache/";
	string db_url="sqlite://var/log/mpkg/packages.db";
	string scripts_dir="/var/log/mpkg/scripts/";
	string sql_type;
//	vector<CDROM_DEVICE> cdrom_list; // CD-ROM descriptors
	vector<string> repository_list;
	vector<string> disabled_repository_list;
	bool conf_init=false;
	if (access(config_file.c_str(), R_OK)==0)
	{
		XMLNode config=XMLNode::openFileHelper(config_file.c_str(), "mpkgconfig");
		if (config.nChildNode("run_scripts")!=0)
			run_scripts=(string) config.getChildNode("run_scripts").getText();
		if (config.nChildNode("checkFileConflicts")!=0)
			check_files = (string) config.getChildNode("checkFileConflicts").getText();
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
			for (int i=0; i<config.getChildNode("repository_list").nChildNode("disabled_repository"); i++)
			{
				disabled_repository_list.push_back((string) config.getChildNode("repository_list").getChildNode("disabled_repository").getText());
			}
		}
		if (config.nChildNode("scripts_dir")!=0)
		{
			scripts_dir = (string) config.getChildNode("scripts_dir").getText();
		}

	}
	else
	{
		conf_init=true;
		printf("Configuration file /etc/mpkg.xml not found, using defaults and creating config\n");
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
	if (check_files == "preinstall")
		fileConflictChecking = CHECKFILES_PREINSTALL;
	if (check_files == "postinstall")
		fileConflictChecking = CHECKFILES_POSTINSTALL;
	if (check_files == "disable")
		fileConflictChecking = CHECKFILES_DISABLE;
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
	DISABLED_REPOSITORY_LIST = disabled_repository_list;
#ifdef DEBUG
	printf("System configuration:\n\trun_scripts: %s\n\tsys_root: %s\n\tsys_cache: %s\n\tSQL type: %s\n\tSQL filename: %s\n", \
			run_scripts.c_str(), sys_root.c_str(), sys_cache.c_str(), sql_type.c_str(), db_url.c_str());
#endif
	if (conf_init) mpkgconfig::initConfig();

	currentStatus = "Settings loaded";
	return 0;
}

XMLNode mpkgconfig::getXMLConfig(string conf_file)
{
	debug("getXMLConfig");
	XMLNode config;
	if (access(conf_file.c_str(), R_OK)==0)
	{
		config=XMLNode::openFileHelper(conf_file.c_str(), "mpkgconfig");
	}
	else config=XMLNode::createXMLTopNode("mpkgconfig");

	if (config.nChildNode("run_scripts")==0)
	{
		config.addChild("run_scripts");
		if (get_runscripts()) config.getChildNode("run_scripts").addText("yes");
		else config.getChildNode("run_scripts").addText("no");
	}
	if (config.nChildNode("checkFileConflicts")==0)
	{
		config.addChild("checkFileConflicts");
		if (get_checkFiles()==CHECKFILES_PREINSTALL) config.getChildNode("checkFileConflicts").addText("preinstall");
		if (get_checkFiles()==CHECKFILES_POSTINSTALL) config.getChildNode("checkFileConflicts").addText("postinstall");
		if (get_checkFiles()==CHECKFILES_DISABLE) config.getChildNode("checkFileConflicts").addText("disable");

	}

	if (config.nChildNode("sys_root")==0)
	{
		config.addChild("sys_root");
		config.getChildNode("sys_root").addText(get_sysroot().c_str());
	}

	if (config.nChildNode("sys_cache")==0)
	{
		config.addChild("sys_cache");
		config.getChildNode("sys_cache").addText(get_syscache().c_str());
	}

	if (config.nChildNode("database_url")==0)
	{
		config.addChild("database_url");
		config.getChildNode("database_url").addText(get_dburl().c_str());
	}

	if (config.nChildNode("repository_list")==0)
	{
		config.addChild("repository_list");
	}

	if (config.nChildNode("scripts_dir")==0)
	{
		config.addChild("scripts_dir");
		config.getChildNode("scripts_dir").addText(get_scriptsdir().c_str());
	}
	debug("getXMLConfig end");
	return config;
}

int mpkgconfig::initConfig()
{
	XMLNode tmp=getXMLConfig();
	return setXMLConfig(tmp);
}

int mpkgconfig::setXMLConfig(XMLNode xmlConfig, string conf_file)
{
	if (xmlConfig.writeToFile(conf_file.c_str())!=0) printf("error writing config file");
	loadGlobalConfig();
	return 0;
}

string mpkgconfig::get_sysroot()
{
	return SYS_ROOT;
}

string mpkgconfig::get_syscache()
{
	return SYS_CACHE;
}

vector<string> mpkgconfig::get_repositorylist()
{
	return REPOSITORY_LIST;
}

vector<string> mpkgconfig::get_disabled_repositorylist()
{
	return DISABLED_REPOSITORY_LIST;
}

string mpkgconfig::get_dburl()
{
	return "sqlite://"+DB_FILENAME;
}

string mpkgconfig::get_scriptsdir()
{
	return SCRIPTS_DIR;
}

bool mpkgconfig::get_runscripts()
{
	return !DO_NOT_RUN_SCRIPTS;
}

unsigned int mpkgconfig::get_checkFiles()
{
	return fileConflictChecking;
}
int mpkgconfig::set_repositorylist(vector<string> newrepositorylist)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("repository_list").deleteNodeContent(1);
	tmp.addChild("repository_list");
	for (unsigned int i=0; i<newrepositorylist.size(); i++)
	{
		tmp.getChildNode("repository_list").addChild("repository");
		tmp.getChildNode("repository_list").getChildNode("repository", i).addText(newrepositorylist[i].c_str());
	}
	return setXMLConfig(tmp);
}

int mpkgconfig::set_disabled_repositorylist(vector <string> newrepositorylist)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("repository_list").deleteNodeContent(1);
	tmp.addChild("repository_list");
	for (unsigned int i=0; i<newrepositorylist.size(); i++)
	{
		tmp.getChildNode("repository_list").addChild("disabled_repository");
		tmp.getChildNode("repository_list").getChildNode("disabled_repository", i).addText(newrepositorylist[i].c_str());
	}
	return setXMLConfig(tmp);
}

int mpkgconfig::set_sysroot(string newsysroot)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("sys_root").deleteNodeContent(1);
	tmp.addChild("sys_root");
	tmp.getChildNode("sys_root").addText(newsysroot.c_str());
	return setXMLConfig(tmp);
}


int mpkgconfig::set_syscache(string newsyscache)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("sys_cache").deleteNodeContent(1);
	tmp.addChild("sys_cache");
	tmp.getChildNode("sys_cache").addText(newsyscache.c_str());
	return setXMLConfig(tmp);
}

int mpkgconfig::set_dburl(string newdburl)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("database_url").deleteNodeContent(1);
	tmp.addChild("database_url");
	tmp.getChildNode("database_url").addText(newdburl.c_str());
	return setXMLConfig(tmp);
}

int mpkgconfig::set_scriptsdir(string newscriptsdir)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("scripts_dir").deleteNodeContent(1);
	tmp.addChild("scripts_dir");
	tmp.getChildNode("scripts_dir").addText(newscriptsdir.c_str());
	return setXMLConfig(tmp);
}

int mpkgconfig::set_runscripts(bool dorun)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("run_scripts").deleteNodeContent(1);
	tmp.addChild("run_scripts");
	if (dorun) tmp.getChildNode("run_scripts").addText("yes");
	else tmp.getChildNode("run_scripts").addText("no");
	return setXMLConfig(tmp);
}

int mpkgconfig::set_checkFiles(unsigned int value)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("checkFileConflicts").deleteNodeContent(1);
	tmp.addChild("checkFileConflicts");
	if (value == CHECKFILES_PREINSTALL) tmp.getChildNode("checkFileConflicts").addText("preinstall");
	if (value == CHECKFILES_POSTINSTALL) tmp.getChildNode("checkFileConflicts").addText("postinstall");
	if (value == CHECKFILES_DISABLE) tmp.getChildNode("checkFileConflicts").addText("disable");

	return setXMLConfig(tmp);
}




