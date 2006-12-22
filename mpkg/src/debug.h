/* Debugging output function(s)
$Id: debug.h,v 1.4 2006/12/22 13:14:56 adiakin Exp $
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


void debug(std::string str);
#endif //DEBUG_H_

