/* Debugging output function(s)
 $Id: cddebug.cpp,v 1.2 2007/05/17 15:12:36 i27249 Exp $
 */
 


#include "debug.h"



void DbgPrint(char* file, int line, std::string message) {
#ifdef DEBUG
	say("[DEBUG] %s:%i %s\n", file, line, message.c_str());
#endif
}

