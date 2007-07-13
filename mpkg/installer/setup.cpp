/****************************************************
 * MOPSLinux: system setup (new generation)
 * $Id: setup.cpp,v 1.20 2007/07/13 14:50:44 i27249 Exp $
 *
 * Required libraries:
 * libparted
 * libmpkg (with libmpkg-http, libstdc++)
 * libcurl (with libreadline, libz)
 * 
 * Required tools:
 * mpkg
 * mkfs.*
 * mkswap
 * swapon/swapoff
 *
 * *************************************************/

#include "setup.h"
string i_menuHead = "Установка MOPSLinux 6.0";
SysConfig systemConfig;
PACKAGE_LIST i_availablePackages;
vector<string> i_tagList;
mpkg *core=NULL;

vector<pEntry> getGoodPartitions(vector<string> goodFSTypes)
{
	mDebug("start");
	ped_device_probe_all();
	vector<PedDevice *> devList;
	vector<PedDisk *> partList;
	vector<PedPartition *> tmpPartList;
	vector< vector<PedPartition *> > partitionList;
	bool enumFinished=false;
	bool partFinished=false;
	PedDevice *tmpDevice=NULL;
	PedDisk *tmpDisk=NULL;
	PedPartition *tmpPartition=NULL;
	//int x=0;
	while (!enumFinished)
	{
		tmpDevice = ped_device_get_next(tmpDevice);
		if (tmpDevice == NULL)
		{
			enumFinished=true;
		}
		else
		{
			devList.push_back(tmpDevice);
			tmpDisk = ped_disk_new(tmpDevice);
			partList.push_back(tmpDisk);
			partFinished=false;
			tmpPartition=NULL;
			tmpPartList.clear();
			while (!partFinished)
			{
				tmpPartition=ped_disk_next_partition(tmpDisk, tmpPartition);
				if (tmpPartition==NULL)
				{
					partFinished=true;
				}
				else
				{
					tmpPartList.push_back(tmpPartition);
				}
			}
			partitionList.push_back(tmpPartList);
		}
	}
	pEntry tmpEntry;
	vector<pEntry> ret;
	for (unsigned int i=0; i<devList.size(); i++)
	{
		for (unsigned int t=0; t<partitionList[i].size(); t++)
		{
			if (partitionList[i][t]->num>0 && partitionList[i][t]->type!=PED_PARTITION_EXTENDED)
			{
				if (partitionList[i][t]->fs_type==NULL)
				{
					// Means that no filesystem is detected on partition
					tmpEntry.devname=(string) ped_partition_get_path(partitionList[i][t]);
					tmpEntry.fstype="unformatted";
					tmpEntry.fslabel="";
					tmpEntry.size=IntToStr(partitionList[i][t]->geom.length*devList[i]->sector_size/1048576);
					ret.push_back(tmpEntry);
				}

				if (goodFSTypes.empty() && partitionList[i][t]->fs_type!=NULL)
				{
					tmpEntry.devname=(string) ped_partition_get_path(partitionList[i][t]);
					tmpEntry.fstype=(string) partitionList[i][t]->fs_type->name;
					if (ped_disk_type_check_feature(ped_disk_probe(devList[i]), PED_DISK_TYPE_PARTITION_NAME))
					{
						tmpEntry.fslabel=(string) ped_partition_get_name(partitionList[i][t]);
					}
					else tmpEntry.fslabel="";
					tmpEntry.size=IntToStr(partitionList[i][t]->geom.length*devList[i]->sector_size/1048576);
					ret.push_back(tmpEntry);
				}

				for (unsigned int z=0; z<goodFSTypes.size(); z++)
				{
					if (partitionList[i][t]->fs_type!=NULL && (string) partitionList[i][t]->fs_type->name == goodFSTypes[z])
					{
						tmpEntry.devname=(string) ped_partition_get_path(partitionList[i][t]);
						tmpEntry.fstype=(string) partitionList[i][t]->fs_type->name;
						tmpEntry.fslabel=(string) ped_partition_get_name(partitionList[i][t]);
						tmpEntry.size=IntToStr(partitionList[i][t]->geom.length*devList[i]->sector_size/1048576);
						ret.push_back(tmpEntry);
					}

				}
			}
		}
	}
	mDebug("end");
	return ret;
}
	

void showGreeting()
{
	mDebug("start");
	Dialog dialogItem("Приветствие");
	dialogItem.execMsgBox("Добро пожаловать в MOPSLinux!\n\nПрограмма установки выполнит установку системы на ваш компьютер\nи проведет первоначальную конфигурацию.\n");
	mDebug("end");
}

int setSwapSpace()
{
	// Selects the partition to use as a swap space
	mDebug("start");
	vector<string> gp;
	vector<pEntry> swapList = getGoodPartitions(gp);
	Dialog dialogItem ("Выбор раздела подкачки");
	if (swapList.empty())
	{
		mDebug("no partitions detected");
		dialogItem.execMsgBox("На жестких дисках не найдено ни одного раздела. Произведите разбивку диска");
		return -3;
	}
	vector<TagPair> sList;
	mDebug("filling options list");
	for (unsigned int i=0; i<swapList.size(); i++)
	{
		sList.push_back(TagPair(swapList[i].devname, swapList[i].fstype + " (" + swapList[i].size + "Mb)"));
	}
	string swapPartition;
selectSwapPartition:
	swapPartition = dialogItem.execMenu("Укажите раздел, который вы желаете использовать в качестве файла подкачки", 0,0,0,sList);
	if (swapPartition.empty())
	{
		return -1;
	}
	for (unsigned int i=0; i<swapList.size(); i++)
	{
		if (swapPartition==swapList[i].devname)
		{
			// We found it! Check it out
			if (swapList[i].fstype.find("linux-swap")==std::string::npos && swapList[i].fstype!="unformatted")
			{
				// Seems that we are trying to overwrite some filesystem, maybe important...
				if (!dialogItem.execYesNo("Вы собираетесь создать раздел подкачки на разделе " + \
							swapList[i].devname + ", на котором уже имеется файловая система " + swapList[i].fstype + \
							". Вы уверены? Все данные будут надежно уничтожены, так что подумайте хорошо"))
				{
					goto selectSwapPartition;
				}
			}
			systemConfig.swapPartition = swapList[i].devname;
		}
	}
	mDebug("end");
	return 0;
}

