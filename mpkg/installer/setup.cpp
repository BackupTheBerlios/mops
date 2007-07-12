/****************************************************
 * MOPSLinux: system setup (new generation)
 * $Id: setup.cpp,v 1.15 2007/07/12 09:28:12 i27249 Exp $
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
	
bool setPartitionMap()
{
	mDebug("start");
	vector<string> gp;
       	gp.push_back("ext2");
       	gp.push_back("ext3");
	gp.push_back("xfs");
	gp.push_back("jfs");
       	gp.push_back("reiserfs");
	vector<pEntry> pList = getGoodPartitions(gp);
	for (unsigned int i=0; i<pList.size(); i++)
	{
	}
	mDebug("end");
	return true;
}

void showGreeting()
{
	mDebug("start");
	Dialog dialogItem;
	dialogItem.execMsgBox("Добро пожаловать в MOPSLinux!\n\nПрограмма установки выполнит установку системы на ваш компьютер\nи проведет первоначальную конфигурацию.\n");
	mDebug("end");
}

int setSwapSpace()
{
	mDebug("start");
	vector<string> gp;
	vector<pEntry> swapList = getGoodPartitions(gp);
	Dialog dialogItem;
	if (swapList.empty())
	{
		mDebug("no partitions detected");
		dialogItem.execMsgBox("No partitions found. Go partition your drives first");
	}
	vector<TagPair> sList;
	mDebug("filling options list");
	for (unsigned int i=0; i<swapList.size(); i++)
	{
		sList.push_back(TagPair(swapList[i].devname, swapList[i].fstype + " (" + swapList[i].size + "Mb)"));
	}
	string swapPartition;
selectSwapPartition:
	swapPartition = dialogItem.execMenu("Укажите раздел, который вы желаете использовать в качестве swap", 0,0,0,sList);
	if (swapPartition.empty())
	{
		//if (!dialogItem.execYesNo("Продолжить без раздела подкачки?")) goto selectSwapPartition;
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
			// Confirmed:
			/*string swapoff="swapoff " + swapList[i].devname;
			system(swapoff.c_str());
			string swapMake = "mkswap " + swapList[i].devname;
			if (system(swapMake.c_str())!=0)
			{
				mDebug("error creating swap on " + swapList[i].devname);
				dialogItem.execMsgBox("При создании файла подкачки произошла ошибка, попробуйте другой раздел");
				goto selectSwapPartition;
			}
			else
			{
				mDebug("swap created on " + swapList[i].devname);*/
			systemConfig.swapPartition = swapList[i].devname;

/*			swapMake = "swapon " + swapList[i].devname;
				dialogItem.execInfoBox("Активация раздела подкачки...");
				if (system(swapMake.c_str())==0)
				{
					dialogItem.execInfoBox("Swap activated");
				}
				break;
			}*/
		
		}
	}
	mDebug("end");
	return 0;
}

int setRootPartition()
{
	mDebug("start");
	vector<string> gp;
	vector<pEntry> pList;
	pList= getGoodPartitions(gp);
	Dialog dialogItem;
	vector<TagPair> menuItems;
	for (unsigned int i=0; i<pList.size(); i++)
	{
		//if (pList[i].fstype.find("linux-swap")==std::string::npos)
			menuItems.push_back(TagPair(pList[i].devname, pList[i].fstype + " (" + pList[i].size + "Mb)"));
	}
	string rootPartition;
selectRootPartition:
	rootPartition = dialogItem.execMenu("Укажите корневой раздел для MOPSLinux", 0,0,0,menuItems);
	if (rootPartition.empty())
	{
		mDebug("nothing selected");
		/*if (dialogItem.execYesNo("Без корневого раздела установить систему нельзя. Вы действительно хотите выйти из установки?"))
		{
			mDebug("aborted");
			abort();
		}
		else goto selectRootPartition;*/
		return -1;
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
		//if (dialogItem.execYesNo("Установка еще не завершена - выйти из программы?")) { mDebug("aborted"); abort(); }
	}
	systemConfig.rootPartition = rootPartition;

	if (formatOption!="---")
	{
		systemConfig.rootPartitionType = formatOption;
		mDebug("set to format " + rootPartition + " as " + formatOption);
		umount(rootPartition.c_str());
		string mkfs_cmd;
		if (formatOption!="xfs") mkfs_cmd = "mkfs." + formatOption + " " + rootPartition + " 2>> /var/install.log";
		else mkfs_cmd = "mkfs." + formatOption + " -f " + rootPartition + " 2>> /var/install.log";
		mDebug("mkfs_cmd = " + mkfs_cmd);
		dialogItem.execInfoBox("Идет форматирование " + rootPartition + " в файловую систему " + formatOption + "\nЭто займет какое-то время");
		/*if (system(mkfs_cmd.c_str())!=0)
		{
			mDebug("Failed to format " + rootPartition + " as " + formatOption);
			if (dialogItem.execYesNo("При форматировании произошла ошибка. Попробовать выбрать другой раздел?")) goto selectRootPartition;
			else abort();
		}
		else
		{
			mDebug("Root FS Format ok");
		}*/
	}
	mDebug("end");
	return 0;

}

