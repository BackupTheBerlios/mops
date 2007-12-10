/******************************************************
 * MOPSLinux packaging system - global configuration
 * $Id: config.cpp,v 1.54 2007/12/10 03:12:58 i27249 Exp $
 *
 * ***************************************************/

#include "config.h"
#include "xmlParser.h"
#include "file_routines.h"
bool verbose=false;
bool useBuildCache=true;
bool enableDownloadResume=true;
int autogenDepsMode=ADMODE_MOZGMERTV;
mpkgErrorCode errorCode;
mpkgErrorReturn errorReturn;
bool setupMode=false;
bool interactive_mode=false;
bool require_root = true;
bool simulate=false;
vector<string> removeBlacklist;
bool ignoreDeps=false;
bool force_dep=false;
bool force_skip_conflictcheck=false;
bool force_essential_remove=false;
bool force_conflicts=false;
bool download_only=false;
string log_directory = "/var/log/";
bool consoleMode=true;
bool dialogMode=false;
bool DO_NOT_RUN_SCRIPTS;
bool forceSkipLinkMD5Checks=false;
unsigned int fileConflictChecking = CHECKFILES_PREINSTALL;
bool dont_export=false;
string legacyPkgDir = "/var/log/packages/";
string SYS_ROOT;
string SYS_CACHE;
string SCRIPTS_DIR;
string SYS_BACKUP; // TODO
unsigned int DATABASE;
string DB_FILENAME;
vector<string> REPOSITORY_LIST;
vector<string> DISABLED_REPOSITORY_LIST;
#ifndef HTTP_LIB
string CDROM_DEVICE;// = "/dev/hda";
string CDROM_MOUNTPOINT;// = "/mnt/cdrom";
#endif
string CDROM_VOLUMELABEL;
string CDROM_DEVICENAME;

#ifdef HTTP_LIB
string DL_CDROM_DEVICE;
string DL_CDROM_MOUNTPOINT;
#endif
Config mConfig;
string SYS_MPKG_VAR_DIRECTORY="/var/mpkg/";
void initDirectoryStructure()
{
	string cmd;
	cmd = "mkdir -p " + SYS_ROOT;
	system(cmd.c_str());
	cmd = "mkdir -p " + SYS_CACHE;
	system(cmd.c_str());
	cmd = "mkdir -p " + SCRIPTS_DIR;
	system(cmd.c_str());
	//system("mkdir -p " + SYS_BACKUP);
}