int activateSwapSpace()
{
	// activates the swap space
	Dialog dialogItem ("Активация раздела подкачки");
	if (!systemConfig.swapPartition.empty())
	{
		string swapoff="swapoff " + systemConfig.swapPartition;
		system(swapoff.c_str());
		string swapMake = "mkswap " + systemConfig.swapPartition;
		if (system(swapMake.c_str())!=0)
		{
			mDebug("error creating swap on " + systemConfig.swapPartition);
			dialogItem.execMsgBox("При создании файла подкачки произошла ошибка, попробуйте другой раздел");
			return -1;
		}
		else
		{
			mDebug("swap created on " + systemConfig.swapPartition);

			swapMake = "swapon " + systemConfig.swapPartition;
			dialogItem.execInfoBox("Активация раздела подкачки...");
			if (system(swapMake.c_str())==0)
			{
				dialogItem.execInfoBox("Раздел подкачки подключен");
			}
		}
	}
	return 0;
}


int setRootPartition()
{
	// Selects the root partition
	mDebug("start");
	vector<string> gp;
	vector<pEntry> pList;
	pList= getGoodPartitions(gp);
	Dialog dialogItem("Настройка раздела для корневой файловой системы");
	vector<TagPair> menuItems;
	for (unsigned int i=0; i<pList.size(); i++)
	{
		//if (pList[i].fstype.find("linux-swap")==std::string::npos)
			menuItems.push_back(TagPair(pList[i].devname, pList[i].fstype + " (" + pList[i].size + "Mb)"));
	}
	string rootPartition;
	rootPartition = dialogItem.execMenu("Укажите корневой раздел для MOPSLinux", 0,0,0,menuItems);
	if (rootPartition.empty())
	{
		mDebug("nothing selected");
		return -1;
	}
	for (unsigned int i=0; i<pList.size(); i++)
	{
		if (pList[i].devname == rootPartition)
		{
			systemConfig.rootPartitionType = pList[i].fstype;
		}
	}
	mDebug("selecting FS type");
	vector<TagPair> formatOptions;
	formatOptions.push_back(TagPair("ext3", "Стандартная журналируемая файловая система Linux."));
	formatOptions.push_back(TagPair("ext2", "Стандартная файловая система Linux (без журналирования)."));
	formatOptions.push_back(TagPair("xfs", "XFS - быстрая файловая система, изначально разработанная SGI для IRIX (мин. 16Мб)"));
	formatOptions.push_back(TagPair("jfs", "Журналируемая файловая система, разработанная IBM (мин. 16Мб)"));
	formatOptions.push_back(TagPair("reiserfs", "Файловая система ReiserFS, хранит данные в сбалансированном дереве (мин. 32Мб)"));
	formatOptions.push_back(TagPair("---", "Не форматировать - оставить как есть, cохранив данные"));
	string formatOption;

	formatOption = dialogItem.execMenu("Вы выбрали " + rootPartition + " в качестве корневой файловой системы.\nКак вы хотите ее отформатировать?",\
			0,0,0,formatOptions);
	if (formatOption.empty())
	{
		return -1;
	}
	
	systemConfig.rootPartition = rootPartition;

	if (formatOption!="---")
	{
		systemConfig.rootPartitionFormat=true;
		systemConfig.rootPartitionType = formatOption;
		mDebug("set to format " + rootPartition + " as " + formatOption);
		umount(rootPartition.c_str());
	}

	else
	{
		systemConfig.rootPartitionFormat=false;
	}
		
	mDebug("end");
	return 0;

}

int setOtherPartitionItems()
{
	// Initializes partition list.
	vector<string> gp;
	vector<pEntry> pList_raw = getGoodPartitions(gp);
	systemConfig.otherMountFSTypes.clear();
	systemConfig.otherMounts.clear();
	systemConfig.otherMountFormat.clear();
	for (unsigned int i=0; i<pList_raw.size(); i++)
	{
		systemConfig.otherMounts.push_back(TagPair(pList_raw[i].devname, "не подключено"));
		systemConfig.otherMountFSTypes.push_back(pList_raw[i].fstype);
		systemConfig.otherMountFormat.push_back(false);
	}
	return 0;
}

