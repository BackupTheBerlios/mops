/******************************************************
 * MOPSLinux packaging system - global configuration
 * $Id: cdconfig.cpp,v 1.1 2007/04/13 13:52:27 i27249 Exp $
 *
 * ***************************************************/

#include "cdconfig.h"
#include "xmlParser.h"
#define HTTP_LIB;
//mpkgErrorCode errorCode;
//mpkgErrorReturn errorReturn;
//bool DO_NOT_RUN_SCRIPTS;
//unsigned int fileConflictChecking = CHECKFILES_PREINSTALL;
//string currentStatus;
//string currentItem;
//double currentProgress2;
//double progressMax2;
//bool progressEnabled2 = false;
//double currentProgress;
//double progressMax;
//bool progressEnabled = false;
//string SYS_ROOT;
//string SYS_CACHE;
//string SCRIPTS_DIR;
//unsigned int DATABASE;
//string DB_FILENAME;
//vector<string> REPOSITORY_LIST;
//vector<string> DISABLED_REPOSITORY_LIST;

//#ifndef HTTP_LIB
//string CDROM_DEVICE;// = "/dev/hda";
//string CDROM_MOUNTPOINT;// = "/mnt/cdrom";
//#endif

//#ifdef HTTP_LIB
string DL_CDROM_DEVICE;
string DL_CDROM_MOUNTPOINT;
//#endif
int loadGlobalCdConfig(string config_file)
{
//	currentStatus = "Loading CD-ROM configuration...";
/*	string run_scripts="yes";
	string cdrom_device="/dev/cdrom";
	string cdrom_mountpoint="/mnt/cdrom";
	string check_files = "preinstall";
	string sys_root="/root/development/sys_root/";
	string sys_cache="/root/development/sys_cache/";
	string db_url="sqlite://var/log/mpkg/packages.db";
	string scripts_dir="/var/log/mpkg/scripts/";
	string sql_type;
	vector<string> repository_list;
	vector<string> disabled_repository_list;*/
	bool conf_init=false;
	XMLResults xmlErrCode;
	if (access(config_file.c_str(), R_OK)==0)
	{
		XMLNode config=XMLNode::parseFile(config_file.c_str(), "mpkgconfig", &xmlErrCode);
		
		if (xmlErrCode.error != eXMLErrorNone)
		{
			printf("config parse error!\n");
			/*setErrorCode(MPKG_SUBSYS_XMLCONFIG_READ_ERROR);
			while(getErrorReturn() == MPKG_RETURN_WAIT)
			{
				printf("waiting responce...\n");
				sleep(1);
				if (getErrorReturn() == MPKG_RETURN_REINIT)
				{
					conf_init = true;
					break;
				}
			}*/
			conf_init=true;
		}
		if (!conf_init)
		{
			/*
			if (config.nChildNode("run_scripts")!=0)
				run_scripts=(string) config.getChildNode("run_scripts").getText();
			if (config.nChildNode("checkFileConflicts")!=0)
				check_files = (string) config.getChildNode("checkFileConflicts").getText();
			if (config.nChildNode("sys_root")!=0)
				sys_root=(string) config.getChildNode("sys_root").getText();
			if (config.nChildNode("sys_cache")!=0)
				sys_cache=(string) config.getChildNode("sys_cache").getText();*/
			if (config.nChildNode("cdrom_device")!=0)
				cdrom_device=(string) config.getChildNode("cdrom_device").getText();
			if (config.nChildNode("cdrom_mountpoint")!=0)
				cdrom_mountpoint = (string) config.getChildNode("cdrom_mountpoint").getText();
/*			if (config.nChildNode("database_url")!=0)
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
			}*/
		}

	}
	else
	{
		conf_init=true;
		printf("Configuration file /etc/mpkg.xml not found, using defaults and creating config\n");
	}
	// parsing results
	
	// run_scripts
	/*if (run_scripts=="yes")
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
	*/
	if (cdrom_device.empty())
	{
		printf("empty cd-rom, using default\n");
		DL_CDROM_DEVICE="/dev/cdrom";
	}
	else
	{
		printf("using cd-rom drive %s\n", cdrom_device.c_str());
		DL_CDROM_DEVICE=cdrom_device;
	}

	if (cdrom_mountpoint.empty())
	{
		DL_CDROM_MOUNTPOINT="/mnt/cdrom";
	}

	else
	{
		DL_CDROM_MOUNTPOINT=cdrom_mountpoint;
	}

	//SYS_ROOT=sys_root;
	////sys_cache
	//SYS_CACHE=sys_cache;
	//SCRIPTS_DIR=scripts_dir;	
	/*
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
*/
	//currentStatus = "Settings loaded";
	return 0;
}

