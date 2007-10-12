/******************************************************
* File operations
* $Id: file_routines.h,v 1.21 2007/10/12 19:14:18 i27249 Exp $
* ****************************************************/
#ifndef FILE_ROUTINES_H_
#define FILE_ROUTINES_H_
#include <string>
#include <vector>
#include <math.h>
#include "config.h"
#include "dataunits.h"
#include "string_operations.h"
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

bool isMounted(string mountpoint);
bool lockDatabase();
bool unlockDatabase();
bool isDatabaseLocked();
int system(string cmd);
bool backupDatabase();
bool restoreDatabaseFromBackup();
long double get_disk_freespace(string point="/");
string get_file_md5(string filename);
string get_tmp_file();
void delete_tmp_files();
bool FileExists(string filename, bool *broken_symlink=NULL);
bool FileNotEmpty(string filename);
string ReadFile(string filename);
int WriteFile(string filename, string data);
int extractFromTgz(string filename, string file_to_extract, string output);
string getCdromVolname(string *rep_location=NULL);
bool cacheCdromIndex(string vol_id, string rep_location);
// Package type definition (also defines repository type)
vector<string>ReadFileStrings(string filename);
unsigned int CheckFileType(string fname);
#endif
