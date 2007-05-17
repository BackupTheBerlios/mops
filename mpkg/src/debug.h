/* Debugging output function(s)
$Id: debug.h,v 1.7 2007/05/17 13:17:35 i27249 Exp $
*/


#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <stdlib.h>
#include "config.h"
#include "colors.h"
typedef int DEBUG_LEVEL;

#define debug(m) DbgPrint(__FILE__, __LINE__, __func__, m)
#define mError(m) _mError(__FILE__, __LINE__, __func__, m)
#ifdef DEBUG
#define ASSERT(m) (assert(m))
#else
#define ASSERT(m)
#endif
void _mError(char* file, int line, const char *func, std::string message);

void DbgPrint(char* file, int line, const char *func, std::string message);

//void debug(std::string str);

#endif //DEBUG_H_

