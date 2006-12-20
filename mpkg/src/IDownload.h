/**
 *
 * $Id: IDownload.h,v 1.1 2006/12/20 20:02:56 adiakin Exp $
 */

#include <string>
#include <map>

#include "cErrors.h"
#include "HttpDownload.h"

class IDownload {
public:

	/**
	 * download package
	 * @param url package uri
	 * @param toFile where save file
	 * @return result code
	 */
	virtual DownloadResult getFile(std::string url, std::string toFile) = 0;
};

class DownloadFactory {
private:
	std::map<std::string, IDownload*> classes;

public:
	void addMethodHandler(std::string type, IDownload* method);
	void getMethodHandler(std::string type);
};

void DownloadFactory::addMethodHandler(std::string type, IDownload* method)
{
	classes[ type ] = method;
}

IDownload* getMethodHandler(std::string type)
{
	return classes[ type ];
}
		


