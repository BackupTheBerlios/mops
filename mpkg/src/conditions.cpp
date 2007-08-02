/*
    Dependency and suggestions conditions conversion
    $Id: conditions.cpp,v 1.11 2007/08/02 10:39:13 i27249 Exp $
*/


#include "conditions.h"
#include "string_operations.h"
int condition2int(string condition)
{
	if (condition==COND_MORE) return VER_MORE;
	if (condition==COND_LESS) return VER_LESS;
	if (condition==COND_EQUAL) return VER_EQUAL;
	if (condition==COND_NOTEQUAL) return VER_NOTEQUAL;
	if (condition==COND_XMORE) return VER_XMORE;
	if (condition==COND_XLESS) return VER_XLESS;
	if (condition==COND_ANY) return VER_ANY;
	mError("error input (unknown condition): " + condition);
	return -1; // SUPER_PUPER_ERROR
}

string hcondition2xml(string condition)
{
	condition = cutSpaces(condition);
	if (condition == HCOND_MORE) return COND_MORE;
	if (condition == HCOND_LESS) return COND_LESS;
	if (condition == HCOND_NOTEQUAL) return COND_NOTEQUAL;
	if (condition == HCOND_XMORE || condition == HCOND_XMORE2) return COND_XMORE;
	if (condition == HCOND_XLESS || condition == HCOND_XLESS2) return COND_XLESS;
	if (condition == HCOND_EQUAL || condition == HCOND_EQUAL2) return COND_EQUAL;
	if (condition == HCOND_ANY) return COND_ANY;
	mError("Unknown condition " + condition);
	return COND_ANY;
}
