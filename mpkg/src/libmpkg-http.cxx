#include "HttpDownload.h"
#include "IDownload.h"


extern "C" IDownload* getHandler()
{
	return new HttpDownload();
}

