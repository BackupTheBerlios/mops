/* Debugging output function(s)
$Id: debug.h,v 1.5 2007/01/19 14:32:42 i27249 Exp $
*/


#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <stdlib.h>
/*
#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/basicconfigurator.h>
*/
#include "config.h"
/*
using namespace log4cxx;
using namespace log4cxx::helpers;
*/
typedef int DEBUG_LEVEL;

#define debug(m) DbgPrint(__FILE__, __LINE__, m)

void DbgPrint(char* file, int line, std::string message);

//void debug(std::string str);

#endif //DEBUG_H_

