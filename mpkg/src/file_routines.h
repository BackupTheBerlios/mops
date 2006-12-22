/******************************************************
* File operations
* $Id: file_routines.h,v 1.4 2006/12/22 10:26:05 adiakin Exp $
* ****************************************************/
#ifndef FILE_ROUTINES_H_
#define FILE_ROUTINES_H_

#include "local_package.h"
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
string ReadFile(string filename, int max_count=0);
int WriteFile(string filename, string data);

#endif
