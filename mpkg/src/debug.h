/* Debugging output function(s)
$Id: debug.h,v 1.9 2007/07/20 12:38:39 adiakin Exp $
*/


#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <stdlib.h>
#include "config.h"
#include "colors.h"
typedef int DEBUG_LEVEL;

#define mDebug(m) DbgPrint(__FILE__, __LINE__, __func__, m)
#define mError(m) _mError(__FILE__, __LINE__, __func__, m)
#define say printf
#ifdef DEBUG
#define ASSERT(m) (assert(m))
#else
#define ASSERT(m)
#endif
void _mError(char* file, int line, const char *func, std::string message);
void DbgPrint(char* file, int line, const char *func, std::string message);

//void debug(std::string str);

std::string strim(std::string& s, const std::string& drop = "\n\t ");

#endif //DEBUG_H_

