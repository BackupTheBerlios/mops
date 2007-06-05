/* Debugging output function(s)
 $Id: debug.cpp,v 1.11 2007/06/05 12:18:40 i27249 Exp $
 */
 


#include "debug.h"
void _mError(char* file, int line, const char *func, std::string message)
{
#ifdef DEBUG
	fprintf(stderr, "%s[ERROR]: %sin %s  (%s:%i):%s %s\n",CL_RED, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());
#else
	fprintf(stderr, "%sError:%s %s\n",CL_RED, CL_WHITE, message.c_str());
#endif
#ifdef ENABLE_LOGGING
	FILE *log = fopen("/var/mpkg/errors.log", "a");
	if (log)
	{
		fprintf(log, "%s  (%s:%i): %s\n", func, file, line, message.c_str());
		fclose(log);
	}
#endif



}

void DbgPrint(char* file, int line, const char *func, std::string message) {
#ifdef DEBUG
	fprintf(stdout, "%s[DEBUG] %sin %s  (%s:%i):%s %s\n",CL_GREEN, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());
#endif
#ifdef ENABLE_LOGGING
	FILE *log = fopen("/var/mpkg/debug.log", "a");
	if (log)
	{
		fprintf(log, "%s  (%s:%i): %s\n",func, file, line, message.c_str());
		fclose(log);
	}
#endif


}

