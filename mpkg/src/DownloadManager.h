#ifndef _DOWNLOAD_MANAGER_H_
#define _DOWNLOAD_MANAGER_H_

#include <string>

#include "IDownload.h"
#include "DownloadFactory.h"

typedef IDownload* (*GetHandler)();


//extern IDownload *g_pCurrentMethod = NULL;
//extern DownloadFactory *g_pDownloadFactory = NULL;

IDownload*  InitializeDownloadObjects(DownloadFactory* factory);
DownloadResults CommonGetFile( std::string url, std::string output, void *callback = NULL); 
DownloadResults CommonGetFileEx( DownloadsList &list,  double *dlnow, double *dltotal , double *itemnow, double *itemtotal, std::string *itemname); 

#endif


