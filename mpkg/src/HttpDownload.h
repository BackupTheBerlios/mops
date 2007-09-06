#ifndef HTTPDOWNLOAD_H_
#define HTTPDOWNLOAD_H_

#include "faststl.h"
#include "debug.h"
#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "IDownload.h"
bool isMounted(string mountpoint);
extern bool usedCdromMount;
int cdromFetch(string source, string output, bool do_cache);
struct cdromItem
{
	int id;
	string volname;
};
class HttpDownload/* : public IDownload */{
public:
	HttpDownload();
	
//	virtual ~HttpDownload() {};
	//~HttpDownload();
	DownloadResults getFile(string url, string file, string cdromDevice = CDROM_DEVICE, string cdromMountPoint = CDROM_MOUNTPOINT);
	DownloadResults getFile(DownloadsList &list, string *itemname, string cdromDevice = CDROM_DEVICE, string cdromMountPoint = CDROM_MOUNTPOINT, ActionBus *aaBus = &actionBus, ProgressData *prData = &pData);

private:
	FILE* out_f;
	CURL* ch;

	string uri;
	string out;
		
};

#endif

