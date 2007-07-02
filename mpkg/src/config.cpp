/******************************************************
 * MOPSLinux packaging system - global configuration
 * $Id: config.cpp,v 1.35 2007/07/02 14:04:49 i27249 Exp $
 *
 * ***************************************************/

#include "config.h"
#include "xmlParser.h"

mpkgErrorCode errorCode;
mpkgErrorReturn errorReturn;

bool simulate=false;

bool force_dep=false;
bool force_skip_conflictcheck=false;
bool force_essential_remove=false;
bool force_conflicts=false;
bool download_only=false;

bool consoleMode=true;
bool dialogMode=false;
bool DO_NOT_RUN_SCRIPTS;
bool forceSkipLinkMD5Checks=false;
unsigned int fileConflictChecking = CHECKFILES_PREINSTALL;
string SYS_ROOT;
string SYS_CACHE;
string SCRIPTS_DIR;
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

void initDirectoryStructure()
{
	string cmd;
	cmd = "mkdir -p " + SYS_ROOT;
	system(cmd.c_str());
	cmd = "mkdir -p " + SYS_CACHE;
	system(cmd.c_str());
	cmd = "mkdir -p " + SCRIPTS_DIR;
	system(cmd.c_str());
}

int loadGlobalConfig(string config_file)
{
#ifdef HTTP_LIB
	mError("error: core running non-core code\n");
#endif
	currentStatus = "Loading configuration...";
	string run_scripts="yes";
	string cdrom_device="/dev/cdrom";
	string cdrom_mountpoint="/mnt/cdrom";
	string check_files = "preinstall";
	string sys_root="/root/development/sys_root/";
	string sys_cache="/root/development/sys_cache/";
	string db_url="sqlite://var/log/mpkg/packages.db";
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
				for (int i=0;i<config.getChildNode("repository_list").nChildNode("repository");i++)
				{
					repository_list.push_back((string) config.getChildNode("repository_list").getChildNode("repository",i).getText());
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
	// sys_root
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
	setErrorCode(errCode);
	if (consoleMode)
	{
		consoleEventResolver();
	}
	else
	{
		while ( getErrorReturn() == MPKG_RETURN_WAIT)
		{
			sleep(1);
		}
	}
	return getErrorReturn();
}

int consoleSendErrorMessage(string header, string text, string actionList, string defaultAction)
{
	say("[%s]\n%s\n",header.c_str(), text.c_str());
	mDebug("actionList: " + actionList);
	say("Action:\n");
	vector<string> actList;
	while(true)//actionList.find_first_of(" ")!=string::npos)
	{
		actList.push_back(actionList.substr(0,actionList.find_first_of(" ")));
		if (actionList.find_first_of(" ")==string::npos) break;
		actionList=actionList.substr(actionList.find_first_of(" ")+1);
	}
	unsigned int def_action_num=1;
	for (unsigned int i=0; i<actList.size(); i++)
	{
		if (actList[i]==defaultAction)
		{
			def_action_num = i+1;
			say("[%d] %s (default)\n", i+1, actList[i].c_str());

		}
		else say("[%d] %s\n", i+1, actList[i].c_str());
	}
	//char ret[200];
	unsigned int r=0;
	scanf("%d", &r);
       	//scanf("%s", &ret);
	//string t=ret;
	int ret_num = r;//atoi(t.c_str());
	if (r==0 || r>actList.size()) ret_num=def_action_num;
	mDebug("return value = [" + IntToStr(ret_num) + "]");
	return ret_num;
}

void consoleEventResolver()
{
	int userReply;
	switch(getErrorCode())
	{
		//-------- PACKAGE DOWNLOAD ERRORS ---------//
		case MPKG_DOWNLOAD_OK:
			setErrorReturn(MPKG_RETURN_CONTINUE);
			break;
		case MPKG_DOWNLOAD_TIMEOUT:
		case MPKG_DOWNLOAD_MD5:
		case MPKG_DOWNLOAD_HOST_NOT_FOUND:
		case MPKG_DOWNLOAD_FILE_NOT_FOUND:
		case MPKG_DOWNLOAD_LOGIN_INCORRECT:
		case MPKG_DOWNLOAD_FORBIDDEN:
		case MPKG_DOWNLOAD_OUT_OF_SPACE:
		case MPKG_DOWNLOAD_WRITE_ERROR:

		case MPKG_DOWNLOAD_ERROR:
			userReply=consoleSendErrorMessage("Download error","Some files failed to download. What to do?",
								"Retry Abort Ignore", 
								"Retry");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							case 3:
								setErrorReturn(MPKG_RETURN_IGNORE);
								break;
							default:
								mError("Unknown reply");
						}
						break;
						// BREAKAGE DUE TO DEBUG
	


					//---------------CD-ROM ERRORS/EVENTS---------------------//
					case MPKG_CDROM_WRONG_VOLNAME:
					case MPKG_CDROM_MOUNT_ERROR:
						//txt = "Please insert disk with label "+CDROM_VOLUMELABEL+" into "+CDROM_DEVICENAME;
						userReply = consoleSendErrorMessage("Please insert disk", \
								"Please insert disk with label "+CDROM_VOLUMELABEL+" into "+CDROM_DEVICENAME, "Ok Abort", "Ok");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							default:
								setErrorReturn(MPKG_RETURN_ABORT);
						}
						break;


					//-------- INDEX ERRORS ---------------------------//
					
					case MPKG_INDEX_DOWNLOAD_TIMEOUT:
						userReply =consoleSendErrorMessage("Download error",\
								"Unable to download repository index", \
								"Retry Abort Ignore", \
								"Retry");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							case 3:
								setErrorReturn(MPKG_RETURN_IGNORE);
								break;
							default:
								mError("Unknown reply");
						}
						break;

					case MPKG_INDEX_PARSE_ERROR:
						userReply = consoleSendErrorMessage("Parse error","Error parsing repository index!",
								"Skip", 
								"Skip");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_SKIP);
								break;
							default:
								mError("Unknown reply");
						}
						break;

					case MPKG_INDEX_HOST_NOT_FOUND:
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INDEX_NOT_RECOGNIZED:
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INDEX_LOGIN_INCORRECT:
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INDEX_FORBIDDEN:
						setErrorReturn(MPKG_RETURN_SKIP);
					case MPKG_INDEX_ERROR:
						userReply = consoleSendErrorMessage("Repository index error",\
								"Error retrieving repository index!","Ok","Ok");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_SKIP);
								break;
							default:
								mError("Unknown reply");
						}
						break;



					//---------- INSTALLATION ERRORS --------------//
					case MPKG_INSTALL_OUT_OF_SPACE:
						userReply=consoleSendErrorMessage("Out of space!",
								"Error installing packages - out of space.\nFree some disk space and try again", \
								"Retry Abort", "Retry");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
							default:
								mError("Unknown reply");
						}
						break;

					case MPKG_INSTALL_SCRIPT_ERROR:
						userReply = consoleSendErrorMessage("Script error", "Error executing script",\
							       	"Ok", "Ok");
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
					case MPKG_INSTALL_EXTRACT_ERROR:
						userReply=consoleSendErrorMessage("Package extraction error",\
							       	"Error extracting package.",\
							       "Ok", "Ok");
						setErrorReturn(MPKG_RETURN_SKIP);
						break;
	

