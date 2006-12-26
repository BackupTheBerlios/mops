/* Temporary config - until a full-functional config will be implemented
    $Id: config.h,v 1.7 2006/12/26 18:57:11 i27249 Exp $
*/



#ifndef CONFIG_H_
#define CONFIG_H_

// Database type definitions
#define DB_SQLITE_LOCAL 0x01
#define DB_REMOTE 0x02

// Debug-time configuration
#define DO_NOT_RUN_SCRIPTS // Prevent executing of scripts - it may be dangerous
//#define ENABLE_DEBUG

// System configuration
#define SYS_ROOT "/root/development/sys_root/"
#define SYS_CACHE "/root/development/sys_cache/"
#define DATABASE DB_SQLITE_LOCAL
#define SQLITE_DB_FILENAME "/var/log/packages.db"

#include <libintl.h>
#include <locale.h>


#define _(string) gettext(string)

#endif
