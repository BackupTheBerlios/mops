/************************************************************
 * MOPSLinux package management system
 * Message bus implementation
 * $Id: bus.cpp,v 1.2 2007/05/10 02:39:08 i27249 Exp $
 * *********************************************************/
#include "bus.h"
string currentStatus;
string currentItem;
double currentProgress2;
double progressMax2;
bool progressEnabled2 = false;
double currentProgress;
double progressMax;
bool progressEnabled = false;
ProgressData pData;
double dlProgress;
double dlProgressMax;
/*static void ProgressData::setProgress(unsigned int itemNum, double progress)
{
	if (itemNum<itemProgress.size()) itemProgress.at(itemNum)=progress;
}
static void ProgressData::setProgressMaximum(unsigned int itemNum, double progress)
{
	if (itemNum<itemProgressMaximum.size()) itemProgressMaximum.at(itemNum)=progress;
}
*/

ProgressData::ProgressData()
{
	currentItem=0;
	downloadAction=false;
}

ProgressData::~ProgressData(){}

int ProgressData::addItem(string iName, double maxProgress, int iState)
{
	itemName.push_back(iName);
	itemCurrentAction.push_back("waiting");
	itemActive.push_back(false);
	itemProgress.push_back(0);
	idleTime.push_back(0);
	itemProgressMaximum.push_back(maxProgress);
	itemState.push_back(iState);
	return itemName.size()-1;
}
void ProgressData::clear()
{
	itemName.resize(0);
	itemCurrentAction.resize(0);
	itemProgress.resize(0);
	itemProgressMaximum.resize(0);
	itemState.resize(0);
	currentAction.clear();
	itemActive.resize(0);
	currentItem=-1;
}
unsigned int ProgressData::size()
{
	return itemName.size();
}
double ProgressData::getTotalProgressMax()
{
	double ret=0;
	for (int i=0; i<itemProgressMaximum.size(); i++)
	{
		ret+=itemProgressMaximum.at(i);
	}
	return ret;
}

double ProgressData::getTotalProgress()
{
	double ret=0;
	for (int i=0; i<itemProgress.size(); i++)
	{
		ret+=itemProgress.at(i);
	}
	return ret;
}