int loadGlobalConfig(string config_file)
{
#ifdef HTTP_LIB
	mError("error: core running non-core code\n");
#endif

	currentStatus = "Loading configuration...";
	removeBlacklist = ReadFileStrings("/etc/mpkg-remove-blacklist");
	string run_scripts="yes";
	string cdrom_device="/dev/cdrom";
	string cdrom_mountpoint="/mnt/cdrom";
	string check_files = "preinstall";
	string sys_root="/";
	string sys_cache="/var/mpkg/cache/";
	string db_url="sqlite:///var/log/mpkg/packages.db";
	string scripts_dir="/var/log/mpkg/scripts/";
	string sql_type;
	vector<string> repository_list;
	vector<string> disabled_repository_list;
	bool conf_init=false;
	XMLResults xmlErrCode;
	if (access(config_file.c_str(), R_OK)==0)
	{
		XMLNode config=XMLNode::parseFile(config_file.c_str(), "mpkgconfig", &xmlErrCode);
		
		if (xmlErrCode.error != eXMLErrorNone)
		{
			mError("config parse error!\n");
			mpkgErrorReturn errRet = waitResponce(MPKG_SUBSYS_XMLCONFIG_READ_ERROR);
			if (errRet == MPKG_RETURN_REINIT)
			{
				conf_init = true;
			}
		}
		
		if (!conf_init)
		{
			if (config.nChildNode("run_scripts")!=0)
				run_scripts=(string) config.getChildNode("run_scripts").getText();
			if (config.nChildNode("checkFileConflicts")!=0)
				check_files = (string) config.getChildNode("checkFileConflicts").getText();
			if (config.nChildNode("sys_root")!=0)
				sys_root=(string) config.getChildNode("sys_root").getText();
			if (config.nChildNode("sys_cache")!=0)
				sys_cache=(string) config.getChildNode("sys_cache").getText();
			if (config.nChildNode("cdrom_device")!=0)
				cdrom_device=(string) config.getChildNode("cdrom_device").getText();
			if (config.nChildNode("cdrom_mountpoint")!=0)
				cdrom_mountpoint = (string) config.getChildNode("cdrom_mountpoint").getText();
			if (config.nChildNode("database_url")!=0)
				db_url=(string) config.getChildNode("database_url").getText();
			if (config.nChildNode("repository_list")!=0)
			{
				string tmp_r;
				for (int i=0;i<config.getChildNode("repository_list").nChildNode("repository");i++)
				{
					tmp_r = (string) config.getChildNode("repository_list").getChildNode("repository",i).getText();
					if (tmp_r[tmp_r.length()-1]!='/') tmp_r += "/";
					repository_list.push_back(tmp_r);
				}
				for (int i=0; i<config.getChildNode("repository_list").nChildNode("disabled_repository"); i++)
				{
					disabled_repository_list.push_back((string) config.getChildNode("repository_list").getChildNode("disabled_repository",i).getText());
				}
			}
			if (config.nChildNode("scripts_dir")!=0)
			{
				scripts_dir = (string) config.getChildNode("scripts_dir").getText();
			}
		}

	}
	else
	{
		conf_init=true;
		mError("Configuration file /etc/mpkg.xml not found, using defaults and creating config\n");
	}
	// parsing results
	
	// run_scripts
	if (run_scripts=="yes")
		DO_NOT_RUN_SCRIPTS=false;
	if (run_scripts=="no")
		DO_NOT_RUN_SCRIPTS=true;
	if (run_scripts!="yes" && run_scripts!="no")
	{
		mError("Error in config file, option <run_scripts> should be \"yes\" or \"no\", without quotes");
		return -1; // CONFIG ERROR in scripts
	}
	if (check_files == "preinstall")
		fileConflictChecking = CHECKFILES_PREINSTALL;
	if (check_files == "postinstall")
		fileConflictChecking = CHECKFILES_POSTINSTALL;
	if (check_files == "disable")
		fileConflictChecking = CHECKFILES_DISABLE;
	if (cdrom_device.empty())
	{
		mError("empty cd-rom, using default\n");
#ifndef HTTP_LIB
		CDROM_DEVICE="/dev/cdrom";
#else
		DL_CDROM_DEVICE="/dev/cdrom";
#endif
	}
	else
	{
#ifndef HTTP_LIB
		CDROM_DEVICE=cdrom_device;
#else
		DL_CDROM_DEVICE=cdrom_device;
#endif
	}

	if (cdrom_mountpoint.empty())
	{
#ifndef HTTP_LIB
		CDROM_MOUNTPOINT="/mnt/cdrom";
#else
		DL_CDROM_MOUNTPOINT="/mnt/cdrom";
#endif
	}

	else
	{
#ifndef HTTP_LIB
		CDROM_MOUNTPOINT=cdrom_mountpoint;
#else
		DL_CDROM_MOUNTPOINT=cdrom_mountpoint;
#endif
	}

	// sys_root
	SYS_ROOT=sys_root;
	//sys_cache
	SYS_CACHE=sys_cache;
	SCRIPTS_DIR=scripts_dir;
	SYS_BACKUP = mConfig.getValue("sys_backup");
	if (SYS_BACKUP.empty()) { 
		SYS_BACKUP = "/var/mpkg/backup/";
		mConfig.setValue("sys_backup", SYS_BACKUP);
	}
	if (db_url.find("sqlite://")==0)
	{
		sql_type="sqlite://";

		DATABASE=DB_SQLITE_LOCAL;
		DB_FILENAME=db_url.substr(strlen("sqlite:/"));
	}
	else {
		mError("CRITICAL: cannot find database, check config!");
		abort();
	}
	REPOSITORY_LIST=repository_list;
	DISABLED_REPOSITORY_LIST = disabled_repository_list;
#ifdef DEBUG
	say("System configuration:\n\trun_scripts: %s\n\tsys_root: %s\n\tsys_cache: %s\n\tSQL type: %s\n\tSQL filename: %s\n", \
			run_scripts.c_str(), sys_root.c_str(), sys_cache.c_str(), sql_type.c_str(), db_url.c_str());
#endif
	if (conf_init) mpkgconfig::initConfig();

	currentStatus = "Settings loaded";
	initDirectoryStructure();
	return 0;
}

XMLNode mpkgconfig::getXMLConfig(string conf_file)
{
	mDebug("getXMLConfig");
	XMLNode config;
	XMLResults xmlErrCode;
	bool conf_init = false;
	removeBlacklist = ReadFileStrings("/etc/mpkg-remove-blacklist");
	if (access(conf_file.c_str(), R_OK)==0)
	{
		config=XMLNode::parseFile(conf_file.c_str(), "mpkgconfig", &xmlErrCode);
		if (xmlErrCode.error != eXMLErrorNone)
		{
			mError("config parse error!\n");
			mpkgErrorReturn errRet = waitResponce(MPKG_SUBSYS_XMLCONFIG_READ_ERROR);
			if (errRet == MPKG_RETURN_REINIT)
			{
				conf_init = true;
			}
		}

	}
	else conf_init = true;
	if (conf_init) config=XMLNode::createXMLTopNode("mpkgconfig");

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

	if (config.nChildNode("cdrom_device")==0)
	{
		mError("CD not detected\n");
		config.addChild("cdrom_device");
		config.getChildNode("cdrom_device").addText(get_cdromdevice().c_str());
	}


	if (config.nChildNode("cdrom_mountpoint")==0)
	{
		config.addChild("cdrom_mountpoint");
		config.getChildNode("cdrom_mountpoint").addText(get_cdrommountpoint().c_str());
	}

	if (config.nChildNode("scripts_dir")==0)
	{
		config.addChild("scripts_dir");
		config.getChildNode("scripts_dir").addText(get_scriptsdir().c_str());
	}
	mDebug("getXMLConfig end");
	return config;
}

