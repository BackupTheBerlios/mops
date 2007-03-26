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

DownloadResults HttpDownload::getFile(DownloadsList &list, double *dlnow, double *dltotal, double *itemnow, double *itemtotal, std::string *itemname)
{

//	key_t key;
//	int semid;
//	struct semun arg = {0, -1, 0};


//	key = ftok("/etc/mpkg.xml", getpid());

//	semid = semget(key, 1, 0666, | IPC_CREAT);

	extDlNow = dlnow;
	extDlTotal = dltotal;
	CURLcode result;
	DownloadItem *item;
	FILE* out;
	*itemtotal = list.size();
	for (int i = 0; i < list.size(); i++ ) {
		item = &(list.at(i));
		*itemname = item->name;

		if ( item->status == DL_STATUS_WAIT ) {
		
			out = fopen (item->file.c_str(), "wb");
			if ( out == NULL ) {
				fprintf(stderr, "open target file failed");
				item->status = DL_STATUS_FAILED;
			}
	
			for ( unsigned int j = 0; j < item->url_list.size(); j++ ) {
				curl_easy_setopt(ch, CURLOPT_WRITEDATA, out);
				curl_easy_setopt(ch, CURLOPT_NOPROGRESS, false);
				curl_easy_setopt(ch, CURLOPT_PROGRESSDATA, NULL);
				curl_easy_setopt(ch, CURLOPT_PROGRESSFUNCTION, downloadCallback);
				curl_easy_setopt(ch, CURLOPT_URL, item->url_list.at(j).c_str());
			
				result = curl_easy_perform(ch);
				fclose(out);
	
				if ( result == CURLE_OK  ) {
					item->status = DL_STATUS_OK;
					break;
				} else {
					item->status = DL_STATUS_FAILED;
				}
	
			}

//			if ( item->status == DL_STATUS_FAILED ) {
				/* Oops.. */
/*				arg.val = 1;
				semctrl(semid, 1, SETVAL, arg);

				int r = semop(semid, &arg, 1);
				assert( r == -1 );
				
				r = semop(semid, &arg, 0);
				while ( r != -1 ) {
					sleep("100");
					r = semop(semid, &arg, 0);
					
				}
*/
			}

		*itemnow = i;
	
		}

	return DOWNLOAD_OK;
}