int setOtherPartitions()
{
	mDebug("start");
	vector<string> gp;
	vector<pEntry> pList_raw = getGoodPartitions(gp);
	vector<TagPair> menuItems;
	vector<TagPair> mountPoints;
	vector<pEntry> pList;
	for (unsigned int i=0; i<pList_raw.size(); i++)
	{
		if (pList_raw[i].fstype.find("linux-swap")==std::string::npos && pList_raw[i].devname.find(systemConfig.rootPartition)==std::string::npos)
		{
			pList.push_back(pList_raw[i]);

		}
	}
	pList_raw.clear();
	if (pList.empty())
	{
		mDebug("no other partitions - skipping");
		return 0;
	}
	for (unsigned int i=0; i<pList.size(); i++)
	{
		menuItems.push_back(TagPair(pList[i].devname, "(не подключено)"));

		/*for (unsigned int t=0; t<systemConfig.otherMounts.size(); t++)
		{
			if (systemConfig.otherMounts[t].tag==pList[i].devname) mountPoints.push_back(systemConfig.otherMounts[t]);
			else if (t==systemConfig.otherMounts.size()-1)*/	mountPoints.push_back(TagPair(pList[i].devname, ""));
		//}
	}

	menuItems.push_back(TagPair("Продолжить", "Все готово, продолжить далее"));
	string editItem;
	string tmp_value;
	unsigned int currentItem=0;
	exec_menu:
	//mountPoints=systemConfig.otherMounts;
	for (unsigned int i=0; i<pList.size(); i++)
	{
		menuItems[i] = TagPair(pList[i].devname + " (" + pList[i].fstype + ", "+pList[i].size+" Mb)" , mountPoints[i].value);
	}

	Dialog dialogItem;
	editItem = dialogItem.execMenu("Выберите разделы, которые вы хотели бы подключить к системе", 0,0,0,menuItems);
	if (editItem.find_first_of("/")!=0)
	{
		for (unsigned int i=0; i<menuItems.size()-1; i++)
		{
			if (menuItems[i].value.find_first_of("/")==0)
			{
				mDebug("added " + pList[i].devname + " to mount at " + mountPoints[i].value);
				systemConfig.otherMounts.push_back(TagPair(pList[i].devname, mountPoints[i].value));
				systemConfig.otherMountFSTypes.push_back(pList[i].fstype);
			}
		}
		mDebug("end");
		return 0;
	}
	else
	{
		for (unsigned int i=0; i<menuItems.size()-1; i++)
		{
			if (editItem==pList[i].devname)
			{
				currentItem=i;
				break;
			}
		}
		tmp_value = dialogItem.execInputBox("Выберите точку подключения для " + editItem, mountPoints[currentItem].value);
		mountPoints[currentItem].value=tmp_value;
		goto exec_menu;
	}
	return 0;
}

