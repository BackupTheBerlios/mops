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

int curlProgressCallback(void *clientp,
                        double dltotal,
                        double dlnow,
                        double ultotal,
                        double ulnow)
{


DownloadResults HttpDownload::getFile(std::string url, std::string file)
{
	uri = url;
	out = file;

	out_f = fopen( out.c_str(), "wb" );
	if ( out_f == NULL )
			return DOWNLOAD_ERROR;

	curl_easy_setopt(ch, CURLOPT_WRITEDATA, out_f);
	curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, &curlProgressCallback);
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

