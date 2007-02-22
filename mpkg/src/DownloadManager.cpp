#include <iostream>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>


#include "debug.h"
#include "DownloadManager.h"

IDownload*
InitializeDownloadObjects(DownloadFactory *factory)
{

	IDownload *handler = NULL;

	void *dlhandler = dlopen("./libmpkg-http.so", RTLD_LAZY);

	if ( dlhandler == NULL ) {
		fprintf(stderr, "critical error: cannot load libmpkg-http.so\n");
		debug(dlerror());
		abort();
	}

	debug("libmpkg-http.so loaded");
	
	GetHandler lpLoadHandler = (GetHandler ) dlsym(dlhandler, "getHandler");
	//ASSERT( !lpLoadHandler );

	if ( !lpLoadHandler ) {
		fprintf(stderr, "critical error: cannot load symbol: 'getHandler'\n");
		debug(dlerror());
		dlclose(dlhandler);
		abort();
	}

	debug("lpLoadHandler init ok");

	handler = lpLoadHandler();
	
	factory->addMethodHandler(HTTP, handler);

	return handler;

	dlclose(dlhandler);

}

DownloadResults
CommonGetFile(std::string url, std::string output)
{

	DownloadFactory *g_pDownloadFactory = new DownloadFactory();
	IDownload *g_pCurrentMethod = InitializeDownloadObjects(g_pDownloadFactory);

	assert( g_pCurrentMethod );
	debug("load file " + url + " to " + output);

	
	return g_pCurrentMethod->getFile(url, output); 
}	