int setOtherOptions(string devName)
{
	// Sets an options for other partitions
	Dialog d("Настройка параметров подключения " + devName);
	string fstype_ret;
	string mountpoint_ret;
	vector<TagPair> formatOptions;
	formatOptions.push_back(TagPair("ext3", "Стандартная журналируемая файловая система Linux."));
	formatOptions.push_back(TagPair("ext2", "Стандартная файловая система Linux (без журналирования)."));
	formatOptions.push_back(TagPair("xfs", "XFS - быстрая файловая система для данных, изначально разработанная SGI для IRIX (мин. 16Мб)"));
	formatOptions.push_back(TagPair("jfs", "Журналируемая файловая система, разработанная IBM (мин. 16Мб)"));
	formatOptions.push_back(TagPair("reiserfs", "Файловая система ReiserFS, хранит данные в сбалансированном дереве (мин. 32Мб)"));
	formatOptions.push_back(TagPair("---", "Не форматировать - оставить как есть, cохранив данные"));

	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		if (systemConfig.otherMounts[i].tag == devName)
		{
			if (systemConfig.otherMounts[i].value[0]== '/') mountpoint_ret = systemConfig.otherMounts[i].value; 
			fstype_ret = systemConfig.otherMountFSTypes[i];
select_mp:
			mountpoint_ret = d.execInputBox("Выберите точку подключения для " + systemConfig.otherMounts[i].tag, mountpoint_ret);
			if (mountpoint_ret.empty()) return -1;
			if (mountpoint_ret[0]!='/')
			{
				d.execMsgBox("Точка подключения " + mountpoint_ret + " некорректна. Это должен быть абсолютный путь, например /usr/local");
				goto select_mp;
			}
			systemConfig.otherMounts[i].value = mountpoint_ret;

			fstype_ret = d.execMenu("Выберите способ форматирования раздела " + systemConfig.otherMounts[i].tag,0,0,0,formatOptions);
			if (fstype_ret.empty()) return -1;
			if (fstype_ret == "---")
			{
				systemConfig.otherMountFormat[i]=false;
			}
			else
			{
				systemConfig.otherMountFormat[i]=true;
				systemConfig.otherMountFSTypes[i]=fstype_ret;
			}
			return 0;
		}
	}
	// If we reach this point - this mean that nothing was found...
	mError("No such device " + devName);
	sleep(4);
	return -2;

}

int setOtherPartitions()
{
	Dialog d("Подключение других разделов");
	vector<TagPair> menuItems;
	string ret;
	TagPair tmpTag;
other_part_menu_main:
	menuItems.clear();
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		if (systemConfig.otherMounts[i].tag!=systemConfig.rootPartition && systemConfig.otherMounts[i].tag!=systemConfig.swapPartition)
		{
			tmpTag = systemConfig.otherMounts[i];
			if (tmpTag.value[0]=='/') 
				tmpTag.value += ", " + systemConfig.otherMountFSTypes[i] + ", форматирование: " + doFormatString(systemConfig.otherMountFormat[i]);
			menuItems.push_back(tmpTag);
		}
	}
	menuItems.push_back(TagPair("Готово", "Продолжить"));
	ret = d.execMenu("Выберите точки подключения для остальных разделов",0,0,0,menuItems);
	if (ret.empty()) return -1;
	if (ret == "Готово") return 0;
	setOtherOptions(ret);
	goto other_part_menu_main;
}
bool formatPartition(string devname, string fstype)
{
	if (!simulate)
	{
		string cmd = "mkfs -t " + fstype + " " + devname;
		if (system(cmd)==0) return true;
		else return false;
	}
	// Simulating format =)
	sleep(3);
	return true;
}
int formatPartitions()
{
	Dialog d("Форматирование разделов");
	// Do the actual format
	if (systemConfig.rootPartitionFormat)
	{
		d.execInfoBox("Форматирование корневой файловой системы " + systemConfig.rootPartition + \
				", подключение: /\nТип файловой системы: " + systemConfig.rootPartitionType);
		if (!formatPartition(systemConfig.rootPartition, systemConfig.rootPartitionType)) 
		{
			d.execMsgBox("При форматировании корневой ФС произошла ошибка");
			return -1;
		}
	}
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		if (systemConfig.otherMountFormat[i])
		{
			d.execInfoBox("Форматирование файловой системы " + systemConfig.otherMounts[i].tag + \
					", подключение: " + systemConfig.otherMounts[i].value + "\nТип файловой системы: " + systemConfig.otherMountFSTypes[i]);
			if (!formatPartition(systemConfig.otherMounts[i].tag, systemConfig.otherMountFSTypes[i]))
			{
				d.execMsgBox("При форматировании " + systemConfig.otherMounts[i].tag + " произошла ошибка");
				return -1;
			}
		}
	}
	return 0;

}



int mountPartitions()
{
	// Mount all HDD partitions
	mDebug("start");
	Dialog dialogItem("Монтирование разделов");
	string mount_cmd;
	string mkdir_cmd;
	dialogItem.execInfoBox("Монтируется корневая файловая система ("+systemConfig.rootPartition + ")");

	mkdir_cmd = "mkdir -p " + systemConfig.rootMountPoint;
	mount_cmd = "mount " + systemConfig.rootPartition + " " + systemConfig.rootMountPoint;
	mDebug("creating root mount point: " + mkdir_cmd);
	mDebug("and mounting: " + mount_cmd);
	if (system(mkdir_cmd.c_str()) !=0 || system(mount_cmd.c_str())!=0)
	{
		mDebug("mkdir or mount failed");
		dialogItem.execInfoBox("Произошла ошибка при монтировании корневой файловой системы.\nПроверьте всё и начните установку заново");
		abort();
	}
	else mDebug("root mkdir and mount OK");

	dialogItem.execInfoBox("Перемещение базы данных на жесткий диск");
	moveDatabaseToHdd();
	// Sorting mount points
	vector<int> mountOrder, mountPriority;
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		mountPriority.push_back(0);
	}
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		for (unsigned int t=0; t<systemConfig.otherMounts.size(); t++)
		{
			if (systemConfig.otherMounts[i].value.find(systemConfig.otherMounts[t].value)==0)
			{
				mountPriority[i]++;
			}
			if (systemConfig.otherMounts[i].value[0]!='/') mountPriority[i]=-1;
		}
	}
	for (unsigned int i=0; i<mountPriority.size(); i++)
	{
		for (unsigned int t=0; t<mountPriority.size(); t++)
		{
			if (mountPriority[t]==(int) i)
			{
				mountOrder.push_back(t);
			}
		}
	}
	if (mountPriority.size()!=systemConfig.otherMounts.size()) 
	{
		mError("mount code error, size mismatch: priority size = " + \
			IntToStr(mountPriority.size()) + \
			"queue size: " + IntToStr(systemConfig.otherMounts.size())); 
		abort();
	}

	// Mounting others...
	
	for (unsigned int i=0; i<mountOrder.size(); i++)
	{
		dialogItem.execInfoBox("Монтируются дополнительные файловые системы: " + systemConfig.otherMounts[mountOrder[i]].tag + " ["+ \
				systemConfig.otherMounts[mountOrder[i]].value+"]");
		mkdir_cmd = "mkdir -p " + systemConfig.rootMountPoint+systemConfig.otherMounts[mountOrder[i]].value;
		mount_cmd = "mount " + systemConfig.otherMounts[mountOrder[i]].tag + " " + systemConfig.rootMountPoint+systemConfig.otherMounts[mountOrder[i]].value;

		mDebug("Attempting to mkdir: " + mkdir_cmd);
	       	mDebug("and Attempting to mount: " + mount_cmd);	
		if (system(mkdir_cmd.c_str())!=0 || system(mount_cmd.c_str())!=0)
		{
			mDebug("error while mount");
			dialogItem.execInfoBox("Произошла ошибка при монтировании файловой системы " + \
					systemConfig.otherMounts[mountOrder[i]].tag + "\nПроверьте всё и начните установку заново");
			return -1;
		}
		else mDebug("mount ok");
	}
	mDebug("end");
	return 0;
}


