/* Debugging output function(s)
 $Id: debug.cpp,v 1.5 2007/02/22 12:51:19 adiakin Exp $
 */
 


#include "debug.h"



void DbgPrint(char* file, int line, std::string message) {
#ifdef DEBUG
	fprintf(stdout, "[DEBUG] %s:%i %s\n", file, line, message.c_str());
#endif
}

