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

	int *extDlNow;
	int *extDlTotal;

static int downloadCallback(void *clientp,
                       double dltotal,
                       double dlnow,
                       double ultotal,
                       double ulnow)
{
	*extDlTotal = int(dltotal);
	*extDlNow = int(dlnow);
	printf("Download %d/%d\n", *extDlNow, *extDlTotal);
	return 0;
}


DownloadResults HttpDownload::getFile(std::string url, std::string file)
{
	uri = url;
	out = file;

	out_f = fopen( out.c_str(), "wb" );
	if ( out_f == NULL )
			return DOWNLOAD_ERROR;

	curl_easy_setopt(ch, CURLOPT_WRITEDATA, out_f);
	//curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, &curlProgressCallback);
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

DownloadResults HttpDownload::getFile(DownloadsList list, int *dlnow, int *dltotal)
{
	extDlNow = dlnow;
	extDlTotal = dltotal;
	CURLcode result;
	DownloadItem item;
	FILE* out;
	for (int i = 0; i < list.size(); i++ ) {
	process:
		item = list.at(i);
		printf("Processing %s...\n", item.file.c_str());
		
		out = fopen(item.file.c_str(), "wb" );
		if ( out == NULL) {
			printf("Error opening target file!\n");
			return DOWNLOAD_ERROR;
		}
		else printf("Target will be written to %s\n", item.file.c_str());

		curl_easy_setopt(ch, CURLOPT_WRITEDATA, out);
		curl_easy_setopt(ch, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, NULL);
		curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, downloadCallback);
		curl_easy_setopt(ch, CURLOPT_URL, item.url.c_str());

		result = curl_easy_perform(ch);
		fclose(out);
		if ( result != CURLE_OK ) {
			i++;
			printf("Download failed\n");
			if ( item.name == list.at(i).name ) {
				goto process;
			} else {
				item.status = 2;
				return DOWNLOAD_ERROR;
			}
			
		}
	
	}

	return DOWNLOAD_OK;
}
