/******************************************************
* File operations
* $Id: file_routines.h,v 1.1 2006/12/20 20:00:38 i27249 Exp $
* ****************************************************/
#ifndef FILE_ROUTINES_H_
#define FILE_ROUTINES_H_

#include "local_package.h"
#include <unistd.h>
#include <errno.h>

string get_tmp_file();
void delete_tmp_files();
bool FileExists(string filename);
string ReadFile(string filename);
int WriteFile(string filename, string data);

#endif