// NEXT - TODO (convert to console methods)
					case MPKG_INSTALL_META_ERROR:
						userReply = consoleSendErrorMessage("Error extracting metadata", "Error while extracting metadata from package. Seems that package is broken", "Ok", "Ok");
						setErrorReturn(MPKG_RETURN_SKIP);
						break;

					case MPKG_INSTALL_FILE_CONFLICT:
						userReply = consoleSendErrorMessage("File conflict detected",\
							       	"File conflict detected. You can force installation, but it is DANGEROUS (it may broke some components)", "Ignore Abort", "Ignore");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_IGNORE);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						break;


					//---------STARTUP ERRORS---------------//
					case MPKG_STARTUP_COMPONENT_NOT_FOUND:
						userReply =consoleSendErrorMessage("Some components not found!",\
							       "Some components were not found, the program can fail during runtime. Continue?", "Yes No", "No");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_CONTINUE);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
						}
						break;
					case MPKG_STARTUP_NOT_ROOT:
						userReply =consoleSendErrorMessage("UID != 0", \
								"You should run this program as root!",\
							       	"Abort", "Abort");

						setErrorReturn(MPKG_RETURN_ABORT);
						break;

					
					//---------- SUBSYSTEM ERRORS ---------------------//
					case MPKG_SUBSYS_SQLDB_INCORRECT:
						userReply = consoleSendErrorMessage("SQL database error",\
								"Incorrect database structure. Reinitialize?",
								"Yes No","No");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_REINIT);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply");
						}
						break;

					case MPKG_SUBSYS_SQLDB_OPEN_ERROR:
						userReply = consoleSendErrorMessage("SQL database error",\
								"Unable to open database. This is critical, cannot continue",
								"Ok",
								"Ok");
						setErrorReturn(MPKG_RETURN_ABORT);
						break;

					case MPKG_SUBSYS_XMLCONFIG_READ_ERROR:
						userReply = consoleSendErrorMessage("Error in configuration files",
							       	"Error in configuration files. Try to recreate? WARNING: all your settings will be lost!|",
								"Yes Abort", 
								"Abort");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_REINIT);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply\n");
						}
						break;
					case MPKG_SUBSYS_XMLCONFIG_WRITE_ERROR:
						userReply = consoleSendErrorMessage("Error writing configuration files",
							       	"Error writing configuration files. Retry?",
								"Retry Abort", 
								"Abort");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply\n");
						}
						break;

					case MPKG_SUBSYS_SQLQUERY_ERROR:
						userReply = consoleSendErrorMessage("Internal error",
							       	"SQL query error detected. This is critical internal error, we exit now.",
								"Abort", "Abort");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply\n");
						}
						break;

					case MPKG_SUBSYS_TMPFILE_CREATE_ERROR:
						userReply = consoleSendErrorMessage("Error creating temp file",
							       	"Error while creating a temp file. In most cases this mean that no free file descriptors available. This is critical, cannot continue",
								"Abort", 
								"Abort");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply\n");
						}
						break;

					case MPKG_SUBSYS_FILE_WRITE_ERROR:
						userReply =consoleSendErrorMessage("Error writing file",
							       	"File write error! Check for free space. Retry?",
								"Yes Abort", 
								"Abort");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case 2:
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply\n");
						}
						break;

					case MPKG_SUBSYS_FILE_READ_ERROR:
						userReply =consoleSendErrorMessage("Error reading file",
							       	"File read error! Retry?",
								"Yes Abort", 
								"Abort");
						switch(userReply)
						{
							case 1:
								setErrorReturn(MPKG_RETURN_RETRY);
								break;
							case 2:
							
								setErrorReturn(MPKG_RETURN_ABORT);
								break;
							default:
								mError("Unknown reply\n");
						}
						break;
					default:
						userReply = consoleSendErrorMessage("Unknown error!!!", "Unknown error occured!!",\
							       	"Ignore", "Ignore");
						setErrorReturn(MPKG_RETURN_IGNORE);
						break;
				}
}
