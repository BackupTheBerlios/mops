/* Debugging output function(s)
 $Id: debug.cpp,v 1.3 2006/12/22 10:26:05 adiakin Exp $
 */
 


#include "debug.h"



void debug(std::string str)
{
#ifdef DEBUG
	str+="\n";
	printf(str.c_str()); // Comment out this strings to disable debug output
#endif
}

