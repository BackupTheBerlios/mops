/* Temporary config - until a full-functional config will be implemented
    $Id: config.h,v 1.29 2007/04/20 04:01:42 i27249 Exp $
*/


#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <vector>
using namespace std;
#define CHECKFILES_PREINSTALL 1
#define CHECKFILES_POSTINSTALL 2
#define CHECKFILES_DISABLE 0

// Database type definitions
#define DB_SQLITE_LOCAL 0x01
#define DB_REMOTE 0x02
// Global error code definition

typedef enum {
    MPKG_OK = 0,
	MPKG_DOWNLOAD_OK,
	MPKG_DOWNLOAD_TIMEOUT,
	MPKG_DOWNLOAD_MD5,
	MPKG_DOWNLOAD_HOST_NOT_FOUND,
	MPKG_DOWNLOAD_FILE_NOT_FOUND,
	MPKG_DOWNLOAD_LOGIN_INCORRECT,
	MPKG_DOWNLOAD_FORBIDDEN,
	MPKG_DOWNLOAD_OUT_OF_SPACE,
	MPKG_DOWNLOAD_WRITE_ERROR,
	MPKG_DOWNLOAD_ERROR,
	
	MPKG_INDEX_OK,
	MPKG_INDEX_DOWNLOAD_TIMEOUT,
	MPKG_INDEX_PARSE_ERROR,
	MPKG_INDEX_HOST_NOT_FOUND,
	MPKG_INDEX_NOT_RECOGNIZED,
	MPKG_INDEX_LOGIN_INCORRECT,
	MPKG_INDEX_FORBIDDEN,
	MPKG_INDEX_ERROR,
	
	MPKG_INSTALL_OK,
	MPKG_INSTALL_OUT_OF_SPACE,
	MPKG_INSTALL_SCRIPT_ERROR,
	MPKG_INSTALL_EXTRACT_ERROR,
	MPKG_INSTALL_META_ERROR,
	MPKG_INSTALL_FILE_CONFLICT,
	MPKG_INSTALL_NOT_IN_DB,

	MPKG_CDROM_MOUNT_ERROR,
	MPKG_CDROM_WRONG_VOLNAME,
	
	MPKG_SUBSYS_SQLDB_INCORRECT,
	MPKG_SUBSYS_SQLDB_OPEN_ERROR,
	MPKG_SUBSYS_XMLCONFIG_READ_ERROR,
	MPKG_SUBSYS_XMLCONFIG_WRITE_ERROR,
	MPKG_SUBSYS_SQLQUERY_ERROR,
	MPKG_SUBSYS_TMPFILE_CREATE_ERROR,
	MPKG_SUBSYS_FILE_WRITE_ERROR,
	MPKG_SUBSYS_FILE_READ_ERROR,
	
	MPKG_STARTUP_COMPONENT_NOT_FOUND,
	MPKG_STARTUP_NOT_ROOT
} mpkgErrorCode;

typedef enum {
	MPKG_RETURN_WAIT = 0,
	MPKG_RETURN_ABORT,
	MPKG_RETURN_SKIP,
	MPKG_RETURN_IGNORE,
	MPKG_RETURN_CONTINUE,
	MPKG_RETURN_ACCEPT,
	MPKG_RETURN_DECLINE,
	MPKG_RETURN_RETRY,
	MPKG_RETURN_REINIT,
} mpkgErrorReturn;

#ifndef HTTP_LIB
// Global configuration and message bus
extern bool DO_NOT_RUN_SCRIPTS;	// Prevent executing of scripts - it may be dangerous
extern unsigned int fileConflictChecking;
extern string currentStatus;
extern string currentItem;
extern double currentProgress;
extern double progressMax;
extern bool progressEnabled;
extern double currentProgress2;
extern double progressMax2;
extern bool progressEnabled2;
extern mpkgErrorCode errorCode;
extern mpkgErrorReturn errorReturn;
extern string CDROM_DEVICE;
extern string CDROM_MOUNTPOINT;
extern string CDROM_VOLUMELABEL;
extern string CDROM_DEVICENAME;
void setErrorCode(mpkgErrorCode value);
void setErrorReturn(mpkgErrorReturn value);

mpkgErrorCode getErrorCode();
mpkgErrorReturn getErrorReturn();

mpkgErrorReturn waitResponce(mpkgErrorCode);
// System configuration
extern bool consoleMode;
extern string SYS_ROOT;		// "/root/development/sys_root/"
extern string SYS_CACHE; 	//"/root/development/sys_cache/"
extern string SCRIPTS_DIR;
extern unsigned int DATABASE;
extern string DB_FILENAME;
extern vector<string> REPOSITORY_LIST;
extern vector<string> DISABLED_REPOSITORY_LIST;
#include <libintl.h>
#include <locale.h>
#include "xmlParser.h"
#include "debug.h"
#endif //HTTP_LIB
#ifdef HTTP_LIB
extern string DL_CDROM_DEVICE;
extern string DL_CDROM_MOUNTPOINT;
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE "/etc/mpkg.xml"
#endif
int loadGlobalConfig(string config_file=CONFIG_FILE);
void consoleEventResolver();

namespace mpkgconfig
{
	int initConfig();
	XMLNode getXMLConfig(string conf_file=CONFIG_FILE);
	int setXMLConfig(XMLNode xmlConfig, string conf_file=CONFIG_FILE);

	vector<string> get_repositorylist();
	vector<string> get_disabled_repositorylist();
	string get_sysroot();
	string get_syscache();
	string get_dburl();
	string get_scriptsdir();
	string get_cdromdevice();
	string get_cdrommountpoint();
	bool get_runscripts();
	unsigned int get_checkFiles();

	int set_repositorylist(vector<string> newrepositorylist, vector<string> drList);
	int set_disabled_repositorylist(vector<string> newrepositorylist);
	int set_sysroot(string newsysroot);
	int set_syscache(string newsyscache);
	int set_dburl(string newdburl);
	int set_scriptsdir(string newscriptsdir);
	int set_cdromdevice(string cdromDevice);
	int set_cdrommountpoint(string cdromMountPoint);
	int set_runscripts(bool dorun);
	int set_checkFiles(unsigned int value);
}


#define _(string) gettext(string)

#endif