int mpkgconfig::initConfig()
{
	XMLNode tmp=getXMLConfig();
	return setXMLConfig(tmp);
}

int mpkgconfig::setXMLConfig(XMLNode xmlConfig, string conf_file)
{
	mpkgErrorReturn errRet;

write_config:
	if (xmlConfig.writeToFile(conf_file.c_str())!=eXMLErrorNone) 
	{
		mError("error writing config file");
		errRet = waitResponce(MPKG_SUBSYS_XMLCONFIG_WRITE_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
		{
			goto write_config;
		}
	}
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
	mDebug("filename = " + DB_FILENAME);
	return "sqlite:/"+DB_FILENAME;
}

string mpkgconfig::get_scriptsdir()
{
	return SCRIPTS_DIR;
}

bool mpkgconfig::get_runscripts()
{
	return !DO_NOT_RUN_SCRIPTS;
}

string mpkgconfig::get_cdromdevice()
{
#ifndef HTTP_LIB
	return CDROM_DEVICE;
#else
	return DL_CDROM_DEVICE;
#endif
}

string mpkgconfig::get_cdrommountpoint()
{
#ifndef HTTP_LIB
	return CDROM_MOUNTPOINT;
#else
	return DL_CDROM_MOUNTPOINT;
#endif
}

unsigned int mpkgconfig::get_checkFiles()
{
	return fileConflictChecking;
}
int mpkgconfig::set_repositorylist(vector<string> newrepositorylist, vector<string> drList)
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
	for (unsigned int i=0; i<drList.size(); i++)
	{
		tmp.getChildNode("repository_list").addChild("disabled_repository");
		tmp.getChildNode("repository_list").getChildNode("disabled_repository", i).addText(drList[i].c_str());
	}
	return setXMLConfig(tmp);
}

int mpkgconfig::set_disabled_repositorylist(vector <string> newrepositorylist)
{
	mError("WARNING! You should NOT use set_disabled_repositorylist!");
	return -1;
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

int mpkgconfig::set_cdromdevice(string cdromDevice)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("cdrom_device").deleteNodeContent(1);
	tmp.addChild("cdrom_device");
	mDebug("setting cd device to " + cdromDevice);
	tmp.getChildNode("cdrom_device").addText(cdromDevice.c_str());
	return setXMLConfig(tmp);
}

int mpkgconfig::set_cdrommountpoint(string cdromMountPoint)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("cdrom_mountpoint").deleteNodeContent(1);
	tmp.addChild("cdrom_mountpoint");
	tmp.getChildNode("cdrom_mountpoint").addText(cdromMountPoint.c_str());
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



void setErrorCode(mpkgErrorCode value)
{
	if (value != MPKG_OK) setErrorReturn(MPKG_RETURN_WAIT);
	errorCode = value;
}
void setErrorReturn(mpkgErrorReturn value)
{
	setErrorCode(MPKG_OK);
	errorReturn = value;
}

mpkgErrorCode getErrorCode()
{
	return errorCode;
}
mpkgErrorReturn getErrorReturn()
{
	return errorReturn;
}


mpkgErrorReturn waitResponce(mpkgErrorCode errCode)
{
	return callError(errCode);
}


Config::Config(string _configName)
{
	configName = _configName;
}

Config::~Config()
{
}

string Config::getValue(string attribute, int value_id)
{
	if (!readXml()) {
		return "";
	}
	if (node.nChildNode(attribute.c_str())<=value_id) return "";
	return (string) node.getChildNode(attribute.c_str(),value_id).getText();
}

bool Config::setValue(string attribute, string value, int value_id)
{
	if (!readXml()) return false;
	while (node.nChildNode(attribute.c_str())<=value_id) {
		node.addChild(attribute.c_str());
	}
	node.getChildNode(attribute.c_str(), value_id).deleteNodeContent();
	node.addChild(attribute.c_str());
	node.getChildNode(attribute.c_str(), value_id).addText(value.c_str());
	return writeXml();
}

bool Config::readXml()
{
	XMLResults xmlErrCode;
	if (access(configName.c_str(), R_OK)!=0) {
		mError("Config file doesn't exist");
		return false;
	}
	node=XMLNode::parseFile(configName.c_str(), "mpkgconfig", &xmlErrCode);
	if (xmlErrCode.error != eXMLErrorNone) {
		mError("Configuration parse error\n");
		return false;
	}
	return true;
}

bool Config::writeXml()
{
	if (node.writeToFile(configName.c_str())!=eXMLErrorNone) {
		mError("Error writing configuration file");
		return false;
	}
	return true;
}
