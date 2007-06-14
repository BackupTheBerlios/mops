/****************************************************
 * MOPSLinux: system setup (new generation)
 * $Id: setup.cpp,v 1.2 2007/06/14 08:02:40 i27249 Exp $
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

#include <mpkg/libmpkg.h>
#include <mpkg/dialog.h>
#include <parted/parted.h>

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
};

SysConfig systemConfig;

vector<pEntry> getGoodPartitions(vector<string> goodFSTypes)
{
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
	int x=0;
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
			while (!partFinished && x<10)
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
			if (partitionList[i][t]->num>0)
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
	return ret;
}
	
bool setPartitionMap()
{
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
	return true;
}

void showGreeting()
{
	Dialog dialogItem;
	dialogItem.execMsgBox("Добро пожаловать в MOPSLinux!\n\nПеред вами - экспериментальная версия, это значит что вместе с новыми \nдостоинствами возможно добавились и новые ошибки, но мы надеемся что все будет гораздо лучше. Удачи!");
}

void setSwapSpace()
{
	vector<string> gp;
	vector<pEntry> swapList = getGoodPartitions(gp);
	Dialog dialogItem;
	if (swapList.empty())
	{
		dialogItem.execMsgBox("No partitions found. Go partition your drives first");
	}
	vector<TagPair> sList;
	for (unsigned int i=0; i<swapList.size(); i++)
	{
		sList.push_back(TagPair(swapList[i].devname, swapList[i].fstype + " (" + swapList[i].size + "Mb)"));
	}
	string swapPartition;
selectSwapPartition:
	swapPartition = dialogItem.execMenu("Укажите раздел, который вы желаете использовать в качестве swap", 0,0,0,sList);
	if (swapPartition.empty())
	{
		if (!dialogItem.execYesNo("Продолжить без раздела подкачки?")) goto selectSwapPartition;
		else return;
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
			string swapMake = "mkswap " + swapList[i].devname;
			if (system(swapMake.c_str())!=0)
			{
				dialogItem.execMsgBox("При создании файла подкачки произошла ошибка, попробуйте другой раздел");
				goto selectSwapPartition;
			}
			else
			{
				systemConfig.swapPartition = swapList[i].devname;
				swapMake = "swapon " + swapList[i].devname;
				dialogItem.execInfoBox("Активация раздела подкачки...");
				if (system(swapMake.c_str())==0)
				{
					dialogItem.execInfoBox("Swap activated");
				}
				break;
			}
		
		}
	}
}

void setRootPartition()
{
	vector<string> gp;
	vector<pEntry> pList;
	pList= getGoodPartitions(gp);
	Dialog dialogItem;
	vector<TagPair> menuItems;
	for (unsigned int i=0; i<pList.size(); i++)
	{
		if (pList[i].fstype.find("linux-swap")==std::string::npos)
			menuItems.push_back(TagPair(pList[i].devname, pList[i].fstype + " (" + pList[i].size + "Mb)"));
	}
	string rootPartition;
selectRootPartition:
	rootPartition = dialogItem.execMenu("Укажите корневой раздел для MOPSLinux", 0,0,0,menuItems);
	if (rootPartition.empty())
	{
		if (dialogItem.execYesNo("Без корневого раздела установить систему нельзя. Вы действительно хотите выйти из установки?"))
		{
			abort();
		}
		else goto selectRootPartition;
	}
	systemConfig.rootPartition = rootPartition;
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
		if (dialogItem.execYesNo("Установка еще не завершена - выйти из программы?")) abort();
	}
	if (formatOption!="---")
	{
		string mkfs_cmd;
		if (formatOption!="xfs") mkfs_cmd = "mkfs." + formatOption + " " + rootPartition + " >> /var/install.log";
		else mkfs_cmd = "mkfs." + formatOption + " -f " + rootPartition + " >> /var/install.log";
		dialogItem.execInfoBox("Идет форматирование " + rootPartition + " в файловую систему " + formatOption + "\nЭто займет какое-то время");
		if (system(mkfs_cmd.c_str())!=0)
		{
			if (dialogItem.execYesNo("При форматировании произошла ошибка. Попробовать выбрать другой раздел?")) goto selectRootPartition;
			else abort();
		}
	}


}

void setOtherPartitions()
{
	vector<string> gp;
	vector<pEntry> pList = getGoodPartitions(gp);
	vector<TagPair> menuItems;
	vector<TagPair> mountPoints;
	for (unsigned int i=0; i<pList.size(); i++)
	{
		menuItems.push_back(TagPair(pList[i].devname, "(не подключено)"));

		mountPoints.push_back(TagPair(pList[i].devname, ""));
	}

	menuItems.push_back(TagPair("Продолжить", "Все готово, продолжить далее"));
	string editItem;
	string tmp_value;
	unsigned int currentItem;
	exec_menu:
	for (unsigned int i=0; i<pList.size(); i++)
	{

		if (pList[i].fstype.find("linux-swap")==std::string::npos && mountPoints[i].tag!=systemConfig.rootPartition)
		{
			menuItems[i] = TagPair(pList[i].devname + " (" + pList[i].fstype + ", "+pList[i].size+" Mb)" , mountPoints[i].value);
		}
	}

	Dialog dialogItem;
	editItem = dialogItem.execMenu("Выберите разделы, которые вы хотели бы подключить к системе", 0,0,0,menuItems);
	if (editItem.find_first_of("/")!=0)
	{
		for (unsigned int i=0; i<menuItems.size()-1; i++)
		{
			if (menuItems[i].value.find_first_of("/")==0)
			{
				systemConfig.otherMounts.push_back(TagPair(pList[i].devname, mountPoints[i].value));
			}
		}
		return;
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
}

void formatPartitions()
{
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
	menuItems.push_back("Продолжить", "Все разделы отформатированы как надо, продолжить");
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
	formatItem = dialogItem.execMenu("Выберите разделы, которые вы хотите отформатировать", 0,0,0,menuItems);
	if (formatItem.find_first_of("/")==std::string::npos)
	{
		return;
	}
	else
	{
		fsFormat = dialogItem.execMenu("В какую файловую систему отформатировать " + formatItem + "?", 0,0,0, formatOptions);
		if (fsFormat.empty() || fsFormat=="---") goto loadData;
		if (fsFormat!="xfs") mkfs_cmd = "mkfs." + fsFormat + " " + formatItem + " >> /var/install.log";
		else mkfs_cmd = "mkfs." + fsFormat + " -f " + formatItem + " >> /var/install.log";
		dialogItem.execInfoBox("Идет форматирование " + formatItem + " в файловую систему " + fsFormat + "\nЭто займет какое-то время");
		if (system(mkfs_cmd.c_str())!=0)
		{
			dialogItem.execMsgBox("При форматировании раздела " + formatItem + " произошла ошибка. Попробуйте другой тип файловой системы");
		}
		goto loadData;
	}
}

int mountPartitions()
{
	Dialog dialogItem;
	string mount_cmd;
	string mkdir_cmd;
	dialogItem.execInfoBox("Монтируется корневая файловая система ("+systemConfig.rootPartition + ")");

	mkdir_cmd = "mkdir -p " + systemConfig.rootMountPoint;
	mount_cmd = "mount " + systemConfig.rootPartition + " " + systemConfig.rootMountPoint;
	if (system(mkdir_cmd.c_str()) !=0 || system(mount_cmd.c_str())!=0)
	{
		dialogItem.execInfoBox("Произошла ошибка при монтировании корневой файловой системы.\nПроверьте всё и начните установку заново");
		abort();
	}
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		dialogItem.execInfoBox("Монтируются дополнительные файловые системы: " + systemConfig.otherMounts[i].tag + " ["+ \
				systemConfig.otherMounts[i].value+"]");
		mount_cmd = "mount -p " + systemConfig.otherMounts[i].tag + " " + systemConfig.otherMounts[i].value;
		if (system(mkdir_cmd.c_str())!=0 || system(mount_cmd.c_str())!=0)
		{
			dialogItem.execInfoBox("Произошла ошибка при монтировании файловой системы " + \
					systemConfig.otherMounts[i].tag + "\nПроверьте всё и начните установку заново");
			abort();
		}
	}
	return 0;
}

void autoInstall()
{
	mpkg core;
	Dialog dialogItem;
	vector<string> tags;
	core.get_available_tags(&tags);
	vector<TagPair> items;
	for (unsigned int i=0; i<tags.size(); i++)
	{
		items.push_back(TagPair(tags[i], tags[i]));
	}
	dialogItem.execCheckList("Выберите категории пакетов для установки", 0,0,0, &items);
	vector<string> installGroups;
	for (unsigned int i=0; i<items.size(); i++)
	{
		if (items[i].checkState) installGroups.push_back(items[i].tag);
	}
	core.installGroups(installGroups);
	core.commit();
	delete_tmp_files();

}
void manualInstall()
{
	Dialog dialogItem;
	vector<TagPair> pkgList;
	SQLRecord sqlSearch;
	PACKAGE_LIST packageList;
	core.get_packagelist(&sqlSearch, &packageList);
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
		core.uninstall(removeList);
		core.install(installList);
		core.commit();
	}
	else printf("Cancelled\n");
}

void initDatabaseStructure()
{
	string cmd;
	cmd = "mkdir -p " systemConfig.rootMountPoint+"/var/mpkg/scripts";
	system(cmd);
	cmd = "mkdir -p " systemConfig.rootMountPoint+"/var/mpkg/backup";
	system(cmd);
	cmd = "mkdir -p " systemConfig.rootMountPoint+"/var/mpkg/cache";
	system(cmd);
	system(cmd);
	cmd = "ln -sf " systemConfig.rootMountPoint+"/var/mpkg /var/mpkg";
	system(cmd);
	cmd = "cp -f /packages.db /var/mpkg/";
	system(cmd);
}





	
void selectInstallMethod()
{
	initDatabaseStructure();
	Dialog dialogItem;
	vector<string> options;
	options.push_back("Автоматический режим: выбор пакетов по группам");
	options.push_back("Экспертный режим: индивидуальный выбор пакетов");
	int opt = dialogItem.execMenu("Выберите метод установки:", options);
	if (opt<0) abort();
       	if (opt==0) autoInstall();
	else manualInstall();
}

void performConfig()
{

}

int main()
{
	systemConfig.rootMountPoint="/mnt";
	showGreeting();
	setSwapSpace();
	setRootPartition();
	setOtherPartitions();
	formatPartitions();
	selectInstallMethod();
	performConfig();
	return 0;
}

