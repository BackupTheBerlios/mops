/*
Local package installation functions
$Id: local_package.h,v 1.15 2007/06/22 00:59:13 i27249 Exp $
*/


#ifndef LOCAL_PACKAGE_H_
#define LOCAL_PACKAGE_H_

#include <sys/param.h>

#include "dependencies.h"
#include "PackageConfig.h"
#include "debug.h"
#include "file_routines.h"

// Package type: mpkg
class LocalPackage
{
	public:
		LocalPackage(string _f, unsigned int pkgType=PKGTYPE_MOPSLINUX);
		~LocalPackage();
		PACKAGE data;
		int injectFile(bool index=false);
		XMLNode getPackageXMLNode();
		XMLNode getPackageFListNode();
		int fill_filelist(PACKAGE *package, bool index=false);
		int fill_scripts(PACKAGE *package);
		int fill_configfiles(PACKAGE *package);

	private:
		bool internal;
		string files2xml(string input);
		string filename;
		XMLNode _packageXMLNode, _packageFListNode;
		int create_md5();
		int get_size();
		int get_xml();
		int get_filelist(bool index=false);
		int set_additional_data();
		unsigned int packageType;
		int CreateFlistNode(string fname, string tmp_xml);

};

#endif //LOCAL_PACKAGE_H_

