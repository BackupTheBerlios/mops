/****************************************************
 * MOPSLinux: system setup (new generation)
 * $Id: setup.cpp,v 1.5 2007/06/14 20:36:21 i27249 Exp $
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
	string rootMountPoint;
	string cdromDevice;
};

SysConfig systemConfig;

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
	dialogItem.execMsgBox("Добро пожаловать в MOPSLinux!\n\nПеред вами - экспериментальная версия, это значит что вместе с новыми \nдостоинствами возможно добавились и новые ошибки, но мы надеемся что все будет гораздо лучше. Удачи! Debug release!");
	mDebug("end");
}

void setSwapSpace()
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
			string swapoff="swapoff " + swapList[i].devname;
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
				mDebug("swap created on " + swapList[i].devname);
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
	mDebug("end");
}

void setRootPartition()
{
	mDebug("start");
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
		mDebug("nothing selected");
		if (dialogItem.execYesNo("Без корневого раздела установить систему нельзя. Вы действительно хотите выйти из установки?"))
		{
			mDebug("aborted");
			abort();
		}
		else goto selectRootPartition;
	}
	mDebug("selecting FS type");
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
		if (dialogItem.execYesNo("Установка еще не завершена - выйти из программы?")) { mDebug("aborted"); abort(); }
	}
	if (formatOption!="---")
	{
		mDebug("formatting " + rootPartition + " as " + formatOption);
		umount(rootPartition.c_str());
		string mkfs_cmd;
		if (formatOption!="xfs") mkfs_cmd = "mkfs." + formatOption + " " + rootPartition + " 2>> /var/install.log";
		else mkfs_cmd = "mkfs." + formatOption + " -f " + rootPartition + " 2>> /var/install.log";
		mDebug("mkfs_cmd = " + mkfs_cmd);
		dialogItem.execInfoBox("Идет форматирование " + rootPartition + " в файловую систему " + formatOption + "\nЭто займет какое-то время");
		if (system(mkfs_cmd.c_str())!=0)
		{
			mDebug("Failed to format " + rootPartition + " as " + formatOption);
			if (dialogItem.execYesNo("При форматировании произошла ошибка. Попробовать выбрать другой раздел?")) goto selectRootPartition;
			else abort();
		}
		else
		{
			mDebug("Root FS Format ok");
		}
	}
	mDebug("end");

}

void setOtherPartitions()
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
		return;
	}
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
			}
		}
		mDebug("end");
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
		return;
	}
	formatItem = dialogItem.execMenu("Выберите разделы, которые вы хотите отформатировать", 0,0,0,menuItems);
	if (formatItem.find_first_of("/")==std::string::npos)
	{
		mDebug("all done, returning");
		return;
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
	else mDebug("mkdir and mount OK");
	for (unsigned int i=0; i<systemConfig.otherMounts.size(); i++)
	{
		dialogItem.execInfoBox("Монтируются дополнительные файловые системы: " + systemConfig.otherMounts[i].tag + " ["+ \
				systemConfig.otherMounts[i].value+"]");
		mkdir_cmd = "mkdir -p " + systemConfig.rootMountPoint+systemConfig.otherMounts[i].value;
		mount_cmd = "mount " + systemConfig.otherMounts[i].tag + " " + systemConfig.rootMountPoint+systemConfig.otherMounts[i].value;

		mDebug("Attempting to mkdir: " + mkdir_cmd);
	       	mDebug("and Attempting to mount: " + mount_cmd);	
		if (system(mkdir_cmd.c_str())!=0 || system(mount_cmd.c_str())!=0)
		{
			mDebug("error while mount");
			dialogItem.execInfoBox("Произошла ошибка при монтировании файловой системы " + \
					systemConfig.otherMounts[i].tag + "\nПроверьте всё и начните установку заново");
			abort();
		}
		else mDebug("mount ok");
	}
	mDebug("end");
	return 0;
}

int autoInstall()
{
	mDebug("start");
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
	int ret = core.commit();
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
	}
	delete_tmp_files();

	mDebug("end");
	return ret;
}
int manualInstall()
{
	mDebug("start");
	Dialog dialogItem;
	vector<TagPair> pkgList;
	SQLRecord sqlSearch;
	PACKAGE_LIST packageList;
	mpkg core;
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
		int ret = core.commit();
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
		}
		delete_tmp_files();

		return ret;

	}
	else printf("Cancelled\n");
	mDebug("end");
	return MPKGERROR_ABORTED;
}



void initDatabaseStructure()
{
	mDebug("start");
	mpkg *core;
beg:
	mDebug("Processing directories");
	string cmd;
	cmd = "rm -rf " + systemConfig.rootMountPoint+"/var/mpkg 2>>/var/install.log";
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
	core = new mpkg;
	mDebug("updating database...");
	
	if (core->update_repository_data()!=0)
	{
		mDebug("failed");
		Dialog dialogItem;
		delete core;
		if (dialogItem.execYesNo("При создании начальной базы пакетов произошла ошибка. Проверьте носители."))	goto beg;
		else abort();
	}
	else
		mDebug("ok");
	mDebug("end");
}


void mountMedia()
{
	mDebug("start");
	if (umount("/var/log/mount")==0) mDebug("successfully unmounted old mount");
	else mDebug("cd wasn't mounted or unmount error. Processing to detection");
	Dialog dialogItem;
	dialogItem.execInfoBox("Попытка автоматического определения DVD-ROM...");
	int ret=-1;
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
	string manualMount = dialogItem.execInputBox("Укажите вручную имя устройства (например /dev/sda)", "/dev/");
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

void performConfig()
{
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
}

int main()
{
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

