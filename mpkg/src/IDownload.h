#ifndef IDOWNLOAD_H_
#define IDOWNLOAD_H_

#include <string>
#include <vector>

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
	std::string url;
	std::string server;
	std::string name;
	unsigned int priority;
	unsigned int status;
};

typedef std::vector<DownloadItem> DownloadsList;


class IDownload {
public:
	virtual DownloadResults getFile(std::string url, std::string file) = 0;
	virtual DownloadResults getFile(DownloadsList list, int *dlnow, int *dltotal) = 0;
	virtual ~IDownload() {};
};

#endif

