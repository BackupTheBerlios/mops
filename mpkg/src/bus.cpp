/************************************************************
 * MOPSLinux package management system
 * Message bus implementation
 * $Id: bus.cpp,v 1.4 2007/05/11 01:19:35 i27249 Exp $
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
	downloadAction=false;
}

ProgressData::~ProgressData(){}

int ProgressData::addItem(string iName, double maxProgress, int iState)
{
	itemName.push_back(iName);
	itemCurrentAction.push_back("waiting");
	itemChanged.push_back(true);
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
	itemChanged.resize(0);
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

void ProgressData::setItemChanged(int itemID)
{
	if (itemChanged.size()>itemID) itemChanged.at(itemID)=0;
	resetIdleTime(itemID);
}

void ProgressData::setItemUnchanged(int itemID)
{
	if (itemChanged.size()>itemID && itemChanged.at(itemID)<500) itemChanged.at(itemID)++;
}

void ProgressData::setItemName(int itemID, string name)
{
	if (itemName.size()>itemID) itemName.at(itemID)=name;
	setItemChanged(itemID);
}

void ProgressData::setItemCurrentAction(int itemID, string action)
{
	if (itemCurrentAction.size()>itemID) itemCurrentAction.at(itemID)=action;
	setItemChanged(itemID);
}

void ProgressData::setItemProgress(int itemID, double progress)
{
	if (itemProgress.size()>itemID) itemProgress.at(itemID)=progress;
	setItemChanged(itemID);
}

void ProgressData::setItemProgressMaximum(int itemID, double progress)
{
	if (itemProgressMaximum.size()>itemID) itemProgressMaximum.at(itemID)=progress;
}

void ProgressData::setItemState(int itemID, int state)
{
	if (itemState.size()>itemID) itemState.at(itemID)=state;
	setItemChanged(itemID);
}

void ProgressData::increaseIdleTime(int itemID)
{
	if (idleTime.size()>itemID) idleTime.at(itemID)++;
}

void ProgressData::resetIdleTime(int itemID)
{
	if (idleTime.size()>itemID) idleTime.at(itemID)=0;
}

int ProgressData::getIdleTime(int itemID)
{
	if (idleTime.size()>itemID) return idleTime.at(itemID);
	else return 0;
}
bool ProgressData::itemUpdated(int itemID)
{
	if (itemChanged.size()>itemID && itemChanged.at(itemID)<499) return true;
	else
	{
		//printf("Call of unexistent item\n");
		return false;
	}
}
void ProgressData::resetItems(string action, double __currentProgress, double __maxProgress, int state)
{
	for (int i=0; i<itemName.size(); i++)
	{
		setItemState(i, state);
		setItemCurrentAction(i, action);
		setItemProgressMaximum(i, __maxProgress);
		setItemProgress(i,__currentProgress);
		resetIdleTime(i);
	}
}
void ProgressData::increaseItemProgress(int itemID)
{
	if (itemProgress.size()>itemID)
	{
		itemProgress.at(itemID) = itemProgress.at(itemID) + 1;
	}
	else printf("NO ITEM!\n");
}

string ProgressData::getCurrentAction()
{
	return currentAction;
}
void ProgressData::setCurrentAction(string action)
{
	currentAction=action;
}

string ProgressData::getItemName(int itemID)
{
	if (itemName.size()>itemID) return itemName.at(itemID);
	else return "noSuchItem";
}

string ProgressData::getItemCurrentAction(int itemID)
{
	if (itemCurrentAction.size()>itemID) return itemCurrentAction.at(itemID);
	else return "noSuchItem";
}
double ProgressData::getItemProgress(int itemID)
{
	if (itemProgress.size()>itemID) return itemProgress.at(itemID);
	else return 0;
}
double ProgressData::getItemProgressMaximum(int itemID)
{
	if (itemProgressMaximum.size()>itemID) return itemProgressMaximum.at(itemID);
	else return 0;
}
int ProgressData::getItemState(int itemID)
{
	if (itemState.size()>itemID) return itemState.at(itemID);
	else return ITEMSTATE_WAIT;
}


