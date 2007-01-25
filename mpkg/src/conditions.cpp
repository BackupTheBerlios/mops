/*
    Dependency and suggestions conditions conversion
    $Id: conditions.cpp,v 1.4 2007/01/25 14:17:13 i27249 Exp $
*/


#include "conditions.h"

int condition2int(string condition)
{
	if (condition==COND_MORE) return VER_MORE;
	if (condition==COND_LESS) return VER_LESS;
	if (condition==COND_EQUAL) return VER_EQUAL;
	if (condition==COND_NOTEQUAL) return VER_NOTEQUAL;
	if (condition==COND_XMORE) return VER_XMORE;
	if (condition==COND_XLESS) return VER_XLESS;
	if (condition==COND_ANY) return VER_ANY;
//#ifdef DEBUG
	printf("conditions.cpp: condition2int(): error input (unknown condition): %s\n", condition.c_str());
//#endif
	return -1; // SUPER_PUPER_ERROR
}