string getTagDescription(string tag)
{
	return tag;
}

int initDatabaseStructure()
{
	say("init: deleting core\n");
	if (core!=NULL) delete core;
	say("init: core removed\n");
	if (system("rm -rf /var/mpkg; mkdir -p /var/mpkg && mkdir -p /var/mpkg/scripts && mkdir -p /var/mpkg/backup && mkdir -p /var/mpkg/cache && cp -f /packages.db /var/mpkg/packages.db")!=0)
	{
		mError("При создании структуры каталогов базы данных произошла ошибка");
		sleep(2);
		return -1;
	}
	return 0;
}

int moveDatabaseToHdd()
{
	say("move: deleting core");
	if (core!=NULL) delete core;
	say("move: deleted core");
	log_directory = "/mnt/var/log/";
	if (system("rm -rf /mnt/var/mpkg; mkdir -p /mnt/var/log; cp -R /var/mpkg /mnt/var/ && rm -rf /var/mpkg && ln -s /mnt/var/mpkg /var/mpkg")!=0)
	{
		mError("Ошибка при перемещении данных на жесткий диск");
		sleep(2);
		return -1;
	}
	return 0;
}

int mountMedia()
{
	mDebug("start");
	if (umount("/var/log/mount")==0) mDebug("successfully unmounted old mount");
	else mDebug("cd wasn't mounted or unmount error. Processing to detection");
	Dialog dialogItem ("Поиск и монтирование привода CD/DVD");
	dialogItem.execInfoBox("Попытка автоматического определения DVD-ROM...");
	vector<string> devList;
	// IDE drives
	devList.push_back("/dev/hda");
	devList.push_back("/dev/hdb");
	devList.push_back("/dev/hdc");
	devList.push_back("/dev/hdd");
	devList.push_back("/dev/hde");
	devList.push_back("/dev/hdf");
	devList.push_back("/dev/hdg");
	devList.push_back("/dev/hdh");
	// SCSI CD-ROM
	devList.push_back("/dev/scd0");
	devList.push_back("/dev/scd1");
	devList.push_back("/dev/scd2");
	devList.push_back("/dev/scd3");
	devList.push_back("/dev/scd4");
	devList.push_back("/dev/scd5");
	devList.push_back("/dev/scd6");
	devList.push_back("/dev/scd7");
	devList.push_back("/dev/scd8");
	devList.push_back("/dev/scd9");
	devList.push_back("/dev/scd10");
	devList.push_back("/dev/scd11");
	devList.push_back("/dev/scd12");
	devList.push_back("/dev/scd13");
	devList.push_back("/dev/scd14");
	devList.push_back("/dev/scd15");

	// SCSI/SATA CD-RW
	devList.push_back("/dev/sr0");
	devList.push_back("/dev/sr1");
	devList.push_back("/dev/sr2");
	devList.push_back("/dev/sr3");
	devList.push_back("/dev/sr4");
	devList.push_back("/dev/sr5");
	devList.push_back("/dev/sr6");
	devList.push_back("/dev/sr7");
	devList.push_back("/dev/sr8");
	devList.push_back("/dev/sr9");
	devList.push_back("/dev/sr10");
	devList.push_back("/dev/sr11");
	devList.push_back("/dev/sr12");
	devList.push_back("/dev/sr13");
	devList.push_back("/dev/sr14");
	devList.push_back("/dev/sr15");

	// LPT drives
	devList.push_back("/dev/pcd0");
	devList.push_back("/dev/pcd1");
	devList.push_back("/dev/pcd2");
	devList.push_back("/dev/pcd3");

	// Ugly (pre-ide) drives
	devList.push_back("/dev/sonycd");
	devList.push_back("/dev/optcd");
	devList.push_back("/dev/sjcd");
	devList.push_back("/dev/mcdx0");
	devList.push_back("/dev/mcdx1");
	devList.push_back("/dev/cdu535");
	devList.push_back("/dev/sbpcd");
	devList.push_back("/dev/aztcd");
	devList.push_back("/dev/cm205cd");
	devList.push_back("/dev/cm206cd");
	devList.push_back("/dev/bpcd");
	devList.push_back("/dev/mcd");

	// Trying to mount
	string cmd;
	for (unsigned int i=0; i<devList.size(); i++)
	{
		mDebug("searching DVD at " + devList[i]);
		cmd = "mount -t iso9660 " + devList[i]+" /var/log/mount 2>> /var/install.log";
		if (system(cmd.c_str())==0)
		{
			mDebug("Successfully found at " + devList[i]);
			systemConfig.cdromDevice=devList[i];
			dialogItem.execInfoBox("Диск успешно найден в " + devList[i]);
			mDebug("end");
			return 0;
		}
	}
	mDebug("Drive not found. Proceeding to manual selection");
	// Failed? Select manually please
	if (!dialogItem.execYesNo("Автоопределение привода не удалось. Хотите указать вручную?"))
	{
		mDebug("aborted");
		return -1;
	}
manual:
	mDebug("manual selection");
	string manualMount = dialogItem.execInputBox("Укажите вручную имя устройства (например /dev/hda)", "/dev/");
	if (manualMount.empty())
	{
		return -1; //if (dialogItem.execYesNo("Вы ничего не указали. Выйти из программы установки?")) { mDebug("aborted"); abort(); }
	}
	cmd = "mount -t iso9660 " + manualMount + " /var/log/mount 2>> /var/install.log";
	if (system(cmd.c_str())!=0)
	{
		mDebug("manual mount error");
		dialogItem.execMsgBox("Не удалось смонтировать указанный вами диск");
		goto manual;
	}
	return 0;
	mDebug("end");
}
bool showLicense()
{
	Dialog d("Лицензионное соглашение");
	if (system((string) "dialog --extra-button --extra-label \"Не принимаю\" --no-cancel --ok-label \"Принимаю\"	--textbox /license 0 0")==0) return true;
	else return false;
	
}
void writeFstab()
{
	mDebug("start");
	string data = systemConfig.rootPartition + "\t/\t" + systemConfig.rootPartitionType + "\tdefaults\t1 1\n" + \
	(string) "devpts\t/dev/pts\tdevpts\tgid=5,mode=620\t0 0\n" + \
	(string) "proc\t/proc\tproc\tdefaults\t0 0\n";
	string options="defaults";
	string fstype="auto";
	if (!systemConfig.swapPartition.empty()) data += systemConfig.swapPartition + "\tswap\tswap\tdefaults\t0 0\n";
	
	// Sorting
	vector<int> mountOrder, mountPriority;
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		mountPriority.push_back(0);
	}
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		for (unsigned int t=0; t<systemConfig.otherMounts.size(); t++)
		{
			if (systemConfig.otherMounts[i].value.find(systemConfig.otherMounts[t].value)==0)
			{
				mountPriority[i]++;
			}
			if (systemConfig.otherMounts[i].value[0]!='/') mountPriority[i]=-1;
		}
	}
	for (unsigned int i=0; i<mountPriority.size(); i++)
	{
		for (unsigned int t=0; t<mountPriority.size(); t++)
		{
			if (mountPriority[t]== (int) i)
			{
				mountOrder.push_back(t);
			}
		}
	}
	if (mountPriority.size()!=systemConfig.otherMounts.size()) 
	{
		mError("mount code error, size mismatch: priority size = " + \
			IntToStr(mountPriority.size()) + \
			"queue size: " + IntToStr(systemConfig.otherMounts.size())); 
		abort();
	}
	// Storing data
	for (unsigned int i=0; i<mountOrder.size(); i++)
	{
		options="defaults";
		fstype="auto";
		if (systemConfig.otherMountFSTypes[mountOrder[i]].find("fat")!=std::string::npos)
		{
			options="codepage=866,iocharset=utf-8,user,users,umask=022";
			fstype="vfat";
		}
		if (systemConfig.otherMountFSTypes[mountOrder[i]].find("ntfs")!=std::string::npos)
		{
			options="locale=ru_RU.utf8,umask=022";
			fstype="ntfs-3g";
		}
		data += systemConfig.otherMounts[mountOrder[i]].tag + "\t" + systemConfig.otherMounts[mountOrder[i]].value + "\t"+fstype+"\t" + options + "\t0 0";
	}
	mDebug("data is:\n"+data);
	
	if (!simulate) WriteFile(systemConfig.rootMountPoint + "/etc/fstab", data);
	mDebug("end");
}



