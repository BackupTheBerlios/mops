/* Temporary config - until a full-functional config will be implemented
    $Id: config.h,v 1.18 2007/03/21 15:30:14 i27249 Exp $
*/


#ifndef CONFIG_H_
#define CONFIG_H_

//#include "cdrom.h"
#include <string>
#include <vector>
using namespace std;



// Database type definitions
#define DB_SQLITE_LOCAL 0x01
#define DB_REMOTE 0x02


// Debug-time configuration
extern bool DO_NOT_RUN_SCRIPTS;	// Prevent executing of scripts - it may be dangerous

extern string currentStatus;
extern int currentProgress;

// System configuration

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

#define MOUNTMODEL_AUTOVOLUME 0x01
#define MOUNTMODEL_AUTOFIXED 0x02
#define MOUNTMODEL_MANUAL 0x03

extern unsigned int CDROM_MOUNTMODEL;
extern string CDROM_VOLUMEMOUNTDIR;
//extern vector<CDROM_DEVICE> CDROMS;
#ifndef CONFIG_FILE
#define CONFIG_FILE "/etc/mpkg.xml"
#endif
int loadGlobalConfig(string config_file=CONFIG_FILE);


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
	bool get_runscripts();

	int set_repositorylist(vector<string> newrepositorylist);
	int set_disabled_repositorylist(vector<string> newrepositorylist);
	int set_sysroot(string newsysroot);
	int set_syscache(string newsyscache);
	int set_dburl(string newdburl);
	int set_scriptsdir(string newscriptsdir);
	int set_runscripts(bool dorun);
}


#define _(string) gettext(string)

#endif
