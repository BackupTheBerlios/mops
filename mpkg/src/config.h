/* Temporary config - until a full-functional config will be implemented
    $Id: config.h,v 1.8 2006/12/29 12:57:00 i27249 Exp $
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
extern unsigned int DATABASE;
extern string DB_FILENAME;
extern vector<string> REPOSITORY_LIST;
#include <libintl.h>
#include <locale.h>

int loadGlobalConfig(string config_file="/etc/mpkg.xml");
#define _(string) gettext(string)

#endif
