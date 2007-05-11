/************************************************************
 * MOPSLinux package management system
 * Message bus
 * $Id: bus.h,v 1.6 2007/05/11 10:25:03 i27249 Exp $
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

#define IDLE_MAX 500

class ProgressData
{
	public:
		//static void setProgress(unsigned int itemNum, double progress);
		//static void setProgressMaximum(unsigned int itemNum, double progress);
		void setItemName(int itemID, string name);
		void setItemCurrentAction(int itemID, string action);
		void setItemProgress(int itemID, double progress);
		void increaseItemProgress(int itemID);
		void setItemProgressMaximum(int itemID, double progress);
		void setItemState(int itemID, int state);
		void increaseIdleTime(int itemID);
		void resetIdleTime(int itemID);
		int getIdleTime(int itemID);
		bool itemUpdated(int itemID);
		void setItemChanged(int itemID);
		void setItemUnchanged(int itemID);
		void resetItems(string action="waiting", double __currentProgress=0, double __maxProgress=0, int state=ITEMSTATE_WAIT);

		int addItem(string iName, double maxProgress, int iState=ITEMSTATE_WAIT);


		string getItemName(int itemID);
		string getItemCurrentAction(int itemID);
		double getItemProgress(int itemID);
		double getItemProgressMaximum(int itemID);
		int getItemState(int itemID);
		double getTotalProgress();
		double getTotalProgressMax();
		unsigned int size();
		void clear();

		string getCurrentAction();
		void setCurrentAction(string action);
		ProgressData();
		~ProgressData();

		bool downloadAction;


	private:
		vector<string> itemName;
		vector<string> itemCurrentAction;
		vector<double> itemProgress;
		vector<double> itemProgressMaximum;
		vector<int>itemState;
		vector<unsigned int >itemChanged;
		
		vector<int> idleTime;
		
		string currentAction;
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

typedef enum {
	ACTIONID_DOWNLOAD = 0,
	ACTIONID_INSTALL,
	ACTIONID_REMOVE,
	ACTIONID_PURGE,
	ACTIONID_CACHECHECK,
	ACTIONID_MD5CHECK,
	ACTIONID_DBUPDATE
} ActionID;

struct ActionState
{
	ActionID actionID;
	string actionName;
	unsigned int state;
	bool aborted;
	bool skip;
};


class ActionBus
{
	public:
	vector<ActionState> actions;
	bool _abortActions;
	bool _abortComplete;
	ActionBus();
	~ActionBus();
	unsigned int addAction(ActionID actionID, bool skip=false);
	unsigned int pending();
	unsigned int completed();
	unsigned int size();
	bool idle();
	void clear();
	int currentProcessing(); // returns vector number of processing action in actions array (if exists), otherwise return -1
	void skipAction(ActionID actID);
	void abortActions();
	bool abortComplete();
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
//extern double dlProgress;
//extern double dlProgressMax;


#endif // BUS_H_
