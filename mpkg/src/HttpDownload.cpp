#include "HttpDownload.h"

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>

HttpDownload::HttpDownload()
{
	ch = curl_easy_init();
}
/*
HttpDownload::~HttpDownload()
{

}
*/

	double *extDlNow;
	double *extDlTotal;
	double *extItemTotal;
	double *extItemNow;

static int downloadCallback(void *clientp,
                       double dltotal,
                       double dlnow,
                       double ultotal,
                       double ulnow)
{
	*extDlTotal = dltotal;
	*extDlNow = dlnow;
//	printf("Download %d/%d\n", *extDlNow, *extDlTotal);
	return 0;
}

int fileLinker(std::string source, std::string output)
{
	std::string execLine = "ln -sf "+source+" "+output;
	return system(execLine.c_str());
}

DownloadResults HttpDownload::getFile(std::string url, std::string file)
{
	uri = url;
	out = file;
	if (url.find("file://")==0)
	{
		if (fileLinker(url.substr(strlen("file://")), file) == 0) return DOWNLOAD_OK;
	}
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

DownloadResults HttpDownload::getFile(DownloadsList &list, double *dlnow, double *dltotal, double *itemnow, double *itemtotal, std::string *itemname)
{
	// reset status for retry
	for (int i = 0; i<list.size(); i++)
	{
		if (list[i].status != DL_STATUS_OK) list[i].status = DL_STATUS_WAIT;
	}
	extDlNow = dlnow;
	extDlTotal = dltotal;
	CURLcode result;
	DownloadItem *item;
	FILE* out;
	bool is_have_error = false;
	*itemtotal = list.size();
	for (int i = 0; i < list.size(); i++ ) {
process:
		item = &(list.at(i));
		*itemname = item->name;

		if ( item->status == DL_STATUS_WAIT ) {
		
			out = fopen (item->file.c_str(), "wb");
			if ( out == NULL ) {
				fprintf(stderr, "open target file failed");
				//setErrorCode (MPKG_DOWNLOAD_ERROR);
				item->status = DL_STATUS_FILE_ERROR;
				is_have_error = true;
			} else {
		        	
    			for ( unsigned int j = 0; j < item->url_list.size(); j++ ) {
    				if (item->url_list.at(j).find("file://")==0)
				{
					fclose(out);
					if (fileLinker(item->url_list.at(j).substr(strlen("file://")), item->file)==0) break;
					else out = fopen (item->file.c_str(), "wb");
				}

				
				curl_easy_setopt(ch, CURLOPT_WRITEDATA, out);
    				curl_easy_setopt(ch, CURLOPT_NOPROGRESS, false);
    				curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, NULL);
    				curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, downloadCallback);
    				curl_easy_setopt(ch, CURLOPT_URL, item->url_list.at(j).c_str());
    			
    				result = curl_easy_perform(ch);
    				fclose(out);
    	
    				if ( result == CURLE_OK  ) {
    					printf("Download seems to be OK\n");
					item->status = DL_STATUS_OK;
    					break;
    				} else {
					printf("Perhaps the download fails.........\n");
    				    	is_have_error = true;
    					item->status = DL_STATUS_FAILED;
    				}
    	
    			}
    			
                /*if ( item->status == DL_STATUS_FAILED ) {
    				//setErrorCode(MPKG_DOWNLOAD_ERROR);
    				for (;;) {
    					if ( getErrorReturn() == MPKG_RETURN_RETRY ) {
    						printf("resolved\n");
    						goto process;
    					}
    					
    					if ( getErrorReturn() == MPKG_RETURN_IGNORE ) {
    					    printf ("resolved\n");
    					    break;
    					} 

    					if ( getErrorReturn() == MPKG_RETURN_ABORT ) {
    						printf("abort\n");
    						return DOWNLOAD_ERROR;
    					}
    					sleep (1);
    				}
    			} else {
    				//setErrorCode(MPKG_DOWNLOAD_OK);
    			}
			*/
    		}
        }

	*itemnow = i;
	
    }

	if (!is_have_error) return DOWNLOAD_OK;
	else return DOWNLOAD_ERROR;
	//return is_have_error ? DOWNLOAD_OK : DOWNLOAD_ERROR; 
}
