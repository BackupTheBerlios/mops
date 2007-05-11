#ifndef _DOWNLOAD_MANAGER_H_
#define _DOWNLOAD_MANAGER_H_

#include "faststl.h"

#include "IDownload.h"
#include "DownloadFactory.h"

typedef IDownload* (*GetHandler)();


//extern IDownload *g_pCurrentMethod = NULL;
//extern DownloadFactory *g_pDownloadFactory = NULL;

IDownload*  InitializeDownloadObjects(DownloadFactory* factory);
DownloadResults CommonGetFile( string url, string output, void *callback = NULL); 
DownloadResults CommonGetFileEx( DownloadsList &list, double *itemnow, double *itemtotal, string *itemname, ActionBus *aaBus=&actionBus, ProgressData *prData=&pData); 

#endif


