#include <string>
#include <iostream>
#include <termcap.h>


#include "HttpDownload.h"
#include "cErrors.h"


HttpDownload::HttpDownload()
{
	chandle = curl_easy_init();
}

DownloadResult HttpDownload::getFile(std::string url, std::string toFile)
{
	this->_uri = url;
	this->_out = toFile;

	out_f = fopen( _out.c_str(), "wb");
	if ( out_f == NULL )
		return FILE_WOPEN_ERROR;

	curl_easy_setopt(chandle, CURLOPT_URL, _url);
	curl_easy_setopt(chandle, CURLOPT_WRITEDATA, out_f);
	
	CURLcode res = curl_easy_perform(chandle);
	if ( CURLcode == CURLE_OK ) {
		fclose(out_f);
		return DOWNLOAD_OK;
	}

	return FILE_DOWNLOAD_ERROR;
}

static int HttpDownload::progress_callback(void *clientp,
					                        double dltotal,
					                        double dlnow,
					                        double ultotal,
											double ulnow)
{
	//iint width = tgetnum("co");
	return 0;	
}

