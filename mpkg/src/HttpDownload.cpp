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

#define DOWNLOAD_TIMEOUT 10 // 10 seconds, and failing
int downloadTimeout=0;
double prevDlValue;
bool usedCdromMount=false;
string currentDownloadingString;
HttpDownload::HttpDownload()
{
	ch = curl_easy_init();
}
DownloadItem::DownloadItem()
{
	usedSource = NULL;
}
DownloadItem::~DownloadItem()
{
}
double *extDlNow;
double *extDlTotal;
double *extItemTotal;
double *extItemNow;
double resumePos;
ProgressData *ppData;
ActionBus *ppActionBus;
int currentItemID;
Dialog d;
double i_dlnow=0, i_dltotal=0;
static int downloadCallback(void *clientp,
                       double dltotal,
                       double dlnow,
                       double ultotal,
                       double ulnow)
{
	i_dlnow=dlnow;
	i_dltotal=dltotal;
	double t = ultotal;
	t=ulnow;
	void *t1;
	t1= clientp;
	
	ppData->setItemProgress(currentItemID, resumePos+dlnow);
	//ppData->setItemProgressMaximum(currentItemID, dltotal);
	ppData->setItemCurrentAction(currentItemID, _("Downloading: ") + IntToStr((unsigned int) round((resumePos+dlnow)/((resumePos+dltotal)/100)))+"% (" + IntToStr( (unsigned int) (resumePos+dlnow)/1024) + _(" of ") + IntToStr( (unsigned int) (resumePos+dltotal)/1024) + _(" kbytes)") );
	if (prevDlValue==dlnow) downloadTimeout++;
	else {
		prevDlValue=dlnow;
		downloadTimeout=0;
	}
	if (ppActionBus->_abortActions)
	{
		return -2;
	}
	if (dialogMode)
	{
		d.setGaugeValue((unsigned int) round(dlnow/(dltotal/100)));
	}
	else
	{
	if (!currentDownloadingString.empty()) say("\r%s: %d%% (%d %s %d %s)",currentDownloadingString.c_str(), (unsigned int) round(dlnow/(dltotal/100)), (unsigned int) dlnow/1024, _("of"), (unsigned int) dltotal/1024, _("kb"));
		//zC++;
		//printf("%d\n", zC);
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
	mDebug("DEV: " + CDROM_DEVICE+", MP: " + CDROM_MOUNTPOINT);
	mpkgErrorReturn errRet;
	
	bool mounted = false;
	mDebug("source=["+source+"]");
	mDebug("output=["+output+"]");
	// Special case: packages.xml.gz from CD
	// First, trying to get from cache
	string cdromVolName = source.substr(0,source.find_first_of("/"));
	string sourceFileName = CDROM_MOUNTPOINT + source.substr(source.find_first_of("/"));

	if (source.find("packages.xml.gz")!=std::string::npos && FileExists("/var/mpkg/index_cache/"+cdromVolName+"/packages.xml.gz"))
	{
		mDebug("cached contents");
		sourceFileName = "/var/mpkg/index_cache/"+cdromVolName+"/packages.xml.gz";
		do_cache=true;
		string _cp_cmd;
       		int _link_ret;
		unlink(output.c_str());
		if (do_cache)
		{
			_cp_cmd = "cp -f ";
			_cp_cmd += sourceFileName + " " + output + " 2>/dev/null";
			_link_ret = system(_cp_cmd);
			if (_link_ret == 0) mDebug("cache download OK");
			else mDebug("Failed to download the cached file");
		}

		else _link_ret = symlink(sourceFileName.c_str(), output.c_str());
		return 0;
	}
	else mDebug("Not cached");

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
	// Note: VOLNAME should be determined by file .volume_id at root of CD
	//int mount_ret;
	//int umount_ret;
	
	Dialog d("Монтирование CD-ROM");
	if (true)
	{
		mounted = isMounted(CDROM_MOUNTPOINT);
		if (mounted) mDebug("mounted already, proceeding to check the volume");
		if (!mounted)
		{
			mDebug("Trying to mount");

			mkdir(CDROM_MOUNTPOINT.c_str(), 755);
try_mount:
			if (dialogMode) d.execInfoBox("Подключение " + CDROM_DEVICE + " к точке монтирования " + CDROM_MOUNTPOINT);
			else say(_("Mounting %s to %s\n"), CDROM_DEVICE.c_str(), CDROM_MOUNTPOINT.c_str());
			mDebug("Mounting");
//#ifndef INTERNAL_MOUNT
			mDebug("Mount using system");
			string mnt_cmd = "mount "+CDROM_DEVICE + " " + CDROM_MOUNTPOINT;
			usedCdromMount = true;
			system("umount " + CDROM_MOUNTPOINT);
			int mret = system(mnt_cmd);
//#else
/*
		mDebug("Mount using kernel");
		int mret = mount(CDROM_DEVICE.c_str(), CDROM_MOUNTPOINT.c_str(), "iso9660", MS_RDONLY, NULL);
		usedCdromMount = true;
*/
//#endif
			if (mret!=0)
			{
				mDebug("Mount failed with return code "+ IntToStr(mret));
				CDROM_VOLUMELABEL = cdromVolName;
				CDROM_DEVICENAME = CDROM_DEVICE;
				if (dialogMode) 
				{
					if (d.execYesNo("Вставьте диск с меткой " + cdromVolName + " в привод " + CDROM_DEVICE)) goto try_mount;
					else return -1;
				}
				else
				{
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
			else 
			{
				mDebug("Mount successful");
				mounted=true;
			}
		}
		else mDebug("Already mounted");

		string Svolname = getCdromVolname();
	
		mDebug("CDROM mounted, checking volume ID");
	

		//Svolname = cutSpaces(ReadFile(CDROM_MOUNTPOINT + "/.volume_id"));
		if (Svolname.empty())
		{
			// Means no volname
			mError("No volname");
		}
		if (Svolname != cdromVolName)
		{
			mError("Wrong volname");
			if (dialogMode) 
			{
				if (d.execYesNo("Вставьте диск с меткой " + cdromVolName + " в привод " + CDROM_DEVICE)) goto try_mount;
				else return -1;
			}
			else
			{
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
	}
copy_file:
	mDebug("Copying file");
	string cp_cmd;
       	int link_ret;
	unlink(output.c_str());
	if (do_cache)
	{
		mDebug("Using cache for " + sourceFileName + ", copying to " + output);
		cp_cmd = "cp -f ";
		cp_cmd += sourceFileName + " " + output + " 2>/dev/null";
		link_ret = system(cp_cmd.c_str());
	}
	else 
	{
		mDebug("Creating symlink from " + sourceFileName + " to " + output);
		link_ret = symlink(sourceFileName.c_str(), output.c_str());
		mDebug("Link returned: " + IntToStr(link_ret));
	}

	if (link_ret!=0 && sourceFileName.find("packages")!=std::string::npos && sourceFileName.find("PACKAGES")!=std::string::npos)
	{
		errRet = waitResponce(MPKG_SUBSYS_FILE_READ_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
			goto copy_file;
		else return -1;
	}
	mDebug("Copy OK");
	return 0;
}


DownloadResults HttpDownload::getFile(std::string url, std::string file, std::string cdromDevice, std::string cdromMountPoint )
{
	// Step 1: unlink the file
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

	CDROM_DEVICE = cdromDevice;
	CDROM_MOUNTPOINT = cdromMountPoint;

	// reset status for retry
	for (unsigned int i = 0; i<list.size(); i++)
	{
		if (list[i].status != DL_STATUS_OK) list[i].status = DL_STATUS_WAIT;
	}

	string downloadUrl_string;
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
		if ( item->status != DL_STATUS_WAIT ) printf("Item %d doesn't have a download status\n",i);
		if ( item->status == DL_STATUS_WAIT ) {
			
		        	if (item->url_list.size()==0)
				{
					mError("Downloading " + item->name + "is Failed: void download list");
					item->status=DL_STATUS_FAILED;
					is_have_error=true;
				}

    				for ( unsigned int j = 0; j < item->url_list.size(); j++ ) 
				{
					mDebug("Downloading " + item->url_list.at(j));
					if (prData->size()>0)
					{
						prData->setItemCurrentAction(item->itemID, "Downloading");
						prData->setItemState(item->itemID,ITEMSTATE_INPROGRESS);
					}
					if (item->url_list.at(j).find("local://")==0)
					{
						if (fileLinker(item->url_list.at(j).substr(strlen("local://")), item->file)==0)
						{
							if (item->usedSource!=NULL) *item->usedSource = item->url_list.at(j);
							result=CURLE_OK;
						}
						else result=CURLE_READ_ERROR;
					}
					else
					if (item->url_list.at(j).find("file://")==0)
					{
						
						if (fileLinker(item->url_list.at(j).substr(strlen("file://")), item->file)==0)
						{
							if (item->usedSource!=NULL) *item->usedSource = item->url_list.at(j);
							result=CURLE_OK;
						}
						else result=CURLE_READ_ERROR;
					}
					else
					if (item->url_list.at(j).find("cdrom://")==0)
					{
						if (cdromFetch(item->url_list.at(j).substr(strlen("cdrom://")), item->file, false)==0)
						{
							if (item->usedSource!=NULL) *item->usedSource = item->url_list.at(j);
							result=CURLE_OK;
							prData->setItemProgress(item->itemID, prData->getItemProgressMaximum(item->itemID));
							prData->setItemState(item->itemID, ITEMSTATE_FINISHED);
						}
						else result=CURLE_READ_ERROR;
					}

					else if (item->url_list.at(j).find("file://")!=0 && item->url_list.at(j).find("cdrom://")!=0)
					{
						long long size=0;
						if (enableDownloadResume) {
							struct stat fStat;
							if (stat(item->file.c_str(), &fStat)==0) 
							{
								if (S_ISREG(fStat.st_mode))
								{
									size = fStat.st_size;
								}
								else
								{
									unlink(item->file.c_str());
								}
							}
							prData->setItemProgress(item->itemID, (double) size);
						}
						else {
							prData->setItemProgress(item->itemID, 0);
							unlink(item->file.c_str());
						}
						resumePos = 0;
						out = fopen (item->file.c_str(), "ab");
						if ( out == NULL )
						{
							mError("open target file failed");
							item->status = DL_STATUS_FILE_ERROR;
							is_have_error = true;
						}
						else
						{
							// Test
							//ch = curl_easy_init();


							mDebug("Trying to download via CURL");
							fseek(out,0,SEEK_END);
							if (enableDownloadResume) {
								if (size!=0) {
									say(_("Resuming download from %Li\n"), size);
									curl_easy_setopt(ch, CURLOPT_RESUME_FROM, size);
									resumePos = (double) size;
								}
							}
							curl_easy_setopt(ch, CURLOPT_WRITEDATA, out);
    							curl_easy_setopt(ch, CURLOPT_NOPROGRESS, false);
 	   						curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, NULL);
    							curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, downloadCallback);
    							curl_easy_setopt(ch, CURLOPT_URL, item->url_list.at(j).c_str());
	    						downloadUrl_string = item->url_list.at(j);
							if (dialogMode) {
								d.execGauge("["+IntToStr(i+1)+"/"+IntToStr(list.size())+_("] Downloading file ") + \
									downloadUrl_string, 10,80, (unsigned int) round(i_dlnow/(i_dltotal/100)));
							}
							else if (item->url_list.at(j).find("packages.xml.gz")==std::string::npos && 
									item->url_list.at(j).find("PACKAGES.TXT")==std::string::npos &&
							       		item->url_list.at(j).find("Packages.gz")==std::string::npos) {
								
								currentDownloadingString= "["+IntToStr(i+1) + "/" + IntToStr(list.size())+_("] Downloading file ") + downloadUrl_string;
							}
							result = curl_easy_perform(ch);
							if (!currentDownloadingString.empty()) { 
								say("\n");
								currentDownloadingString.clear();
							}
    							fclose(out);
							if (dialogMode) d.closeGauge();
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
							if (item->usedSource!=NULL) *item->usedSource = item->url_list.at(j);

							return DOWNLOAD_OK;
						}
						item->status = DL_STATUS_OK;
						if (prData->size()>0)
						{
							prData->setItemCurrentAction(item->itemID, _("Downloading finished"));
							prData->setItemState(item->itemID, ITEMSTATE_FINISHED);
						}
    						break;
					}
					else 
					{
						mError(_("Download error: ") + (string) curl_easy_strerror(result));
						if (ppActionBus->_abortActions)
						{
							ppActionBus->_abortComplete=true;
#ifdef DL_CLEANUP
							curl_easy_cleanup(ch);

#endif
							return DOWNLOAD_ERROR;
						}
						if (prData->size()>0) prData->setItemState(item->itemID, ITEMSTATE_FAILED);

						mError(_("Downloading ") + item->name + _(" is failed: error while downloading"));
    			    			is_have_error = true;
    						item->status = DL_STATUS_FAILED;
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
