/*
Local package installation functions
$Id: local_package.h,v 1.10 2007/01/26 16:49:38 i27249 Exp $
*/


#ifndef LOCAL_PACKAGE_H_
#define LOCAL_PACKAGE_H_

#include <sys/param.h>

#include "dependencies.h"
#include "PackageConfig.h"
#include "debug.h"
#include "file_routines.h"
class LocalPackage
{
	public:
		LocalPackage(string _f);
		~LocalPackage();
		PACKAGE data;
		int injectFile(bool index=false);
		XMLNode getPackageXMLNode();
		int fill_filelist(PACKAGE *package);
		int fill_scripts(PACKAGE *package);
		int fill_configfiles(PACKAGE *package);

	private:
		string filename;
		XMLNode _packageXMLNode;
		int create_md5();
		int get_size();
		int get_xml();
		int get_scripts();
		int get_filelist();
		int set_additional_data();
		int CreateFlistNode(string fname, string tmp_xml);

};

#endif //LOCAL_PACKAGE_H_

