/*
Local package installation functions
$Id: local_package.h,v 1.4 2006/12/20 13:00:47 i27249 Exp $
*/


#ifndef LOCAL_PACKAGE_H_
#define LOCAL_PACKAGE_H_

#include "dependencies.h"
#include "PackageConfig.h"
#include "debug.h"
/** Returns free temporary filename **/
string get_tmp_file();
void delete_tmp_files();
class LocalPackage
{
	public:
		LocalPackage(string _f);
		~LocalPackage();
		PACKAGE data;
		int injectFile();
	private:
		string filename;
		int create_md5();
		int get_size();
		int get_xml();
		int get_scripts();
		int get_filelist();
		int set_additional_data();
		int CreateFlistNode(string fname, string tmp_xml);

};

#endif //LOCAL_PACKAGE_H_

