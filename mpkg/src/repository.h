/*****************************************************************
 * Repository working tools: creating index, decrypts data, etc.
 * 
 * Repository types supported:
 * 	Native MOPSLinux (fully supported)
 * 	Legacy Slackware (in development)
 * 	Debian (planned)
 * 	RPM (planned)
 *
 * $Id: repository.h,v 1.13 2007/08/02 13:40:41 i27249 Exp $
 *****************************************************************/
#ifndef REPOSITORY_H_
#define REPOSITORY_H_

#include "DownloadManager.h"
#include "local_package.h"
#include <ftw.h>

#define TYPE_AUTO 0
#define TYPE_MPKG 1
#define TYPE_SLACK 2
#define TYPE_DEBIAN 3
#define TYPE_RPM 4
// Packages & repositories type: mpkg
class Repository
{
	public:
		int build_index(string server_url, string server_name, bool rebuild=false); // builds index of packages (creates packages.xml), consuming REPOSITORY_ROOT is current dir
		int get_index(string server_url, PACKAGE_LIST *packages, unsigned int type=TYPE_AUTO);
		Repository();
		~Repository();
		
	private:
	//	int ProcessPackage(const char *filename, const struct stat *file_status, int filetype);
		PACKAGE_LIST data;
};
//void analyzeFTree(XMLNode *node);
int ProcessPackage(const char *filename, const struct stat *file_status, int filetype);
int slackpackages2list (string *packageslist, string *md5list, PACKAGE_LIST *pkglist, string server_url);

// TODO: Make modules arch, add modules for slackware packages and repositories
// TODO: Add support for retrieving repository list
#endif