int performConfig()
{

	writeFstab();
	mDebug("start");
	setenv("ROOT_DEVICE", systemConfig.rootPartition.c_str(), 1);
	WriteFile("/var/log/setup/tmp/SeTT_PX", systemConfig.rootMountPoint);
	WriteFile("/var/log/setup/tmp/SeTrootdev", systemConfig.rootPartition);
	if (system("/usr/lib/setup/SeTconfig")==0)
	{
		mDebug("Config seems to be ok");
	}
	else 
	{
		Dialog d("Ошибка");
		d.execMsgBox("При выполнении настройки системы произошла ошибка. Устраните причины и повторите установку заново.");
		mDebug("Config seems to be failed");
		return -1;
	}
	mDebug("end");
	return 0;
}

void syncFS()
{
	mDebug("start");
	system("sync");
	umount(systemConfig.cdromDevice.c_str());
	mDebug("end");
}

void cleanup()
{
	if (core==NULL) core = new mpkg;
	core->clean_cache(true);
}

void showFinish()
{
	mDebug("start");
	Dialog dialogItem("Завершение установки");
	dialogItem.execMsgBox("Установка завершена успешно! Перезагрузитесь и наслаждайтесь!");
	mDebug("finish");
	exit(0);
}

int packageSelectionMenu()
{
	Dialog d("Выбор пакетов для установки");
	vector<TagPair> menuItems;
	string ret;
	int i_ret=0;
	bool mark;
	PACKAGE *p;
	string ins_type;
	menuItems.push_back(TagPair("1","Десктоп (для домашнего и офисного применения)"));
	menuItems.push_back(TagPair("2","Сервер (основные сервисы, без X11)"));
	menuItems.push_back(TagPair("3","Тонкий клиент (минимальная установка с X11, умещается на 512Мб)"));
	menuItems.push_back(TagPair("4","Минимальная установка (только базовая система)"));
	menuItems.push_back(TagPair("5","Полная установка (абсолютно все пакеты)"));
	ret = d.execMenu("Выберите набор пакетов для установки.\nОни используются как отправная точка в выборе пакетов.\nВы сможете детально отредактировать список устанавливаемых компонентов",0,0,0,menuItems);
	if (ret.empty()) return 0;

	i_ret = atoi(ret.c_str());

	switch(i_ret)
	{
		case 1: ins_type = "Десктоп";
			break;
		case 2:
			ins_type = "Сервер";
			break;
		case 3:
			ins_type = "Тонкий клиент";
			break;
		case 4:
			ins_type = "Минимум";
			break;
		case 5:
			ins_type = "Полностью";
			break;
		default:
			ins_type = "Я хз что вы там выбрали...";
	}

	if (core==NULL) core = new mpkg;
	SQLRecord sqlSearch;
	printf("cleaning queue\n");
	core->clean_queue();
	printf("clean complete\n");
	core->get_packagelist(&sqlSearch, &i_availablePackages);
	core->get_available_tags(&i_tagList);
	
	// Predefined groups preselect
	for (int i=0; i<i_availablePackages.size(); i++)
	{
		p = i_availablePackages.get_package(i);
		mark=false;
		switch(i_ret)
		{
			case 1:
				if (p->isTaggedBy("desktop")) mark=true;
				break;
			case 2:
				if (p->isTaggedBy("server")) mark=true;
				break;
			case 3:
				if (p->isTaggedBy("thinclient")) mark=true;
				break;
			case 4:
				if (p->isTaggedBy("base")) mark=true;
				break;
			case 5:
				mark=true;
				break;
			default:
				mError("Index " + ret + " is out of range, cannot continue");
				abort();
		}
		if (mark)
		{
			p->set_action(ST_INSTALL);
		}
	}
	
	// Executing adjustment
	bool showThisItem;
group_adjust_menu:
	menuItems.clear();
	menuItems.push_back(TagPair("Готово", "Все готово"));
	for (unsigned int i=0; i<i_tagList.size(); i++)
	{
		if (i_tagList[i]!="desktop" && i_tagList[i]!="server" && i_tagList[i]!="thinclient" && i_tagList[i]!="base")
		{
			menuItems.push_back(TagPair(i_tagList[i], getTagDescription(i_tagList[i])));
		}
	}
	menuItems.push_back(TagPair("extras", "Пакеты, не вошедшие ни в одну из групп"));
	menuItems.push_back(TagPair("everything", "Единый список всех пакетов"));
	
	ret = d.execMenu("Вы выбрали тип установки " + ins_type + "\nОтредактируйте список пакетов и нажмите Готово",0,0,0,menuItems);
	if (ret == "Готово") return 0;
	else
	{
		menuItems.clear();
		for (int i=0; i<i_availablePackages.size(); i++)
		{
			showThisItem=false;
			p = i_availablePackages.get_package(i);
			if (ret == "everything") showThisItem=true;
			if (p->isTaggedBy(ret)) showThisItem=true;
			if (ret == "extras" && p->get_tags()->empty()) showThisItem=true;
			
			if (showThisItem)
			{
				menuItems.push_back(TagPair(IntToStr(p->get_id()), *p->get_name() + " (" + *p->get_short_description() + ")", p->action()==ST_INSTALL));
			}
		}
		if (d.execCheckList("Выберите пакеты для установки (группа " + ret + ")",0,0,0, &menuItems))
		{
			for (unsigned int i=0; i<menuItems.size(); i++)
			{
				for (int t=0; t<i_availablePackages.size(); t++)
				{
					if (atoi(menuItems[i].tag.c_str()) == i_availablePackages.get_package(t)->get_id())
					{
						if (menuItems[i].checkState) i_availablePackages.get_package(t)->set_action(ST_INSTALL);
						else i_availablePackages.get_package(t)->set_action(ST_NONE);
					}
				}
			}

		}
		goto group_adjust_menu;
	}


					


	

	return 0;

}