int formatPartitions()
{
	mDebug("start");
	vector<string> gp;
	Dialog dialogItem;
	vector<pEntry> pList;
	vector<string>formatList;
	vector<TagPair> menuItems;
	string formatItem;
	string fsFormat;
	vector<TagPair> formatOptions;
	formatOptions.push_back(TagPair("ext3", "Стандартная журналируемая файловая система Linux."));
	formatOptions.push_back(TagPair("ext2", "Стандартная файловая система Linux (без журналирования)."));
	formatOptions.push_back(TagPair("xfs", "XFS - быстрая файловая система, изначально разработанная SGI для IRIX (мин. 16Мб)"));
	formatOptions.push_back(TagPair("jfs", "Журналируемая файловая система, разработанная IBM (мин. 16Мб)"));
	formatOptions.push_back(TagPair("reiserfs", "Файловая система ReiserFS, хранит данные в сбалансированном дереве (мин. 32Мб)"));
	formatOptions.push_back(TagPair("---", "Не форматировать - оставить как есть, cохранив данные"));

	string mkfs_cmd;
loadData:
	pList = getGoodPartitions(gp);
	menuItems.clear();
	menuItems.push_back(TagPair("Продолжить", "Все разделы отформатированы как надо, продолжить"));
	for (unsigned int i=0; i<pList.size(); i++)
	{
		for (unsigned int t=0; t<systemConfig.otherMounts.size(); t++)
		{
			if (pList[i].devname==systemConfig.otherMounts[t].tag)
			{
				menuItems.push_back(TagPair(pList[i].devname + " (" + pList[i].size + " Mb) ["+systemConfig.otherMounts[t].value+"]", pList[i].fstype));
			}
		}
	}
	if (menuItems.size()==1)
	{
		mDebug("nothing to format");
		mDebug("end");
		return 0;
	}
	formatItem = dialogItem.execMenu("Выберите разделы, которые вы хотите отформатировать", 0,0,0,menuItems);
	if (formatItem.find_first_of("/")==std::string::npos)
	{
		mDebug("all done, returning");
		return 0;
	}
	else
	{
		fsFormat = dialogItem.execMenu("В какую файловую систему отформатировать " + formatItem + "?", 0,0,0, formatOptions);
		if (fsFormat.empty() || fsFormat=="---") goto loadData;
		if (fsFormat!="xfs") mkfs_cmd = "mkfs." + fsFormat + " " + formatItem + " >> /var/install.log";
		else mkfs_cmd = "mkfs." + fsFormat + " -f " + formatItem + " >> /var/install.log";
		dialogItem.execInfoBox("Идет форматирование " + formatItem + " в файловую систему " + fsFormat + "\nЭто займет какое-то время");
		umount(formatItem.c_str());
		if (system(mkfs_cmd.c_str())!=0)
		{
			dialogItem.execMsgBox("При форматировании раздела " + formatItem + " произошла ошибка. Попробуйте другой тип файловой системы");
		}
		goto loadData;
	}
	mDebug("end");
}

