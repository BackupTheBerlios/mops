/*****************************************************************
 * Repository working tools: creating index, decrypts data, etc.
 * $Id: repository.h,v 1.1 2006/12/21 18:09:17 i27249 Exp $
 *****************************************************************/
#ifndef REPOSITORY_H_
#define REPOSITORY_H_

#include "local_package.h"
#include "ftw.h"
class Repository
{
	public:
		int build_index(string server_url); // builds index of packages (creates packages.xml), consuming REPOSITORY_ROOT is current dir
		PACKAGE_LIST get_index(string server_url);

		Repository();
		~Repository();
		
	private:
	//	int ProcessPackage(const char *filename, const struct stat *file_status, int filetype);
		PACKAGE_LIST data;
};
int ProcessPackage(const char *filename, const struct stat *file_status, int filetype);

#endif