int commit()
{
	Dialog d("Подтвердите правильность параметров");
	string summary;
	string errors;
	bool has_queue=false;
	if (systemConfig.rootPartition.empty()) errors += "Не указан корневой раздел системы\n";
	else if (systemConfig.rootPartition==systemConfig.swapPartition) errors += "У вас раздел для swap и корневой раздел совпадают, вы в своем уме?!\n";
	if (systemConfig.sourceName.empty()) errors += "Не указан источник пакетов\n";
	else
	{
		if (i_availablePackages.IsEmpty()) errors += "Не обнаружено доступных пакетов\n";
		for (int i=0; i<i_availablePackages.size(); i++)
		{
			if (i_availablePackages.get_package(i)->action()==ST_INSTALL) has_queue=true;
		}
		if (!has_queue) errors += "Не выбран набор устанавливаемых пакетов\n";
	}
	

	if (!errors.empty())
	{
		d.execMsgBox(errors);
		return -1;
	}
	
	summary = "Проверьте внимательно все данные прежде чем продолжить!\n\n" + (string) \
		   "Корневой раздел: " + systemConfig.rootPartition + ", " + systemConfig.rootPartitionType + "\n" + \
		   "Раздел подкачки: ";
       if (systemConfig.swapPartition.empty()) summary += "нет\n";
       else summary += systemConfig.swapPartition + "\n";
       summary += "Другие разделы: ";
	if (systemConfig.otherMounts.empty()) summary += "нет\n";
	else
	{
		for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
		{
			if (systemConfig.otherMounts[i].value[0]=='/')
				summary += "                " + \
				    systemConfig.otherMounts[i].tag + \
				    " (" + systemConfig.otherMounts[i].value + "), форматирование: " + systemConfig.otherMountFSTypes[i] + "\n";
		}
	}
	summary += "Источник пакетов: " + systemConfig.sourceName + "\n" + \
		    "\nМожно выполнять установку?";
	if (d.execYesNo(summary))
	{
		if (!simulate)
		{
			if (activateSwapSpace()!=0) return -1;
			if (formatPartitions()!=0) return -1;
			//if (mountMedia()!=0) return -1;
			if (mountPartitions()!=0) return -1;
			if (core==NULL) core = new mpkg;
			d.execInfoBox("Построение очереди...");
			for (int i=0; i<i_availablePackages.size(); i++)
			{
				if (i_availablePackages.get_package(i)->action()==ST_INSTALL) core->install(i_availablePackages.get_package(i));
			}
			if (core->commit()!=0) return -1;
			if (performConfig()!=0) return -1;
			syncFS();//!=0) return -1;
		}
		showFinish();
	}
	return 0;
}