int mountPartitions()
{
	mDebug("start");
	Dialog dialogItem;
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
	
	// Sorting mount points
	vector<unsigned int> mountOrder, mountPriority;
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
		}
	}
	for (unsigned int i=0; i<mountPriority.size(); i++)
	{
		for (unsigned int t=0; t<mountPriority.size(); t++)
		{
			if (mountPriority[t]==i)
			{
				mountOrder.push_back(t);
			}
		}
	}
	if (mountOrder.size()!=systemConfig.otherMounts.size()) 
	{
		mError("mount code error, size mismatch: order size = " + \
			IntToStr(mountOrder.size()) + \
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

int autoInstall()
{
	mDebug("start");
	if (core==NULL) core = new mpkg;
	Dialog dialogItem;
	vector<string> tags;
	core->get_available_tags(&tags);
	vector<TagPair> items;
	for (unsigned int i=0; i<tags.size(); i++)
	{
		items.push_back(TagPair(tags[i], tags[i]));
	}
selectGroups:
	if (!dialogItem.execCheckList("Выберите категории пакетов для установки", 0,0,0, &items))
	{
		if (dialogItem.execYesNo("Прервать установку?")) abort();
		else goto selectGroups;
	}
	vector<string> installGroups;
	for (unsigned int i=0; i<items.size(); i++)
	{
		if (items[i].checkState) installGroups.push_back(items[i].tag);
	}

	dialogItem.execInfoBox("Формирование очереди, ждите...");
	core->installGroups(installGroups);
	/*int ret = core->commit();
	if (ret!=0)
	{
		switch(ret)
		{
			case MPKGERROR_UNRESOLVEDDEPS:
				dialogItem.execMsgBox("Неразрешенные зависимости. Это проблема сборки дистрибутива, обратитесь к разработчикам");
				break;
			case MPKGERROR_SQLQUERYERROR:
				dialogItem.execMsgBox("Произошла внутренняя ошибка в базе пакетов.\nЭто означает, что либо возникли проблемы с базой данных, либо это ошибка в программе");
				break;
			case MPKGERROR_ABORTED:
				dialogItem.execMsgBox("Установка отменена");
				break;
			case MPKGERROR_COMMITERROR:
				dialogItem.execMsgBox("Ошибка при установке пакетов. Чтобы узнать подробности, смотрите /var/mpkg/errors.log");
				break;
			default:
				dialogItem.execMsgBox("Произошла неизвестная ошибка, core.commit() вернуло " + IntToStr(ret));
		}
	}*/
	delete_tmp_files();

	delete core;
	core=NULL;
	mDebug("end");
	return 0;
}

string getTagDescription(string tag)
{
	return tag;
}

int manualInstall()
{
	mDebug("start");
	Dialog dialogItem;
	vector<TagPair> pkgList;
	SQLRecord sqlSearch;
	PACKAGE_LIST packageList;
	if (core==NULL) core = new mpkg;
	core->get_packagelist(&sqlSearch, &packageList);
	for (int i=0; i<packageList.size(); i++)
	{
		pkgList.push_back(TagPair(*packageList.get_package(i)->get_name(),\
				       *packageList.get_package(i)->get_short_description(),\
				       packageList.get_package(i)->installed()));
	}
	vector<TagPair> oldState = pkgList;
	if (dialogItem.execCheckList("Выберите пакеты для установки", 0,0,0, &pkgList))
	{
		vector<string> removeList;
		vector<string> installList;
		for (unsigned int i=0; i<oldState.size(); i++)
		{
			if (oldState[i].checkState!=pkgList[i].checkState)
			{
				if (pkgList[i].checkState) installList.push_back(pkgList[i].tag);
				else removeList.push_back(pkgList[i].tag);
			}
		}
		printf("Install list:\n");
		for (unsigned int i=0; i<installList.size(); i++)
		{
			printf("INSTALL: %s\n", installList[i].c_str());
		}
		for (unsigned int i=0; i<removeList.size(); i++)
		{
			printf("REMOVE: %s\n", removeList[i].c_str());
		}
		core->uninstall(removeList);
		core->install(installList);
		/*int ret = core.commit();
		if (ret!=0)
		{
			switch(ret)
			{
				case MPKGERROR_UNRESOLVEDDEPS:
					dialogItem.execMsgBox("Неразрешенные зависимости. Это проблема сборки дистрибутива, обратитесь к разработчикам");
					break;
				case MPKGERROR_SQLQUERYERROR:
					dialogItem.execMsgBox("Произошла внутренняя ошибка в базе пакетов.\nЭто означает, что либо возникли проблемы с базой данных, либо это ошибка в программе");
					break;
				case MPKGERROR_ABORTED:
					dialogItem.execMsgBox("Установка отменена");
					break;
				case MPKGERROR_COMMITERROR:
					dialogItem.execMsgBox("Ошибка при установке пакетов. Чтобы узнать подробности, смотрите /var/mpkg/errors.log");
					break;
				default:
					dialogItem.execMsgBox("Произошла неизвестная ошибка, core.commit() вернуло " + IntToStr(ret));
			}
		}*/
		delete_tmp_files();

		return 0;

	}
	else printf("Cancelled\n");
	mDebug("end");
	return MPKGERROR_ABORTED;
}



void initDatabaseStructure()
{
	// Should be remaked to use RAM while no HDD is mounted.
	mDebug("start");
	mDebug("Processing directories");
	string cmd;
	cmd = "rm -rf /var/mpkg 2>>/var/install.log";
	mDebug(cmd);
	system(cmd.c_str());
	cmd = "ln -sf " +systemConfig.rootMountPoint+"/var/mpkg /var/mpkg 2>>/var/install.log";
	mDebug(cmd);
	system(cmd.c_str());
	cmd = "mkdir -p " + systemConfig.rootMountPoint+"/var/mpkg/scripts 2>>/var/install.log";
	mDebug(cmd);
	system(cmd.c_str());
	cmd = "mkdir -p "+systemConfig.rootMountPoint+"/var/mpkg/backup 2>>/var/install.log";
	mDebug(cmd);
	system(cmd.c_str());
	cmd = "mkdir -p " +systemConfig.rootMountPoint+"/var/mpkg/cache 2>>/var/install.log";
	mDebug(cmd);
	system(cmd.c_str());
	cmd = "cp -f /packages.db /var/mpkg/ 2>> /var/install.log";
	mDebug(cmd);
	system(cmd.c_str());
	mDebug("Creating mpkg object");
	//packageSourceSelection();
	mDebug("end");
}


void mountMedia()
{
	mDebug("start");
	if (umount("/var/log/mount")==0) mDebug("successfully unmounted old mount");
	else mDebug("cd wasn't mounted or unmount error. Processing to detection");
	Dialog dialogItem;
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
			return;
		}
	}
	mDebug("Drive not found. Proceeding to manual selection");
	// Failed? Select manually please
	if (!dialogItem.execYesNo("Автоопределение привода не удалось. Хотите указать вручную?"))
	{
		mDebug("aborted");
		abort();
	}
manual:
	mDebug("manual selection");
	string manualMount = dialogItem.execInputBox("Укажите вручную имя устройства (например /dev/hda)", "/dev/");
	if (manualMount.empty())
	{
		if (dialogItem.execYesNo("Вы ничего не указали. Выйти из программы установки?")) { mDebug("aborted"); abort(); }
		cmd = "mount -t iso9660 " + manualMount + " /var/log/mount 2>> /var/install.log";
		if (system(cmd.c_str())!=0)
		{
			mDebug("manual mount error");
			dialogItem.execMsgBox("Не удалось смонтировать указанный вами диск");
			goto manual;
		}
	}
	mDebug("end");
}


	
int selectInstallMethod()
{
	mDebug("start");
	initDatabaseStructure();
	Dialog dialogItem;
	vector<string> options;
	options.push_back("Автоматический режим: выбор пакетов по группам");
	options.push_back("Экспертный режим: индивидуальный выбор пакетов");
	int opt = dialogItem.execMenu("Выберите метод установки:", options);
	if (opt<0)
	{
		mDebug("aborted");
		abort();
	}
       	if (opt==0)
	{
		mDebug("Auto install selected");
		return autoInstall();
	}
	else
	{
		mDebug("Manual install selected");
		return manualInstall();
	}
}
bool showLicense()
{
	Dialog d;
	if (system((string) "dialog --extra-button --extra-label \"Не принимаю\" --no-cancel --ok-label \"Принимаю\"	--textbox license 0 0")==0) return true;
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
	vector<unsigned int> mountOrder, mountPriority;
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
		}
	}
	for (unsigned int i=0; i<mountPriority.size(); i++)
	{
		for (unsigned int t=0; t<mountPriority.size(); t++)
		{
			if (mountPriority[t]==i)
			{
				mountOrder.push_back(t);
			}
		}
	}
	if (mountOrder.size()!=systemConfig.otherMounts.size()) 
	{
		mError("mount code error, size mismatch: order size = " + \
			IntToStr(mountOrder.size()) + \
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
	
	WriteFile(systemConfig.rootMountPoint + "/etc/fstab", data);
	//WriteFile("fstab", data);
	mDebug("end");
}



void performConfig()
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
	else mDebug("Config seems to be failed");
	mDebug("end");
}

