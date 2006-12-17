/*
    Dependency and suggestions conditions conversion
    $Id: conditions.cpp,v 1.2 2006/12/17 19:34:57 i27249 Exp $
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
}
