#include "HttpDownload.h"

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#ifndef INTERNAL_MOUNT
#include <sys/mount.h>
#endif
#include <mntent.h>

string DL_CDROM_DEVICE="/dev/cdrom";
string DL_CDROM_MOUNTPOINT="/mnt/cdrom";

HttpDownload::HttpDownload()
{
	ch = curl_easy_init();
}

	double *extDlNow;
	double *extDlTotal;
	double *extItemTotal;
	double *extItemNow;

static int downloadCallback(void *clientp,
                       double dltotal,
                       double dlnow,
                       double ultotal,
                       double ulnow)
{
	*extDlTotal = dltotal;
	*extDlNow = dlnow;
	return 0;
}

int fileLinker(std::string source, std::string output)
{
	if(access(source.c_str(), R_OK)!=0) return -1;
	
	std::string execLine = "ln -sf "+source+" "+output;
	return system(execLine.c_str());
}



int cdromFetch(std::string source, std::string output) // Caching of files from CD-ROM devices. URL format: cdrom://CDROM_UUID/directory/filename.tgz
{
	//printf("fetching from CDROM %s\n", DL_CDROM_DEVICE.c_str());
	// input format:
	// source:
	// 	CDROM_VOLNAME/dir/fname.tgz
	//
	// Scheme:
	// 1. Check if device mounted. If not, mount it.
	// 2. Check media UUID. If ok, process to caching
	// 2a. If UUID is wrong, eject and request valid media
	// 3. Copy requested packages from CD-ROM media to cache.
	// 4. If required next media, eject current and require next disk, next go to 1
	// 5. If all required media is processed, eject the last and return.
	int mount_ret;
	int umount_ret;
	// First, check if device mounted in proper directory. 
	struct mntent *mountList;
	FILE *mtab = fopen("/proc/mounts", "r");
	bool mounted = false;
	char volname[2000];
	string cdromVolName = source.substr(0,source.find_first_of("/")-1);
	string sourceFileName = DL_CDROM_MOUNTPOINT + source.substr(source.find_first_of("/"));
	mpkgErrorReturn errRet;
	if (mtab)
	{
		mountList = getmntent(mtab);
		while ( !mounted && mountList != NULL )
		{
			if (strcmp(mountList->mnt_fsname, DL_CDROM_DEVICE.c_str())==0)
			{
				if (strcmp(mountList->mnt_dir, DL_CDROM_MOUNTPOINT.c_str())!=0)
				{
					umount(DL_CDROM_MOUNTPOINT.c_str());
				}
				else mounted = true;
			}
			mountList = getmntent(mtab);
		}
		fclose(mtab);
	}
	if (!mounted)
	{
try_mount:
#ifndef INTERNAL_MOUNT
		string mnt_cmd = "mount "+DL_CDROM_DEVICE + " " + DL_CDROM_MOUNTPOINT;
		int mret = system(mnt_cmd.c_str());
#else
		int mret = mount(DL_CDROM_DEVICE.c_str(), DL_CDROM_MOUNTPOINT.c_str(), "iso9660", MS_RDONLY, NULL);
#endif
		if (mret!=0)
		{
			perror("Mount error:");
			errRet = waitResponce(MPKG_CDROM_MOUNT_ERROR);
			if (errRet == MPKG_RETURN_RETRY)
			{
				goto try_mount;
			}
			if (errRet == MPKG_RETURN_ABORT)
			{
				return -1;
			}
		}
	}

	string Svolname;
check_volname:
	string vol_cmd = "volname "+DL_CDROM_DEVICE+" > /tmp/mpkg_volname";
	system(vol_cmd.c_str());
	FILE *volnameFile = fopen("/tmp/mpkg_volname", "r");
	if (volnameFile)
	{
		fscanf(volnameFile, "%s", &volname);
		Svolname=volname;
		Svolname=Svolname.substr(0,Svolname.find("\n"));

		fclose(volnameFile);
	}
	if (Svolname == cdromVolName)
	{
		printf("Wrong volname\n");
		errRet = waitResponce(MPKG_CDROM_WRONG_VOLNAME);
		if (errRet == MPKG_RETURN_RETRY)
		{
			umount(DL_CDROM_MOUNTPOINT.c_str());
			goto try_mount;
		}
		if (errRet == MPKG_RETURN_ABORT)
		{
			return -1;
		}
	}

copy_file:
	string cp_cmd = "cp "+sourceFileName + " " + output + " 2>/dev/null";
	if (system(cp_cmd.c_str())!=0 && sourceFileName.find("packages")!=std::string::npos && sourceFileName.find("PACKAGES")!=std::string::npos)
	{
		errRet = waitResponce(MPKG_SUBSYS_FILE_READ_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
			goto copy_file;
		else return -1;
	}
	return 0;
}


DownloadResults HttpDownload::getFile(std::string url, std::string file, std::string cdromDevice, std::string cdromMountPoint )
{
	string dir = file.substr(0,file.find_last_of("/"));
	dir = "mkdir -p "+dir;
	system(dir.c_str());
	DownloadsList dlList;
	DownloadItem dlItem;
	dlItem.file = file;
	dlItem.url_list.push_back(url);
	dlItem.name = url;
	dlItem.status = DL_STATUS_WAIT;
	dlItem.priority = 0;
	double dlnow, dltotal, itemnow, itemtotal;
	ProgressData z;
	string name;
	dlItem.itemID=0;
	dlList.push_back(dlItem);
	int currentItem;
	return this->getFile(dlList, &dlnow, &dltotal, &itemnow, &itemtotal, &name, cdromDevice, cdromMountPoint, &z);
	
}

DownloadResults HttpDownload::getFile(DownloadsList &list, double *dlnow, double *dltotal, double *itemnow, double *itemtotal, std::string *itemname, std::string cdromDevice, std::string cdromMountPoint, ProgressData *prData)
{
	if (list.empty()) 
	{
		return DOWNLOAD_OK;
	}

	// Sorting (for CD-ROM download optimization)
	// Step 1. Retrieving list of cd-roms.
	//printf("Sorting...\n");
	vector<string> cdromVolumeLabels;
	struct cdromItem cdItem;
	vector<struct cdromItem>cdromSourcedPackages;
	vector<int>nonCdromSourcedPackages;
	DownloadItem *tmp_item;
	bool volfound = false;
	bool isCdromSourced = false;
	string tmp_volname;
	for (int i=0; i<list.size(); i++)
	{
		tmp_item = &(list.at(i));

		isCdromSourced = false;
		for (int j=0; j<tmp_item->url_list.size(); j++)
		{
			if (tmp_item->url_list.at(j).find("cdrom://")!=std::string::npos)
			{
				isCdromSourced=true;
				cdItem.id=i;
				tmp_volname = tmp_item->url_list.at(j).substr(strlen("cdrom://"));
				tmp_volname = tmp_volname.substr(0,tmp_volname.find_first_of("/")-1);
				cdItem.volname = tmp_volname;
				volfound = false;
				cdromSourcedPackages.push_back(cdItem);
				for (int c = 0; c<cdromVolumeLabels.size(); c++)
				{
					if (cdromVolumeLabels.at(c) == tmp_volname)
					{
						volfound = true;
					}
				}
				if (!volfound) cdromVolumeLabels.push_back(tmp_volname);
				break;
			}
			
		}
		if (!isCdromSourced) nonCdromSourcedPackages.push_back(i);
	}
	DownloadsList sortedDownloadsList;
	for (int i=0; i<cdromVolumeLabels.size(); i++)
	{
		for (int j=0; j<cdromSourcedPackages.size(); j++)
		{
			if (cdromSourcedPackages.at(j).volname == cdromVolumeLabels.at(i))
			{
				sortedDownloadsList.push_back(list.at(cdromSourcedPackages.at(j).id));
			}
		}
	}
	for (int i=0; i<nonCdromSourcedPackages.size(); i++)
	{
		sortedDownloadsList.push_back(list.at(nonCdromSourcedPackages.at(i)));
	}
	
	if (list.size() == sortedDownloadsList.size())
	{
		list = sortedDownloadsList;
	}
	else
	{
		fprintf(stderr, "Error! Source and sorted lists doesn't equal in size!!!!\n");
	}


	//printf("Sorting complete\n");



	DL_CDROM_DEVICE = cdromDevice;
	DL_CDROM_MOUNTPOINT = cdromMountPoint;
	// reset status for retry
	for (int i = 0; i<list.size(); i++)
	{
		if (list[i].status != DL_STATUS_OK) list[i].status = DL_STATUS_WAIT;
	}
	extDlNow = dlnow;
	extDlTotal = dltotal;

	CURLcode result;
	DownloadItem *item;
	FILE* out;
	bool is_have_error = false;
	*itemtotal = list.size();
	string dir;
	for (int i = 0; i < list.size(); i++ ) {
process:
		item = &(list.at(i));
		*itemname = item->name;
		if (prData->size()>0)
		{
			prData->currentItem=item->itemID;
		}


		if ( item->status == DL_STATUS_WAIT ) {
			dir = "mkdir -p " + item->file.substr(0,item->file.find_last_of("/"));
			system(dir.c_str());

			out = fopen (item->file.c_str(), "wb");
			if ( out == NULL ) {
				fprintf(stderr, "open target file failed");
				item->status = DL_STATUS_FILE_ERROR;
				is_have_error = true;
			} 
			else 
			{
		        	if (item->url_list.size()==0)
				{
					item->status=DL_STATUS_FAILED;
					is_have_error=true;
				}

    				for ( unsigned int j = 0; j < item->url_list.size(); j++ ) 
				{
					printf("Downloading %s\n", item->url_list.at(j).c_str());
					prData->itemCurrentAction.at(item->itemID)="Downloading";

    				
					if (item->url_list.at(j).find("file://")==0)
					{
						fclose(out);
						if (fileLinker(item->url_list.at(j).substr(strlen("file://")), item->file)==0) break;
						else out = fopen (item->file.c_str(), "wb");
					}
					if (item->url_list.at(j).find("cdrom://")==0)
					{
						fclose(out);
						if (cdromFetch(item->url_list.at(j).substr(strlen("cdrom://")), item->file)==0) break;
						else out = fopen (item->file.c_str(), "wb");
					}
				
					curl_easy_setopt(ch, CURLOPT_WRITEDATA, out);
    					curl_easy_setopt(ch, CURLOPT_NOPROGRESS, false);
 	   				curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, NULL);
    					curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, downloadCallback);
    					curl_easy_setopt(ch, CURLOPT_URL, item->url_list.at(j).c_str());
    			
    					result = curl_easy_perform(ch);
    					fclose(out);
    	
    					if ( result == CURLE_OK  ) {
						item->status = DL_STATUS_OK;
						prData->itemProgress.at(item->itemID)=prData->itemProgressMaximum.at(item->itemID);
						prData->itemCurrentAction.at(item->itemID)="Done";
    					break;
    					}
					else 
					{
						printf("Failed.\n");
    				    		is_have_error = true;
    						item->status = DL_STATUS_FAILED;
    					}
    				}
    			}
        	}

		*itemnow = i;
    	}
	if (!is_have_error) return DOWNLOAD_OK;
	else return DOWNLOAD_ERROR;
}