void syncFS()
{
	mDebug("start");
	system("sync");
	umount(systemConfig.cdromDevice.c_str());
	mDebug("end");
}

void showFinish()
{
	mDebug("start");
	Dialog dialogItem;
	dialogItem.execMsgBox("Установка завершена успешно! Перезагрузитесь и наслаждайтесь!");
	mDebug("finish");
	exit(0);
}

void packageSourceSelection()
{
	Dialog dialogItem;
	vector<string> nullList, rList;
start:
	core = new mpkg;
       	rList	= REPOSITORY_LIST;
	dialogItem.execAddableList("Сформируйте список репозиториев\nЭто должны быть URL стандартного вида, указывающие на корень репозитория\nи заканчивающиеся символом /\nДоступные типы: ftp://, http://, file://, cdrom://\nДля установки с этого диска рекомендуем использовать file:///var/log/mount/mops/", &rList, "://");
	// Redefine repository list
	if (rList.empty())
	{
		if (dialogItem.execYesNo("Вы не указали ни одного источника пакетов. Использовать список по умолчанию?"))
		{
			rList.push_back("file:///var/log/mount/mops/");
		}
	}
	dialogItem.execInfoBox("Обновление списка пакетов...");
	core->set_repositorylist(rList, nullList);
	if (core->update_repository_data()!=0)
	{
		mDebug("update failed");
		Dialog dialogItem;
		delete core;
		if (dialogItem.execYesNo("При создании начальной базы пакетов произошла ошибка. Проверьте доступность источников либо измените список."))
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
}

int main_old(int argc, char **argv)
{
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
	mDebug("Main start");
	systemConfig.rootMountPoint="/mnt";
	showGreeting();
	setSwapSpace();
	setRootPartition();
	setOtherPartitions();
	formatPartitions();
	mountMedia();
	mountPartitions();
	if (selectInstallMethod()!=0)
	{
		mDebug("Installation failed");
		return 0;
	}
	performConfig();
	syncFS();
	showFinish();
	mDebug("Installation seems to be OK");
	return 0;
}

int packageSelectionMenu()
{
	Dialog d;
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
	core->clean_queue();
	core->get_packagelist(&sqlSearch, &i_availablePackages);
	core->get_available_tags(&i_tagList);
	
	// Predefined groups preselect
	for (unsigned int i=0; i<i_availablePackages.size(); i++)
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
	
	ret = d.execMenu("Вы выбрали тип установки " + ins_type + "\nОтредактируйте список пакетов и нажмите Готово",0,0,0,menuItems);
	if (ret == "Готово") return 0;
	else
	{
		menuItems.clear();
		for (unsigned int i=0; i<i_availablePackages.size(); i++)
		{
			p = i_availablePackages.get_package(i);
			if (p->isTaggedBy(ret))
			{
				menuItems.push_back(TagPair(IntToStr(p->get_id()), *p->get_name() + " (" + *p->get_short_description() + ")", p->action()==ST_INSTALL));
			}
		}
		if (d.execCheckList("Выберите пакеты для установки (группа " + ret + ")",0,0,0, &menuItems))
		{
			for (unsigned int i=0; i<menuItems.size(); i++)
			{
				for (unsigned int t=0; t<i_availablePackages.size(); t++)
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
	Dialog d;
	string summary;
	summary = "Корневой раздел: " + systemConfig.rootPartition + ", " + systemConfig.rootPartitionType + "\n" + \
		   "Раздел подкачки: " + systemConfig.swapPartition + "\n" + \
		   "Другие разделы: ";
	if (systemConfig.otherMounts.empty()) summary += "нет\n";
	else
	{
		for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
		{
			summary += "                " + systemConfig.otherMounts[i].tag + " (" + systemConfig.otherMounts[i].value + "), форматирование: " + systemConfig.otherMountFSTypes[i] + "\n";
		}
	}
	summary += "Источник пакетов: " + systemConfig.sourceName + "\n" + \
		    "\n\nВсе в порядке, можно выполнять установку?";
	if (d.execYesNo(summary))
	{
		if (!simulate)
		{
			formatPartitions();
			mountMedia();
			mountPartitions();
			if (core==NULL) core = new mpkg;
			core->commit();
			syncFS();
			performConfig();
		}
		showFinish();
	}
	return 0;
}


int setDVDSource()
{
	Dialog dialogItem;
	vector<string> nullList, rList;
start:
	core = new mpkg;
	rList.push_back("file:///var/log/mount/mops/");

	mountMedia();
	dialogItem.execInfoBox("Обновление списка пакетов...");
	core->set_repositorylist(rList, nullList);
	if (core->update_repository_data()!=0)
	{
		mDebug("update failed");
		Dialog dialogItem;
		delete core;
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
	systemConfig.sourceName="DVD";
	return 0;
}

int setHDDSource()
{
	Dialog d;
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
			d.execMsgBox("При загрузке списка пакетов произошла ошибка. Проверьте корректность источникa.");
			goto enter_path;
		}
		else
		{
			mDebug("ok");
		}
		delete core;
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
	Dialog d;
	vector<string> nullList, rList;
	string ret;
start:
	core = new mpkg;
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
	return 0;
}

int packageSourceSelectionMenu()
{

	Dialog d;
	vector<TagPair> menuItems;
	string ret;

	menuItems.clear();
	menuItems.push_back(TagPair("DVD-ROM", "Установка с привода DVD"));
	menuItems.push_back(TagPair("HDD", "Установка с жесткого диска"));
	menuItems.push_back(TagPair("Network", "Установка по сети (FTP, HTTP)"));
	menuItems.push_back(TagPair("Назад", "Выйти в главное меню"));
	ret = d.execMenu("Выберите источник пакетов для установки системы", 0,0,0,menuItems);
	if (ret == "DVD-ROM") setDVDSource();
	if (ret == "HDD") setHDDSource();
	if (ret == "Network") setNetworkSource();
	return 0;
}
	
int diskPartitioningMenu()
{
	Dialog d;
	vector<TagPair> menuItems;
	string ret;
	int r = 0;
part_menu:
	menuItems.clear();
	menuItems.push_back(TagPair("cfdisk", "Простое меню для разбивки"));
	//menuItems.push_back(TagPair("fdisk", "Разбивка диска из командной строки (экспертный режим)"));
	menuItems.push_back(TagPair("parted", "Разбивка диска из командной строки (экспертный режим)"));
	menuItems.push_back(TagPair("Готово", "Все готово, вернуться в главное меню"));

	ret = d.execMenu("Выберите способ разбивки диска",0,0,0,menuItems);
	if (ret == "Готово" || ret.empty()) return 0;
	if (ret == "cfdisk") r = system("cfdisk");
	//if (ret == "fdisk") r = system("fdisk");	// Надо явно указывать какой диск разбивать. Сделаю потом.
	if (ret == "parted") r = system("parted");
	if (r==0) return 0;
	else goto part_menu;
}

int main(int argc, char *argv[])
{
	simulate=true;
	if (argc>=2)
	{
		if (strcmp(argv[1], "--simulate")==0)
		{
			simulate=false;
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

	showGreeting();
	if (!showLicense()) return -1;
	/*else {
		printf("Accepted\n");
		sleep(2);
	}*/

	systemConfig.rootMountPoint="/mnt";

	Dialog d;
	string ret;
	string next_item;
	vector<TagPair> menuItems;

	initDatabaseStructure();
main_menu:
	menuItems.clear();
	menuItems.push_back(TagPair("0","Выполнить разбивку диска"));
	menuItems.push_back(TagPair("1","Выбрать раздел подкачки [" + systemConfig.swapPartition + "]"));
	menuItems.push_back(TagPair("2","Выбрать корневой раздел системы [" + systemConfig.rootPartition + "]"));
	menuItems.push_back(TagPair("3","Настроить подключение других разделов"));
	menuItems.push_back(TagPair("4","Выбор источника пакетов ["+systemConfig.sourceName+"]"));
	menuItems.push_back(TagPair("5","Выбор устанавливаемых пакетов"));
	//menuItems.push_back(TagPair("6","Настройка сервисов"));
	//menuItems.push_back(TagPair("7","Настройка сети"));
	//menuItems.push_back(TagPair("8","Настройка временной зоны"));
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


