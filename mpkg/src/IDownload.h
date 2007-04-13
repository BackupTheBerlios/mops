#ifndef IDOWNLOAD_H_
#define IDOWNLOAD_H_

#include <string>
#include <vector>
#include "config.h"

#define DL_STATUS_OK      1
#define DL_STATUS_WAIT   -1
#define DL_STATUS_FAILED -2
#define DL_STATUS_FILE_ERROR -3

typedef enum {
	DOWNLOAD_OK = 1,
	DOWNLOAD_ERROR,
} DownloadResults;

typedef enum {
	HTTP = 1,
	HTTPS,
	FTP,
	SFTP,
	RSYNC,
} DownloadMethods;

class DownloadItem {
public:
	std::string file;
	std::vector<std::string> url_list;
	std::string name;
	unsigned int priority;
	int status;
};

typedef std::vector<DownloadItem> DownloadsList;





class IDownload {
public:
	virtual DownloadResults getFile(std::string url, std::string file, std::string cdromDevice = CDROM_DEVICE, std::string cdromMountPoint = CDROM_MOUNTPOINT) = 0;
	virtual DownloadResults getFile(DownloadsList &list, double *dlnow, double *dltotal, double *itemnow, double *itemtotal, std::string *itemname, std::string cdromDevice = CDROM_DEVICE, std::string cdromMountPoint = CDROM_MOUNTPOINT) = 0;
	virtual ~IDownload() {};
};

#endif

