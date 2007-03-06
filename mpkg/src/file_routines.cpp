/*******************************************************
 * File operations
 * $Id: file_routines.cpp,v 1.9 2007/03/06 01:01:43 i27249 Exp $
 * ****************************************************/

#include "file_routines.h"
#include <assert.h>

vector<string> temp_files;
extern int errno;

string get_tmp_file()
{
	string tmp_fname;
	debug("get_tmp_file start");
	//char *t=tmpnam(NULL);
	char t[]="/tmp/mpkg-XXXXXX";
	int fd;
	fd=mkstemp(t);

#ifdef DEBUG
	printf("file_routines.cpp: Temp filename: %s, return value = %i\n", t, fd);
#endif
	if ( t == NULL  )
		return NULL;

	tmp_fname=t;
	debug("get_tmp_file end");
	//free(t);
	temp_files.resize(temp_files.size()+1);
	temp_files[temp_files.size()-1]=tmp_fname;
	close(fd);
	return tmp_fname;
}

void delete_tmp_files()
{
	debug("preparing to remove temp files");

	for ( unsigned int i = 0; i < temp_files.size(); i++ ) {
		if ( unlink( temp_files[i].c_str() ) != 0 ) {
			debug("cannot delete temp file ");
			debug( temp_files[i] );
			debug("\n");
			perror( strerror( errno ) );

		}		
	}

	temp_files.clear(); // Clean-up list - for future use
}

bool FileExists(string filename)
{
	if (access(filename.c_str(), R_OK)==0) return true;
	else return false;
}

bool FileNotEmpty(string filename)
{
	if (!ReadFile(filename, 4).empty()) return true;
	else return false;
}
		

string ReadFile(string filename, int max_count)
{
	//printf("ReadFILE: %s\n", filename.c_str());
	FILE *src=fopen(filename.c_str(),"r");
	string ret;
	char buf='a';
	ret.clear();
	if (src)
	{
		//printf("[%s]\n",ret.c_str());
		
		for (int i=0;buf!=EOF;i++)
		{
			buf=fgetc(src);
			if (buf!=EOF) ret+=buf;
			if (i==max_count-1) break; // This will stop reading if we reach max_count of bytes read. Also, it will continue until end if max_count==0.
		}
		fclose(src);
		return ret;
	}
	else
	{
		perror("file_routines.cpp: ReadFile()");
		printf("Cannot open file %s\n", filename.c_str());
		return "";
	}
}

int WriteFile(string filename, string data)
{
	FILE* output=fopen(filename.c_str(),"w");
	if (output)
	{
		for (unsigned int i=0;i<data.length();i++)
		{
			fputc(data[i],output);
		}
		fclose(output);
		return 0;
	}
	else
	{
		//assert(src != NULL);
		
		//fclose(output);
		printf("Unable to write file %s\n", filename.c_str());
		abort();
		return 1;
	}
}



unsigned int CheckFileType(string fname)
{
	// Checking file existanse
	struct stat st;
	if (lstat(fname.c_str(), &st) != 0) {
		if ( errno == ENOENT ) {
			fprintf(stderr, _("file %s not found\n"), fname.c_str());
			return PKGTYPE_UNKNOWN;
		}
	}
	
	if ( !S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode) ) {
		fprintf(stderr, _("Not a regular file\n"));
		return PKGTYPE_UNKNOWN;
	}

	// Checking extensions and (partially) contents
	string ext;
	for (unsigned int i=fname.length()-4; i<fname.length(); i++)
	{
		ext+=fname[i];
	}
	if (ext==".tgz")
	{
		// Is there a install/data.xml file?
		string contCheck = get_tmp_file();
		string check = "tar ztf "+fname+" install/data.xml > "+contCheck+" 2>/dev/null";
		int tar_rat = system(check.c_str());
		debug("Tar returns "+IntToStr(tar_ret));
		if (FileNotEmpty(contCheck))
		{
			return PKGTYPE_MOPSLINUX;
		}
		else
		{
			return PKGTYPE_SLACKWARE;
		}
	}
	if (ext==".deb")
	{
		debug("Debian package detected");
		return PKGTYPE_DEBIAN;
	}
	if (ext = ".rpm")
	{
		debug("RPM package detected");
		return PKGTYPE_RPM;
	}
	debug("Unknown package type "+ext);
	return PKGTYPE_UNKNOWN;
}

