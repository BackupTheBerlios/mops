/* Debugging output function(s)
$Id: debug.h,v 1.6 2007/02/22 12:51:19 adiakin Exp $
*/


#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <stdlib.h>
#include "config.h"
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

