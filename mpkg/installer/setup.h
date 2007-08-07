#ifndef SETUP_H_INCL
#define SETUP_H_INCL

#include <mpkg/libmpkg.h>
#include <mpkg/dialog.h>
#include <parted/parted.h>
#include <sys/mount.h>
#include <mpkg/colors.h>
struct pEntry
{
	string devname;
	string fstype;
	string fslabel;
	string size;
	string freespace;
};

struct SysConfig
{
	string swapPartition;
	string rootPartition;
	string rootPartitionType;
	bool rootPartitionFormat;
	vector<TagPair>otherMounts;
	vector<string>otherMountFSTypes;
	vector<string>oldOtherFSTypes;
	vector<bool>otherMountFormat;
	string rootMountPoint;
	string cdromDevice;
	string sourceName;
};
string getLastError();
string doFormatString(bool input);
vector<pEntry> getGoodPartitions(vector<string> goodFSTypes);
bool setPartitionMap();
void showGreeting();
int setSwapSpace();
int setRootPartition();
int setOtherPartitions();
int formatPartitions();
int mountPartitions();
int autoInstall();
int manualInstall();
int initDatabaseStructure();
int moveDatabaseToHdd();
int mountMedia();
int selectInstallMethod();
void writeFstab();
int performConfig();
void syncFS();
int packageSourceSelectionMenu();
void showFinish();
#endif
