/************************************************************
 * MOPSLinux package management system
 * Message bus
 * $Id: bus.h,v 1.2 2007/05/09 17:46:49 i27249 Exp $
 * *********************************************************/

#ifndef BUS_H_
#define BUS_H_

#include "faststl.h"
#include <vector>
using namespace std;
#define ITEMSTATE_WAIT 0
#define ITEMSTATE_INPROGRESS 1
#define ITEMSTATE_FINISHED 2
#define ITEMSTATE_FAILED 3
class ProgressData
{
	public:
		vector<string> itemName;
		vector<string> itemCurrentAction;
		vector<double> itemProgress;
		vector<double> itemProgressMaximum;
		vector<int>itemState;
		vector<bool> itemActive;
		string currentAction;
		int currentItem;
		vector<int> idleTime;
		ProgressData();
		~ProgressData();
		int addItem(string iName, double maxProgress, int iState=ITEMSTATE_WAIT);
		double getTotalProgress();
		double getTotalProgressMax();
		unsigned int size();
		void clear();
};

struct ItemState
{
	unsigned int progress;
	string name;
	unsigned short state;
	bool skipItem;
};

class ProcessState
{
	public:
		vector<ItemState> items;
		string name;
		unsigned short state;
};



class ActionBus
{
	public:
	
	string globalActionName;
	vector<ProcessState> processes;
	
	ActionBus();
	~ActionBus();

	int totalProgress();
	int pending();
	int completed();
	int size();
	bool idle();
	int currentProcessing();
};
extern ActionBus actionBus;
extern string currentStatus;
extern string currentItem;
extern double currentProgress;
extern double progressMax;
extern bool progressEnabled;
extern double currentProgress2;
extern double progressMax2;
extern bool progressEnabled2;
extern ProgressData pData;



#endif // BUS_H_
