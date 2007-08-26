/* Debugging output function(s)
$Id: debug.h,v 1.13 2007/08/26 00:20:40 i27249 Exp $
*/


#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <stdlib.h>
#include "config.h"
#include "colors.h"
typedef int DEBUG_LEVEL;
#define TEMP_XML_DOC "/tmp/mpkg-temp-doc.xml"

#define mDebug(m) DbgPrint(__FILE__, __LINE__, __func__, m)
#define mError(m) _mError(__FILE__, __LINE__, __func__, m, false)
#define mWarning(m) _mError(__FILE__, __LINE__, __func__, m, true)
#define say printf
#ifdef DEBUG
#define ASSERT(m) (assert(m))
#else
#define ASSERT(m)
#endif
void _mError(char* file, int line, const char *func, std::string message, bool warn);
void DbgPrint(char* file, int line, const char *func, std::string message);

//void debug(std::string str);

std::string strim(std::string& s, const std::string& drop = "\n\t ");

#endif //DEBUG_H_

