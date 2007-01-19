/* Debugging output function(s)
 $Id: debug.cpp,v 1.4 2007/01/19 14:32:42 i27249 Exp $
 */
 


#include "debug.h"


/*
void debug(std::string str)
{
#ifdef DEBUG
	str+="\n";
	printf(str.c_str()); // Comment out this strings to disable debug output
#endif
}
*/

void DbgPrint(char* file, int line, std::string message) {
#ifdef DEBUG
	fprintf(stdout, "[DEBUG] %s:%i %s\n", file, line, message.c_str());
#endif
}

