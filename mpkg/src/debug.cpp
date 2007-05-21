/* Debugging output function(s)
 $Id: debug.cpp,v 1.8 2007/05/21 14:07:18 i27249 Exp $
 */
 


#include "debug.h"

void _mError(char* file, int line, const char *func, std::string message)
{
#ifndef DEBUG
	fprintf(stderr, "%s[ERROR]: %sin %s  (%s:%i):%s %s\n",CL_RED, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());
#else
	fprintf(stderr, "%sError:%s %s\n",CL_RED, CL_WHITE, message.c_str());
#endif

}

void DbgPrint(char* file, int line, const char *func, std::string message) {
#ifdef DEBUG
	fprintf(stdout, "%s[DEBUG] %sin %s  (%s:%i):%s %s\n",CL_GREEN, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());
#endif
}

