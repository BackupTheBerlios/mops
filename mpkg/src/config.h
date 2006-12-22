/* Temporary config - until a full-functional config will be implemented
    $Id: config.h,v 1.6 2006/12/22 10:33:59 adiakin Exp $
*/



#ifndef CONFIG_H_
#define CONFIG_H_
#define DO_NOT_RUN_SCRIPTS // Prevent executing of scripts - it may be dangerous
//#define ENABLE_DEBUG
#define SYS_ROOT "/root/development/sys_root/"
#define SYS_CACHE "/root/development/sys_cache/"

#include <libintl.h>
#include <locale.h>

#define _(string) gettext(string)

#endif
