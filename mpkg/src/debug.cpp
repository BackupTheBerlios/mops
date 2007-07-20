/* Debugging output function(s)
 $Id: debug.cpp,v 1.15 2007/07/20 12:38:39 adiakin Exp $
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
	string logfile = log_directory + "mpkg-errors.log";
	FILE *log = fopen(logfile.c_str(), "a");
	if (log)
	{
		fprintf(log, "%s  (%s:%i): %s\n", func, file, line, message.c_str());
		fclose(log);
	}
#endif
	sleep(2);


}

void DbgPrint(char* file, int line, const char *func, std::string message) {
#ifdef DEBUG
	fprintf(stdout, "%s[DEBUG] %sin %s  (%s:%i):%s %s\n",CL_GREEN, CL_YELLOW, func, file, line, CL_WHITE, message.c_str());
#endif
#ifdef ENABLE_LOGGING
	string logfile = log_directory + "mpkg-debug.log";
	FILE *log = fopen(logfile.c_str(), "a");
	if (log)
	{
		fprintf(log, "%s  (%s:%i): %s\n",func, file, line, message.c_str());
		fclose(log);
	}
#endif

}


std::string strim(std::string& s, const std::string& drop)
{
	std::string r=s.erase(s.find_last_not_of(drop)+1);
	return r.erase(0, r.find_first_not_of(drop)); 
}

