/* Debugging output function(s)
 $Id: debug.cpp,v 1.20 2007/09/06 09:49:58 i27249 Exp $
 */
 

#include "debug.h"
void _mError(char* file, int line, const char *func, std::string message, bool warn)
{
#ifdef DEBUG
	fprintf(stderr, "%s[ERROR]: %sin %s  (%s:%i):%s %s\n",CL_RED, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());
#else
	if (!warn) fprintf(stderr, "%s%s:%s %s\n",CL_RED, _("Error"), CL_WHITE, message.c_str());
	else fprintf(stderr, "%s%s:%s %s\n",CL_YELLOW, _("Warning"), CL_WHITE, message.c_str());
#endif
#ifdef ENABLE_LOGGING
	string logfile = log_directory + "mpkg-errors.log";
	FILE *log = fopen(logfile.c_str(), "a");
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
#ifndef RELEASE
  #ifdef ENABLE_LOGGING
	string logfile = log_directory + "mpkg-debug.log";
	FILE *log = fopen(logfile.c_str(), "a");
	if (log)
	{
		fprintf(log, "%s  (%s:%i): %s\n",func, file, line, message.c_str());
		fclose(log);
	}
  #endif
#endif

}


std::string strim(std::string& s, const std::string& drop)
{
	std::string r=s.erase(s.find_last_not_of(drop)+1);
	return r.erase(0, r.find_first_not_of(drop)); 
}

