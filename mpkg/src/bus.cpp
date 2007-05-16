/************************************************************
 * MOPSLinux package management system
 * Message bus implementation
 * $Id: bus.cpp,v 1.10 2007/05/16 02:37:04 i27249 Exp $
 * *********************************************************/
#include "bus.h"
string currentStatus;
string currentItem;
ProgressData pData;
ActionBus actionBus;
//double dlProgress;
//double dlProgressMax;
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

ActionBus::ActionBus()
{
	_abortActions=false;
	_abortComplete=false;
}

ActionBus::~ActionBus()
{
}

int ActionBus::getActionPosition(ActionID actID, bool addIfNone)
{
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (actions.at(i).actionID==actID) return i;
	}
	if (!addIfNone)	return -1;
	else
	{
		return addAction(actID);
	}

}

void ActionBus::setActionProgress(ActionID actID, double p)
{
	actions.at(currentProcessing())._currentProgress=p;
}

void ActionBus::setActionProgressMaximum(ActionID actID, double p)
{
	actions.at(getActionPosition(actID))._progressMaximum=p;
}
unsigned int ActionBus::addAction(ActionID actionID, bool hasPData, bool skip)
{
	int pos = getActionPosition(actionID, false);
	struct ActionState aState;
	aState.actionID=actionID;
	aState.state=ITEMSTATE_WAIT;
	aState.skip=skip;
	aState.skippable=false;
	aState.hasProgressData=hasPData;
	aState._currentProgress=0;
	aState._progressMaximum=0;
	if (pos < 0)
	{
		actions.push_back(aState);
		pos = actions.size()-1;
	}
	else actions.at(pos)=aState;

	return pos;
}

void ActionBus::setSkippable(ActionID actID, bool flag)
{
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (actions.at(i).actionID==actID) actions.at(i).skippable=flag;
	}
}

bool ActionBus::skippable(ActionID actID)
{
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (actions.at(i).actionID==actID) return actions.at(i).skippable;
	}
	return false;
}

unsigned int ActionBus::completed()
{
	unsigned int ret=0;
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (this->getActionState(i)!=ITEMSTATE_WAIT && this->getActionState(i)!=ITEMSTATE_INPROGRESS) ret++;
	}
	return ret;
}

	
unsigned int ActionBus::pending()
{
	unsigned int ret=0;
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (getActionState(i)==ITEMSTATE_WAIT || getActionState(i)==ITEMSTATE_INPROGRESS) ret++;
	}
	return ret;
}
unsigned int ActionBus::size()
{
	return actions.size();
}

unsigned int ActionBus::getActionState(unsigned int pos)
{
	if (pos<actions.size())
	{
		return actions.at(pos).state;
	}
	else
	{
		printf("%s: no such action\n", __func__);
		return 100;
	}
}
bool ActionBus::idle()
{
	bool is_idle=true;
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (getActionState(i)==ITEMSTATE_INPROGRESS) is_idle=false;
	}
	return is_idle;
}

int ActionBus::currentProcessing()
{
	int ret=0;
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (getActionState(i)==ITEMSTATE_INPROGRESS) return i;
	}
	printf ("currentProcessing: no such action, all processes is idle! We sorry, but have to quit now. Go debugging!\n");
	abort();
	return -1;
}

ActionID ActionBus::currentProcessingID()
{
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (getActionState(i)==ITEMSTATE_INPROGRESS) return actions.at(i).actionID;
	}
	//printf("%s: no action is processing, actions.size = %d\n", __func__, actions.size());
	return ACTIONID_NONE;
}

void ActionBus::setCurrentAction(ActionID actID)
{
	bool found=false;
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (getActionState(i)==ITEMSTATE_INPROGRESS)
		{
			printf("Incorrect use of ActionBus detected: multiple processing, autofix by setting flag ITEMSTATE_FINISHED\n");
			setActionState(i, ITEMSTATE_FINISHED);
		}
	}
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (actions.at(i).actionID==actID)
		{
			setActionState(i,ITEMSTATE_INPROGRESS);
			found=true;
		}
	}
	if (!found)
	{
		actions.at(addAction(actID)).state=ITEMSTATE_INPROGRESS;
		printf("Seems to incorrect use of setCurrentAction: no such action found (added)\n");
	}
}

void ActionBus::setActionState(ActionID actID, unsigned int state)
{
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (actions.at(i).actionID==actID){
		       setActionState(i,state);
		       //printf("%s: setting action state %d to %d (ID=%d)\n", __func__, state, i, (int) actID);
		}
	}
}

void ActionBus::setActionState(unsigned int pos, unsigned int state)
{
	if (pos<actions.size()){
		       actions.at(pos).state=state;
		       //printf("%s: setting state %d to action %d (ID=%d)\n", __func__, state, pos, (int) actions.at(pos).actionID);
		}
	//printf("CHECK: %d\n", getActionState(pos));
	//if (state != getActionState(pos)) printf("ERRRRRRRRRRRRRROOOOOOOOOOOOOOOORRRRRRRRRRRRRRRRR!!!!!!!!!!!!!\n");
	
}



void ActionBus::skipAction(ActionID actID)
{
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (actions.at(i).actionID==actID) actions.at(i).skip=true;
	}
}

void ActionBus::abortActions()
{
	_abortActions=true;
}

bool ActionBus::abortComplete()
{
	return _abortComplete;
}

void ActionBus::clear()
{
	actions.resize(0);
	_abortActions=false;
	_abortComplete=false;
}

bool ActionBus::skipped(ActionID actID)
{
	for (unsigned int i=0; i<actions.size(); i++)
	{
		if (actions.at(i).actionID==actID) return actions.at(i).skip;
	}
	return false;
}

double ActionBus::progress()
{
	double ret=0;
	for (unsigned int i=0; i<actions.size(); i++)
	{
		ret = ret + actions.at(i).currentProgress();
	}
	
	return ret;
}

double ActionBus::progressMaximum()
{
	double ret = 0;
	for (unsigned int i=0; i<actions.size(); i++)
	{
		ret = ret + actions.at(i).progressMaximum();
	}
	return ret;
}

ActionState::ActionState()
{
	skippable=false;
}

ActionState::~ActionState()
{
}

double ActionState::currentProgress()
{
	return _currentProgress;
}

double ActionState::progressMaximum()
{
	return _progressMaximum;
}
