/******************************************************************
 * Repository class: build index, get index...etc.
 * $Id: repository.cpp,v 1.1 2006/12/21 18:09:17 i27249 Exp $
 * ****************************************************************/
#include "repository.h"

Repository::Repository(){}
Repository::~Repository(){}

int ProcessPackage(const char *filename, const struct stat *file_status, int filetype)
{
	string _package=filename;
       	string ext;
	for (int i=_package.length()-4;i<_package.length();i++)
	{
		ext+=_package[i];
	}

	if (filetype==FTW_F && ext==".tgz")
	{
		printf("indexing file %s\n",filename);
		LocalPackage lp(_package);
		lp.injectFile(true);
		// TODO: create XML node
	}
		

	/*if (filetype==FTW_D)
	{
		printf("[dir] %s\n", filename);
	}
	if (filetype!=FTW_F && filetype!=FTW_D)
	{
		printf("[unknown] %s\n", filename);
	}*/
	return 0;
}


int Repository::build_index(string server_url)
{
	// We consider that repository root is current directory. So, what we need to do:
	// Enter each sub-dir, get each file which name ends with .tgz, extracts xml (and other) data from them, 
	// and build an XML tree for whole repository, then write it to ./packages.xml
	
	
	
	ftw(".", ProcessPackage, 100);
	//LocalPackage lp(package);
	

	return 0;
}

PACKAGE_LIST Repository::get_index(string server_url)
{
	PACKAGE_LIST packages;
	return packages;
}
