#include "HttpDownload.h"
#include "dialog.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdlib.h>
#include <errno.h>
#ifndef INTERNAL_MOUNT
#include <sys/mount.h>
#endif
#include <mntent.h>
string DL_CDROM_DEVICE="/dev/cdrom";
string DL_CDROM_MOUNTPOINT="/mnt/cdrom";

#define DOWNLOAD_TIMEOUT 10 // 10 seconds, and failing
int downloadTimeout=0;
double prevDlValue;

HttpDownload::HttpDownload()
{
	ch = curl_easy_init();
}

	double *extDlNow;
	double *extDlTotal;
	double *extItemTotal;
	double *extItemNow;

	ProgressData *ppData;
	ActionBus *ppActionBus;
	int currentItemID;
static int downloadCallback(void *clientp,
                       double dltotal,
                       double dlnow,
                       double ultotal,
                       double ulnow)
{
	double t = ultotal;
	t=ulnow;
	void *t1;
       t1= clientp;
	ppData->setItemProgress(currentItemID, dlnow);
	if (prevDlValue==dlnow) downloadTimeout++;
	else {
		prevDlValue=dlnow;
		downloadTimeout=0;
	}
	if (ppActionBus->_abortActions)
	{
		return -2;
	}
	if (!dialogMode)
	{
		Dialog dialogItem;
		dialogItem.execGauge("Скачивается файл " + ppData->getItemName(currentItemID), 10,80, (unsigned int) round(dlnow/(dltotal/100)));
	}

	if (downloadTimeout>DOWNLOAD_TIMEOUT) return -1;
	return 0;
}


int fileLinker(std::string source, std::string output)
{

	unlink(output.c_str());
	int ret = symlink(source.c_str(), output.c_str());
	return ret;
}

#define CACHE_CDROM_CONTENTS true;

int cdromFetch(std::string source, std::string output, bool do_cache) // Caching of files from CD-ROM devices. URL format: cdrom://CDROM_UUID/directory/filename.tgz
{
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
	
	//int mount_ret;
	//int umount_ret;
	mkdir(DL_CDROM_MOUNTPOINT.c_str(), 755);
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
	
	// check_volname:
	
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
		mError("Wrong volname");
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
	string cp_cmd;
       	int link_ret;
	unlink(output.c_str());
	if (do_cache)
	{
		cp_cmd = "cp -f ";
		cp_cmd += sourceFileName + " " + output + " 2>/dev/null";
		link_ret = system(cp_cmd.c_str());
	}
	else link_ret = symlink(sourceFileName.c_str(), output.c_str());

	if (link_ret!=0 && sourceFileName.find("packages")!=std::string::npos && sourceFileName.find("PACKAGES")!=std::string::npos)
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
	mDebug("Downloading " + url + " to " + file);
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
	ProgressData z;
	string name;
	dlItem.itemID=0;
	dlList.push_back(dlItem);
	unlink(file.c_str()); // Let's download from scratch
	return this->getFile(dlList, &name, cdromDevice, cdromMountPoint, &actionBus, &z);
	
}

