/* Temporary config - until a full-functional config will be implemented
    $Id: config.h,v 1.13 2007/02/21 11:51:10 i27249 Exp $
*/


#ifndef CONFIG_H_
#define CONFIG_H_
#include <string>
#include <vector>
using namespace std;



// Database type definitions
#define DB_SQLITE_LOCAL 0x01
#define DB_REMOTE 0x02


// Debug-time configuration
extern bool DO_NOT_RUN_SCRIPTS; // Prevent executing of scripts - it may be dangerous
//#define ENABLE_DEBUG

// System configuration
extern string SYS_ROOT;// "/root/development/sys_root/"
extern string SYS_CACHE; //"/root/development/sys_cache/"
extern string SCRIPTS_DIR;
extern unsigned int DATABASE;
extern string DB_FILENAME;
extern vector<string> REPOSITORY_LIST;
#include <libintl.h>
#include <locale.h>
#include "xmlParser.h"
#include "debug.h"
#define CONFIG_FILE "/etc/mpkg.xml"
int loadGlobalConfig(string config_file=CONFIG_FILE);

namespace mpkgconfig
{
	int initConfig();
	XMLNode getXMLConfig(string conf_file=CONFIG_FILE);
	int setXMLConfig(XMLNode xmlConfig, string conf_file=CONFIG_FILE);

	vector<string> get_repositorylist();
	string get_sysroot();
	string get_syscache();
	string get_dburl();
	string get_scriptsdir();
	bool get_runscripts();

	int set_repositorylist(vector<string> newrepositorylist);
	int set_sysroot(string newsysroot);
	int set_syscache(string newsyscache);
	int set_dburl(string newdburl);
	int set_scriptsdir(string newscriptsdir);
	int set_runscripts(bool dorun);
}


#define _(string) gettext(string)

#endif