int setCDSource()
{
	// Todo
	Dialog d("Установка с набора CD");
	d.execMsgBox("Пока не доделал, данный вариант недоступен...");
	return -1;
}

int setDVDSource()
{
	Dialog dialogItem ("Поиск пакетов на DVD");
	vector<string> nullList, rList;
start:
	if (core==NULL) core = new mpkg;
	rList.push_back("file:///var/log/mount/mops/");

	mountMedia();
	dialogItem.execInfoBox("Обновление списка пакетов...");
	core->set_repositorylist(rList, nullList);
	if (core->update_repository_data()!=0)
	{
		mDebug("update failed");
		delete core;
		core=NULL;
		if (dialogItem.execYesNo("При создании начальной базы пакетов произошла ошибка. Проверьте доступность источникa либо измените список."))
		{
			goto start;
		}
		else abort();
	}
	else
	{
		mDebug("ok");
	}

	delete core;
	core=NULL;
	systemConfig.sourceName="DVD";
	return 0;
}

int setHDDSource()
{
	Dialog d("Поиск пакетов на жестком диске");
	if (d.execYesNo("Сначала вы должны смонтировать нужный раздел в любую папку. Открыть консоль?")) 
	{
		say("Смонтируйте нужный диск в любую папку, перейдите в директорию с пакетами, и наберите this\n");
		system("/bin/bash");
	}
	string ret="/", index_name;
	vector<string> rList, nullList;
enter_path:
	ret = ReadFile("/var/log/hdddir");
	if (ret.find_first_of("\n\r")!=std::string::npos) ret = ret.substr(0,ret.find_first_of("\n\r"));
	if (ret.empty()) ret = "/";
	ret = d.execInputBox("Укажите АБСОЛЮТНЫЙ путь к каталогу с пакетами:", ret);
	if (ret.empty()) { packageSourceSelectionMenu(); return 0;}
	index_name = ret + "/packages.xml.gz";

	if (access(index_name.c_str(), R_OK)==0)
	{
		rList.clear();
		rList.push_back("file://" + ret + "/");
		d.execInfoBox("Обновление списка пакетов...");
		if (core==NULL) core = new mpkg;
		core->set_repositorylist(rList, nullList);
		if (core->update_repository_data()!=0)
		{
			mDebug("update failed");
			delete core;
			core=NULL;
			d.execMsgBox("При загрузке списка пакетов произошла ошибка. Проверьте корректность источникa.");
			goto enter_path;
		}
		else
		{
			mDebug("ok");
		}
		delete core;
		core=NULL;
		systemConfig.sourceName="file://" + ret + "/";
	}
	else 
	{
		d.execMsgBox("В указанном вами каталоге (" + ret + ") репозиторий не найден.\nПроверьте путь и повторите попытку");
		goto enter_path;
	}
	return 0;
}

int setNetworkSource()
{
	Dialog d("Поиск пакетов в сети");
	vector<string> nullList, rList;
	string ret;
start:
	if (core==NULL) core = new mpkg;
	if (systemConfig.sourceName.find("Из сети")==0 && !REPOSITORY_LIST.empty()) ret = REPOSITORY_LIST[0];

	rList.clear();
	ret = d.execInputBox("Введите URL сетевого репозитория. Позднее вы сможете добавить дополнительные","ftp://");
	if (!ret.empty()) rList.push_back(ret);
	
	//d.execAddableList("Введите URL дополнительных сетевых репозиториев. Можете указать несколько.", &rList, "://");
	// Redefine repository list
	if (rList.empty())
	{
		if (d.execYesNo("Вы не указали ни одного источника пакетов. Использовать репозиторий по умолчанию?"))
		{
			rList.push_back("ftp://ftp.rpunet.ru/mopslinux-current/mops/");
		}
	}
	if (rList.size()==1) systemConfig.sourceName="Из сети: " + rList[0];
	else systemConfig.sourceName="Из сети (несколько источников)";
	d.execInfoBox("Обновление списка пакетов...");
	core->set_repositorylist(rList, nullList);
	if (core->update_repository_data()!=0)
	{
		mDebug("update failed");
		delete core;
		core=NULL;
		if (d.execYesNo("При загрузке списка пакетов произошла ошибка. Проверьте введенные данные, и убедитесь что сеть работает"))
		{
			goto start;
		}
		else return 0;
	}
	else
	{
		mDebug("ok");
	}

	delete core;
	core=NULL;
	return 0;
}

