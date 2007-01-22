/*************************************************************************
 * System constants - bitmask operators and functions
 * $Id: constants.cpp,v 1.1 2007/01/19 06:11:04 i27249 Exp $
 * ***********************************************************************/
#include <string>
#include "dataunits.h"
using namespace std;

bool stInstalled(int status)
{
	string tmp;
	tmp=IntToStr(status);
	if (tmp[1]=='1')
	{
		debug("constants.cpp: stInstalled: status="+tmp+", ST_INSTALLED=TRUE");
		return true;
	}
	else
	{
		debug("constants.cpp: stInstalled: status="+tmp+", ST_INSTALLED=FALSE");
		return false;
	}

}
bool stRemove(int status)
{
	string tmp;
	tmp=IntToStr(status);
	if (tmp[2]=='1')
	{
		debug("constants.cpp: stRemove: status="+tmp+", ST_REMOVE=TRUE");
		return true;
	}
	else
	{
		debug("constants.cpp: stRemove: status="+tmp+", ST_INSTALLED=FALSE");
		return false;
	}
}

bool stInstall(int status)
{
	string tmp;
	tmp=IntToStr(status);
	if (tmp[3]=='1') return true;
	else return false;
}

bool stPurge(int status)
{
	string tmp;
	tmp=IntToStr(status);
	if (tmp[4]=='1') return true;
	else return false;
}

bool stAvailable(int status)
{
	string tmp;
	tmp=IntToStr(status);
	if (tmp[5]=='1') return true;
	else return false;
}
