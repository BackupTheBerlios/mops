#include "HttpDownload.h"

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>

// CD-ROM support
#include <sys/mount.h>
#include <mntent.h>
//#include "cdconfig.h"
//#include "xmlParser.h"

string DL_CDROM_DEVICE="/dev/cdrom";
string DL_CDROM_MOUNTPOINT="/mnt/cdrom";

HttpDownload::HttpDownload()
{
	ch = curl_easy_init();
}
/*
HttpDownload::~HttpDownload()
{

}
*/

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
//	printf("Download %d/%d\n", *extDlNow, *extDlTotal);
	return 0;
}

int fileLinker(std::string source, std::string output)
{
	std::string execLine = "ln -sf "+source+" "+output;
	return system(execLine.c_str());
}



int cdromFetch(std::string source, std::string output) // Caching of files from CD-ROM devices. URL format: cdrom://CDROM_UUID/directory/filename.tgz
{
	printf("fetching from CDROM %s\n", DL_CDROM_DEVICE.c_str());
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
	FILE *mtab = fopen("/etc/mtab", "r");
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
					umount(mountList->mnt_dir);
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
		if (mount(DL_CDROM_DEVICE.c_str(), DL_CDROM_MOUNTPOINT.c_str(), "iso9660", MS_RDONLY, NULL)!=0)
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

check_volname:
	string vol_cmd = "volname "+DL_CDROM_DEVICE+" > /tmp/mpkg_volname";
	system(vol_cmd.c_str());
	FILE *volnameFile = fopen("/tmp/mpkg_volname", "r");
	if (volnameFile)
	{
		fscanf(volnameFile, "%s", &volname);
		fclose(volnameFile);
	}
	if (strcmp(volname, cdromVolName.c_str())!=0)
	{
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
	string cp_cmd = "cp "+sourceFileName + " " + output;
	if (system(cp_cmd.c_str())!=0)
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
	DL_CDROM_DEVICE = cdromDevice;
	DL_CDROM_MOUNTPOINT = cdromMountPoint;

	uri = url;
	out = file;
	if (url.find("file://")==0)
	{
		if (fileLinker(url.substr(strlen("file://")), file) == 0) return DOWNLOAD_OK;
		else return DOWNLOAD_ERROR;
	}
	if (url.find("cdrom://")==0)
	{
		if (cdromFetch(url.substr(strlen("cdrom://")), file) == 0) return DOWNLOAD_OK;
		else return DOWNLOAD_ERROR;
	}
	out_f = fopen( out.c_str(), "wb" );
	if ( out_f == NULL )
			return DOWNLOAD_ERROR;

	curl_easy_setopt(ch, CURLOPT_WRITEDATA, out_f);
	//curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, &curlProgressCallback);
	curl_easy_setopt(ch, CURLOPT_URL, uri.c_str());

	CURLcode res = curl_easy_perform(ch);
#ifdef DEBUG
       	printf("Download: Curl return %d\n", res);
#endif	
	if ( res == CURLE_OK ) {
		fclose(out_f);
		return DOWNLOAD_OK;
	} else
		return DOWNLOAD_ERROR;
}

DownloadResults HttpDownload::getFile(DownloadsList &list, double *dlnow, double *dltotal, double *itemnow, double *itemtotal, std::string *itemname, std::string cdromDevice, std::string cdromMountPoint)
{
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
	for (int i = 0; i < list.size(); i++ ) {
process:
		item = &(list.at(i));
		*itemname = item->name;

		if ( item->status == DL_STATUS_WAIT ) {
		
			out = fopen (item->file.c_str(), "wb");
			if ( out == NULL ) {
				fprintf(stderr, "open target file failed");
				//setErrorCode (MPKG_DOWNLOAD_ERROR);
				item->status = DL_STATUS_FILE_ERROR;
				is_have_error = true;
			} else {
		        	
    			for ( unsigned int j = 0; j < item->url_list.size(); j++ ) {
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
    					printf("Download seems to be OK\n");
					item->status = DL_STATUS_OK;
    					break;
    				} else {
					printf("Perhaps the download fails.........\n");
    				    	is_have_error = true;
    					item->status = DL_STATUS_FAILED;
    				}
    	
    			}
    			
                /*if ( item->status == DL_STATUS_FAILED ) {
    				//setErrorCode(MPKG_DOWNLOAD_ERROR);
    				for (;;) {
    					if ( getErrorReturn() == MPKG_RETURN_RETRY ) {
    						printf("resolved\n");
    						goto process;
    					}
    					
    					if ( getErrorReturn() == MPKG_RETURN_IGNORE ) {
    					    printf ("resolved\n");
    					    break;
    					} 

    					if ( getErrorReturn() == MPKG_RETURN_ABORT ) {
    						printf("abort\n");
    						return DOWNLOAD_ERROR;
    					}
    					sleep (1);
    				}
    			} else {
    				//setErrorCode(MPKG_DOWNLOAD_OK);
    			}
			*/
    		}
        }

	*itemnow = i;
	
    }

	if (!is_have_error) return DOWNLOAD_OK;
	else return DOWNLOAD_ERROR;
	//return is_have_error ? DOWNLOAD_OK : DOWNLOAD_ERROR; 
}
