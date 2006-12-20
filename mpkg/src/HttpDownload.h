/**
 * $Id: HttpDownload.h,v 1.1 2006/12/20 20:02:56 adiakin Exp $
 */

#include <string>
#include <iostream>
#include <curl/easy.h>

#include "IDownload.h"

class HttpDownload : public IDownload {
public:
	HttpDownload();
	~HttpDownload();
	DownloadResult getFile(std::string url, std::string toFile);

private:
	static int progress_callback(void *clientp, double dltotal,
						double dlnow, double ultotal, double ulnow);
	CURL* chandle;
	FILE* out_f;
	std::string _url;
	std::string _out;	
};

