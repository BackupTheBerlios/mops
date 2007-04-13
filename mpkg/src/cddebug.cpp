/* Debugging output function(s)
 $Id: cddebug.cpp,v 1.1 2007/04/13 13:52:27 i27249 Exp $
 */
 


#include "debug.h"



void DbgPrint(char* file, int line, std::string message) {
#ifdef DEBUG
	fprintf(stdout, "[DEBUG] %s:%i %s\n", file, line, message.c_str());
#endif
}

