/* Debugging output function(s)
 $Id: debug.cpp,v 1.2 2006/12/17 19:34:57 i27249 Exp $
 */
 


#include "debug.h"

void debug(string str)
{
#ifdef ENABLE_DEBUG
	str+="\n";
	printf(str.c_str()); // Comment out this strings to disable debug output
#endif
}
