#include <iostream>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>


#include "debug.h"
#include "DownloadManager.h"

void *dlhandler=NULL;

IDownload*
InitializeDownloadObjects(DownloadFactory *factory)
{

	IDownload *handler = NULL;


	dlhandler = dlopen("libmpkg-http.so", RTLD_LAZY);
	//void *dlhandler = dlopen("./libmpkg-http.so", RTLD_LAZY);
	if ( dlhandler == NULL ) {
		mError("critical error: cannot load libmpkg-http.so");
		mDebug(dlerror());
		abort();
	}

	mDebug("libmpkg-http.so loaded");
	
	GetHandler lpLoadHandler = (GetHandler ) dlsym(dlhandler, "getHandler");
	//ASSERT( !lpLoadHandler );

	if ( !lpLoadHandler ) {
		mError("critical error: cannot load symbol: 'getHandler'");
		mDebug(dlerror());
		dlclose(dlhandler);
		abort();
	}

	mDebug("lpLoadHandler init ok");

	handler = lpLoadHandler();
	
	factory->addMethodHandler(HTTP, handler);

	return handler;

}

DownloadResults
CommonGetFile(std::string url, std::string output, void *callback)
{

	DownloadFactory *g_pDownloadFactory = new DownloadFactory();
	IDownload *g_pCurrentMethod = InitializeDownloadObjects(g_pDownloadFactory);

	assert( g_pCurrentMethod );
	mDebug("load file " + url + " to " + output);

	
	DownloadResults ret = g_pCurrentMethod->getFile(url, output); 
	delete g_pDownloadFactory;
	dlclose(dlhandler);
	return ret;
}	

DownloadResults
CommonGetFileEx(DownloadsList &list, std::string *itemname, ActionBus *aaBus, ProgressData *prData)
{

	DownloadFactory *g_pDownloadFactory = new DownloadFactory();
	IDownload *g_pCurrentMethod = InitializeDownloadObjects(g_pDownloadFactory);

	assert( g_pCurrentMethod );

	DownloadResults ret = g_pCurrentMethod->getFile(list, itemname); 
	delete g_pDownloadFactory;
	dlclose(dlhandler);
	return ret;
}	


