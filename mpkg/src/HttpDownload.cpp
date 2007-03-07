#include "HttpDownload.h"

HttpDownload::HttpDownload()
{
	ch = curl_easy_init();
}
/*
HttpDownload::~HttpDownload()
{

}
*/

DownloadResults HttpDownload::getFile(std::string url, std::string file)
{
	uri = url;
	out = file;

	out_f = fopen( out.c_str(), "wb" );
	if ( out_f == NULL )
			return DOWNLOAD_ERROR;

	curl_easy_setopt(ch, CURLOPT_WRITEDATA, out_f);
	curl_easy_setopt(ch, CURLOPT_URL, uri.c_str());

	CURLcode res = curl_easy_perform(ch);
	
	if ( res == CURLE_OK ) {
		fclose(out_f);
		return DOWNLOAD_OK;
	} else
		return DOWNLOAD_ERROR;
}
