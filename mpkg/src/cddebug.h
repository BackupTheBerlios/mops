/* Debugging output function(s)
$Id: cddebug.h,v 1.1 2007/04/13 13:52:27 i27249 Exp $
*/


#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <stdlib.h>
#include "cdconfig.h"
typedef int DEBUG_LEVEL;

#define debug(m) DbgPrint(__FILE__, __LINE__, m)

#ifdef DEBUG
#define ASSERT(m) (assert(m))
#else
#define ASSERT(m)
#endif

void DbgPrint(char* file, int line, std::string message);

//void debug(std::string str);

#endif //DEBUG_H_

