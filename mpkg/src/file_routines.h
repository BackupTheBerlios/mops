/******************************************************
* File operations
* $Id: file_routines.h,v 1.8 2007/03/28 14:39:58 i27249 Exp $
* ****************************************************/
#ifndef FILE_ROUTINES_H_
#define FILE_ROUTINES_H_
#include <string>
#include <vector>
#include "config.h"
#include "dataunits.h"
using namespace std;
#define PKGTYPE_UNKNOWN 0
#define PKGTYPE_SLACKWARE 1
#define PKGTYPE_MOPSLINUX 2
#define PKGTYPE_DEBIAN 3	// For future implementation
#define PKGTYPE_RPM 4		// For future implementation



//#include "local_package.h"
#include <unistd.h>
#include <errno.h>
//#include <libtar.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/param.h>
string get_tmp_file();
void delete_tmp_files();
bool FileExists(string filename);
bool FileNotEmpty(string filename);
string ReadFile(string filename, int max_count=0, bool ignore_failure = true);
int WriteFile(string filename, string data);

// Package type definition (also defines repository type)

unsigned int CheckFileType(string fname);
#endif
