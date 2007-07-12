#ifndef SETUP_H_INCL
#define SETUP_H_INCL

#include <mpkg/libmpkg.h>
#include <mpkg/dialog.h>
#include <parted/parted.h>
#include <sys/mount.h>
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
	vector<TagPair>otherMounts;
	vector<string>otherMountFSTypes;
	string rootMountPoint;
	string cdromDevice;
	string sourceName;
};
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
void initDatabaseStructure();
void mountMedia();
int selectInstallMethod();
void writeFstab();
void performConfig();
void syncFS();
int packageSourceSelectionMenu();
void showFinish();
void packageSourceSelection();
#endif
