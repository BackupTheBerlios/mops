#ifndef HTTPDOWNLOAD_H_
#define HTTPDOWNLOAD_H_

#include <string>
#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "IDownload.h"

class HttpDownload : public IDownload {
public:
	HttpDownload();
	
	virtual ~HttpDownload() {};
	DownloadResults getFile(std::string url, std::string file);
	DownloadResults getFile(DownloadsList &list,  double *dlnow, double *dltotal, double *itemnow, double *itemtotal, std::string *itemname);

private:
	FILE* out_f;
	CURL* ch;

	std::string uri;
	std::string out;
		
};

#endif

