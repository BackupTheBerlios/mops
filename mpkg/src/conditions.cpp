/*
    Dependency and suggestions conditions conversion
    $Id: conditions.cpp,v 1.7 2007/03/12 00:39:44 i27249 Exp $
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

string hcondition2xml(string condition)
{
	//condition=condition.substr(condition.find_first_of("<>="), condition.find_last_of("<>="));
	if (condition.find(HCOND_MORE)!=std::string::npos) return COND_MORE;
	if (condition.find(HCOND_LESS)!=std::string::npos) return COND_LESS;
	if (condition.find(HCOND_NOTEQUAL)!=std::string::npos) return COND_NOTEQUAL;
	if (condition.find(HCOND_XMORE)!=std::string::npos || condition.find(HCOND_XMORE2)!=std::string::npos) return COND_XMORE;
	if (condition.find(HCOND_XLESS)!=std::string::npos || condition.find(HCOND_XLESS2)!=std::string::npos) return COND_XLESS;
	if (condition.find(HCOND_EQUAL)!=std::string::npos || condition.find(HCOND_EQUAL2)!=std::string::npos) return COND_EQUAL;
	if (condition.find(HCOND_ANY)!=std::string::npos) return COND_ANY;
	printf("Unknown condition [%s]\n", condition.c_str());
	return COND_ANY;
}
