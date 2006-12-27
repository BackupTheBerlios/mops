/******************************************************************
 * Repository class: build index, get index...etc.
 * $Id: repository.cpp,v 1.3 2006/12/27 12:19:02 i27249 Exp $
 * ****************************************************************/
#include "repository.h"

Repository::Repository(){}
Repository::~Repository(){}

XMLNode _root;

int ProcessPackage(const char *filename, const struct stat *file_status, int filetype)
{
#ifdef DEBUG
	printf("repository.cpp: processing package %s\n", filename);
#endif
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
		_root.addChild(lp.getPackageXMLNode());
	}
	return 0;
}


int Repository::build_index(string server_url)
{
	// First of all, initialise main XML tree. Due to some code restrictions, we use global variable _root.
	_root=XMLNode::createXMLTopNode("repository");
	_root.addAttribute("server_url", server_url.c_str());
	
	// Next, run thru files and extract data.
	// We consider that repository root is current directory. So, what we need to do:
	// Enter each sub-dir, get each file which name ends with .tgz, extracts xml (and other) data from them, 
	// and build an XML tree for whole repository, then write it to ./packages.xml
	
	ftw(".", ProcessPackage, 100);

	// Finally, write our XML tree to file
	_root.writeToFile("packages.xml");
	return 0;
}

PACKAGE_LIST Repository::get_index(string server_url)
{
	PACKAGE_LIST packages;
	// Fetching packages.xml from server, parses it, and returns full packagelist. TODO.
	return packages;
}