XMLNode mpkgconfig::getXMLConfig(string conf_file)
{
	//printf("getXMLConfig\n");
	debug("getXMLConfig");
	XMLNode config;
	XMLResults xmlErrCode;
	bool conf_init = false;
	if (access(conf_file.c_str(), R_OK)==0)
	{
		config=XMLNode::parseFile(conf_file.c_str(), "mpkgconfig", &xmlErrCode);
		if (xmlErrCode.error != eXMLErrorNone)
		{
			printf("config parse error!\n");
			/*setErrorCode(MPKG_SUBSYS_XMLCONFIG_READ_ERROR);
			while(getErrorReturn() == MPKG_RETURN_WAIT)
			{
				printf("waiting responce...\n");
				sleep(1);
				if (getErrorReturn() == MPKG_RETURN_REINIT)
				{
					conf_init = true;
					break;
				}
			}*/
			conf_init=true;
		}

	}
	else conf_init = true;
	if (conf_init) config=XMLNode::createXMLTopNode("mpkgconfig");
	/*
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
*/
	if (config.nChildNode("cdrom_device")==0)
	{
		config.addChild("cdrom_device");
		config.getChildNode("cdrom_device").addText(get_cdromdevice().c_str());
	}


	if (config.nChildNode("cdrom_mountpoint")==0)
	{
		config.addChild("cdrom_mountpoint");
		config.getChildNode("cdrom_mountpoint").addText(get_cdrommountpoint().c_str());
	}
/*
	if (config.nChildNode("scripts_dir")==0)
	{
		config.addChild("scripts_dir");
		config.getChildNode("scripts_dir").addText(get_scriptsdir().c_str());
	}
	debug("getXMLConfig end");*/
	return config;
}

int mpkgconfig::initConfig()
{
	XMLNode tmp=getXMLConfig();
	return setXMLConfig(tmp);
}

int mpkgconfig::setXMLConfig(XMLNode xmlConfig, string conf_file)
{
//	mpkgErrorReturn errRet;

write_config:
	if (xmlConfig.writeToFile(conf_file.c_str())!=eXMLErrorNone) 
	{
		printf("error writing config file");
		errRet = waitResponce(MPKG_SUBSYS_XMLCONFIG_WRITE_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
		{
			abort();
			//goto write_config;
		}
	}
	loadGlobalCdConfig();
	return 0;
}
/*
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
*/
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
/*
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
*/
int mpkgconfig::set_cdromdevice(string cdromDevice)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("cdrom_device").deleteNodeContent(1);
	tmp.addChild("cdrom_device");
	printf("setting cd device to %s\n", cdromDevice.c_str());
	tmp.getChildNode("cdrom_device").addText(cdromDevice.c_str());
}

int mpkgconfig::set_cdrommountpoint(string cdromMountPoint)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("cdrom_mountpoint").deleteNodeContent(1);
	tmp.addChild("cdrom_mountpoint");
	tmp.getChildNode("cdrom_mountpoint").addText(cdromMountPoint.c_str());
}

/*
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
	setErrorCode(errCode);
	while ( getErrorReturn() == MPKG_RETURN_WAIT)
	{
		sleep(1);
	}
	return getErrorReturn();
}
*/
