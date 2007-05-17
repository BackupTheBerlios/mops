/* Debugging output function(s)
 $Id: debug.cpp,v 1.6 2007/05/17 13:17:35 i27249 Exp $
 */
 


#include "debug.h"

void _mError(char* file, int line, const char *func, std::string message)
{
	fprintf(stderr, "%s[ERROR]: %sin %s  (%s:%i):%s %s\n",CL_RED, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());
}

void DbgPrint(char* file, int line, const char *func, std::string message) {
#ifdef DEBUG
	fprintf(stdout, "%s[DEBUG] %sin %s  (%s:%i):%s %s\n",CL_GREEN, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());

	//fprintf(stdout, "%s[DEBUG]%s %s:%i %s\n",CL_GREEN, CL_WHITE, file, line, message.c_str());
#endif
}