DownloadResults HttpDownload::getFile(DownloadsList &list, std::string *itemname, std::string cdromDevice, std::string cdromMountPoint,  ActionBus *aaBus, ProgressData *prData)
{
	ppActionBus=aaBus;
	ppData=prData;
	if (list.empty()) 
	{
		return DOWNLOAD_OK;
	}

	// Sorting (for CD-ROM download optimization)
	// Step 1. Retrieving list of cd-roms.
	vector<string> cdromVolumeLabels;
	struct cdromItem cdItem;
	vector<struct cdromItem>cdromSourcedPackages;
	vector<int>nonCdromSourcedPackages;
	DownloadItem *tmp_item;
	bool volfound = false;
	bool isCdromSourced = false;
	string tmp_volname;
	for (unsigned int i=0; i<list.size(); i++)
	{
		if (ppActionBus->_abortActions)
		{
			ppActionBus->_abortComplete=true;
			return DOWNLOAD_OK;
		}
		tmp_item = &(list.at(i));

		isCdromSourced = false;
		for (unsigned int j=0; j<tmp_item->url_list.size(); j++)
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
				for (unsigned int c = 0; c<cdromVolumeLabels.size(); c++)
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
	for (unsigned int i=0; i<cdromVolumeLabels.size(); i++)
	{
		for (unsigned int j=0; j<cdromSourcedPackages.size(); j++)
		{
			if (cdromSourcedPackages.at(j).volname == cdromVolumeLabels.at(i))
			{
				sortedDownloadsList.push_back(list.at(cdromSourcedPackages.at(j).id));
			}
		}
	}
	for (unsigned int i=0; i<nonCdromSourcedPackages.size(); i++)
	{
		sortedDownloadsList.push_back(list.at(nonCdromSourcedPackages.at(i)));
	}
	
	if (list.size() == sortedDownloadsList.size())
	{
		list = sortedDownloadsList;
	}
	else
	{
		mError("Error! Source and sorted lists doesn't equal in size!!!!");
	}

	DL_CDROM_DEVICE = cdromDevice;
	DL_CDROM_MOUNTPOINT = cdromMountPoint;

	// reset status for retry
	for (unsigned int i = 0; i<list.size(); i++)
	{
		if (list[i].status != DL_STATUS_OK) list[i].status = DL_STATUS_WAIT;
	}

	CURLcode result=CURLE_OK;
	DownloadItem *item;
	FILE* out;
	bool is_have_error = false;
	string dir;
	for (unsigned int i=0; i<list.size(); i++)
	{
		prData->setItemProgressMaximum(list.at(i).itemID, list.at(i).expectedSize);
	}

	for (unsigned int i = 0; i < list.size(); i++ ) {
//process:
		
		item = &(list.at(i));
		*itemname = item->name;
		currentItemID=item->itemID;

		if ( item->status == DL_STATUS_WAIT ) {
			{
		        	if (item->url_list.size()==0)
				{
					mError("Downloading " + item->name + "is Failed: void download list");
					item->status=DL_STATUS_FAILED;
					is_have_error=true;
				}

    				for ( unsigned int j = 0; j < item->url_list.size(); j++ ) 
				{
					if (!dialogMode) say("Downloading %s\n", item->url_list.at(j).c_str());
					else mDebug("Downloading " + item->url_list.at(j));
					if (prData->size()>0)
					{
						prData->setItemCurrentAction(item->itemID, "Downloading");
						prData->setItemState(item->itemID,ITEMSTATE_INPROGRESS);
					}
					if (item->url_list.at(j).find("local://")==0)
					{
						if (fileLinker(item->url_list.at(j).substr(strlen("local://")), item->file)==0)
						{
							result=CURLE_OK;
						}
						else result=CURLE_READ_ERROR;
					}
					else
					if (item->url_list.at(j).find("file://")==0)
					{
						
						if (fileLinker(item->url_list.at(j).substr(strlen("file://")), item->file)==0)
						{
							result=CURLE_OK;
						}
						else result=CURLE_READ_ERROR;
					}
					else
					if (item->url_list.at(j).find("cdrom://")==0)
					{
						if (cdromFetch(item->url_list.at(j).substr(strlen("cdrom://")), item->file, false)==0)
						{
							result=CURLE_OK;
							prData->setItemProgress(item->itemID, prData->getItemProgressMaximum(item->itemID));
							prData->setItemState(item->itemID, ITEMSTATE_FINISHED);
						}
						else result=CURLE_READ_ERROR;
					}

					else if (item->url_list.at(j).find("file://")!=0 && item->url_list.at(j).find("cdrom://")!=0)
					{	
						long size;
						struct stat fStat;
						if (stat(item->file.c_str(), &fStat)==0) size = fStat.st_size;
						else size=0;
					
						out = fopen (item->file.c_str(), "ab");
						prData->setItemProgress(item->itemID, (double) size);
						if ( out == NULL )
						{
							mError("open target file failed");
							item->status = DL_STATUS_FILE_ERROR;
							is_have_error = true;
						}
						else
						{
							mDebug("Trying to download via CURL");
							fseek(out,0,SEEK_END);
							//if (size!=0) say("Resuming download from %i\n", size);
							//curl_easy_setopt(ch, CURLOPT_RESUME_FROM, size);	
							curl_easy_setopt(ch, CURLOPT_WRITEDATA, out);
    							curl_easy_setopt(ch, CURLOPT_NOPROGRESS, false);
 	   						curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, NULL);
    							curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, downloadCallback);
    							curl_easy_setopt(ch, CURLOPT_URL, item->url_list.at(j).c_str());
	    						result = curl_easy_perform(ch);
    							fclose(out);
						}
					}
	    				if ( result == CURLE_OK  ) 
					{
						if (ppActionBus->_abortActions)
						{
							ppActionBus->_abortComplete=true;
#ifdef DL_CLEANUP
							curl_easy_cleanup(ch);
#endif
							return DOWNLOAD_OK;
						}
						item->status = DL_STATUS_OK;
						if (prData->size()>0)
						{
							prData->setItemCurrentAction(item->itemID, "Downloading finished");
							prData->setItemState(item->itemID, ITEMSTATE_FINISHED);
						}
    						break;
					}
					else 
					{
						if (ppActionBus->_abortActions)
						{
							ppActionBus->_abortComplete=true;
#ifdef DL_CLEANUP
							curl_easy_cleanup(ch);
#endif
							return DOWNLOAD_ERROR;
						}
						if (prData->size()>0) prData->setItemState(item->itemID, ITEMSTATE_FAILED);

						mError("Downloading " + item->name + " is Failed: error while downloading");
    			    			is_have_error = true;
    						item->status = DL_STATUS_FAILED;
    					}
    				}
    			}
        	}

		if (ppActionBus->currentProcessingID()==ACTIONID_DOWNLOAD) ppActionBus->setActionProgress(ACTIONID_DOWNLOAD, i);

    	}
	if (!is_have_error) 
	{
#ifdef DL_CLEANUP
		curl_easy_cleanup(ch);
#endif
		return DOWNLOAD_OK;

	}
	else
	{
#ifdef DL_CLEANUP
		curl_easy_cleanup(ch);
#endif
		return DOWNLOAD_ERROR;
	}
}
