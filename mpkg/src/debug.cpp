#include "debug.h"

void debug(string str)
{
#ifdef ENABLE_DEBUG
	str+="\n";
	printf(str.c_str()); // Comment out this strings to disable debug output
#endif
}
