#ifndef IDOWNLOAD_H_
#define IDOWNLOAD_H_

#include <string>

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

class IDownload {
public:
	virtual DownloadResults getFile(std::string url, std::string file) = 0;
	virtual ~IDownload() {};
};

#endif