int packageSourceSelectionMenu()
{

	Dialog d("Выбор источника пакетов");
	vector<TagPair> menuItems;
	string ret;

	menuItems.clear();
	menuItems.push_back(TagPair("CD-ROM", "Установка с набора CD"));
	menuItems.push_back(TagPair("DVD-ROM", "Установка с привода DVD"));
	menuItems.push_back(TagPair("HDD", "Установка с жесткого диска"));
	menuItems.push_back(TagPair("Network", "Установка по сети (FTP, HTTP)"));
	menuItems.push_back(TagPair("Назад", "Выйти в главное меню"));
	ret = d.execMenu("Выберите источник пакетов для установки системы", 0,0,0,menuItems);
	if (ret == "DVD-ROM") setDVDSource();
	if (ret == "HDD") setHDDSource();
	if (ret == "Network") setNetworkSource();
	if (ret == "CD-ROM") setCDSource();
	return 0;
}
	
int diskPartitioningMenu()
{
	Dialog d("Разбивка диска на разделы");
	vector<TagPair> menuItems;
	string ret;
	string disk_name;
	int r = 0;
part_menu:
	menuItems.clear();
	menuItems.push_back(TagPair("cfdisk", "Простое меню для разбивки"));
	//menuItems.push_back(TagPair("fdisk", "Разбивка диска из командной строки (экспертный режим)"));
	menuItems.push_back(TagPair("parted", "Разбивка диска из командной строки (экспертный режим)"));
	menuItems.push_back(TagPair("Готово", "Все готово, вернуться в главное меню"));

	ret = d.execMenu("Выберите способ разбивки диска",0,0,0,menuItems);
	if (ret == "Готово" || ret.empty()) return 0;
disk_menu:
	disk_name = d.execInputBox("Какой диск вы хотите разметить? Оставьте поле пустым, если хотите взять первый попавшийся", disk_name);
	if (disk_name.find("/dev/")!=0) goto disk_menu;


	if (ret == "cfdisk") r = system("cfdisk " + disk_name);
	//if (ret == "fdisk") r = system("fdisk");	// Надо явно указывать какой диск разбивать. Сделаю потом.
	if (ret == "parted") r = system("parted " + disk_name);
	if (r==0) return 0;
	else goto part_menu;
}
string doFormatString(bool input)
{
	if (input) return "ДА";
	else return "нет";
}
int main(int argc, char *argv[])
{
	simulate=false;
	if (argc>=2)
	{
		if (strcmp(argv[1], "--simulate")==0)
		{
			simulate=true;
			say("Simulation mode!\n");
			sleep(1);
		}
		if (strcmp(argv[1], "--skip-check")==0)
		{
			forceSkipLinkMD5Checks=true;
		}
		else
		{
			printf("MOPSLinux installer 2.0 alpha\n");
			printf("Usage:\n");
			printf("setup [options]\n\n");

			printf("\t--skip-check     Skip package integrity check\n");
			printf("\t--simulate       Simulate only, do not install (not fully implemented yet)\n");
			exit(0);
		}
	}
	dialogMode=true;


	Dialog d ("Главное меню");
	showGreeting();
	if (!showLicense())
	{
		d.execMsgBox("Без принятия условий данной лицензии использование данного программного обеспечения невозможно.\nУстановка прекращена.");
		return -1;
	}

	systemConfig.rootMountPoint="/mnt";
	setOtherPartitionItems();
	string ret;
	string next_item;
	vector<TagPair> menuItems;

	initDatabaseStructure();
main_menu:
	menuItems.clear();
	menuItems.push_back(TagPair("0","Выполнить разбивку диска"));
	if (systemConfig.swapPartition.empty()) menuItems.push_back(TagPair("1", "Выбрать раздел подкачки"));
	else menuItems.push_back(TagPair("1","Раздел подкачки: " + systemConfig.swapPartition));
	if (systemConfig.rootPartition.empty()) 
		menuItems.push_back(TagPair("2","Выбрать раздел для корневой файловой системы"));
	else menuItems.push_back(TagPair("2","Корневой раздел системы: " + systemConfig.rootPartition + \
				" (" + systemConfig.rootPartitionType + ", форматирование: " + doFormatString(systemConfig.rootPartitionFormat) + ")"));
	menuItems.push_back(TagPair("3","Настроить подключение других разделов"));
	if (systemConfig.sourceName.empty()) menuItems.push_back(TagPair("4","Выбор источника пакетов"));
	else menuItems.push_back(TagPair("4", "Источник пакетов: " + systemConfig.sourceName));
	
	menuItems.push_back(TagPair("5","Выбор устанавливаемых пакетов"));
	
	menuItems.push_back(TagPair("6","Установить систему"));
	
	menuItems.push_back(TagPair("7","Выход"));
	ret = d.execMenu("Установка MOPSLinux 6.0", 0,0,0,menuItems, next_item);
	if (ret == "0") if (diskPartitioningMenu()==0) next_item="1";
	if (ret == "1") if (setSwapSpace()==0) next_item="2";
	if (ret == "2") if (setRootPartition()==0) next_item="3";
	if (ret == "3") if (setOtherPartitions()==0) next_item="4";
	if (ret == "4") if (packageSourceSelectionMenu()==0) next_item="5";
	if (ret == "5") if (packageSelectionMenu()==0) next_item="6";
	if (ret == "6") if (commit()==0) next_item="7";
	if (ret == "7" || ret.empty()) return 0;
	goto main_menu;
}


