#ifndef HTTPDOWNLOAD_H_
#define HTTPDOWNLOAD_H_

#include "faststl.h"
#include "debug.h"
#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "IDownload.h"
struct cdromItem
{
	int id;
	string volname;
};
class HttpDownload : public IDownload {
public:
	HttpDownload();
	
	virtual ~HttpDownload() {};
	DownloadResults getFile(string url, string file, string cdromDevice, string cdromMountPoint);
	DownloadResults getFile(DownloadsList &list, string *itemname, string cdromDevice, string cdromMountPoint, ActionBus *aaBus, ProgressData *prData);

private:
	FILE* out_f;
	CURL* ch;

	string uri;
	string out;
		
};

#endif

