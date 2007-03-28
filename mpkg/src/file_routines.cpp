/*******************************************************
 * File operations
 * $Id: file_routines.cpp,v 1.11 2007/03/28 14:39:58 i27249 Exp $
 * ****************************************************/

#include "file_routines.h"
#include <assert.h>
#include <sys/stat.h>

vector<string> temp_files;
extern int errno;

string get_tmp_file()
{
	string tmp_fname;
	debug("get_tmp_file start");
	//char *t=tmpnam(NULL);
	char t[]="/tmp/mpkg-XXXXXX";
	int fd;
	mpkgErrorReturn errRet;
create_tmp:
	fd=mkstemp(t);
	if ( t == NULL  )
	{
		errRet = waitResponce(MPKG_SUBSYS_TMPFILE_CREATE_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
			goto create_tmp;
		if (errRet == MPKG_RETURN_ABORT)
			return NULL;
	}

	tmp_fname=t;
	debug("get_tmp_file end");
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
			printf("cannot delete temp file %s\n", temp_files[i].c_str());
			debug( temp_files[i] );
			debug("\n");
			perror( strerror( errno ) );

		}		
	}

	temp_files.clear(); // Clean-up list - for future use
	printf("temp directory cleaned up\n");
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
		

string ReadFile(string filename, int max_count, bool ignore_failure)
{
	//printf("ReadFILE: %s\n", filename.c_str());
	FILE *src;
	string ret;
	char buf;
	mpkgErrorReturn errRet;
read_file:

	src=fopen(filename.c_str(),"r");
	buf='a';
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
		if (!ignore_failure)
		{
			errRet = waitResponce ( MPKG_SUBSYS_FILE_READ_ERROR);
			if (errRet == MPKG_RETURN_RETRY)
			{
				goto read_file;
			}
		}

		return "";
	}
}

int WriteFile(string filename, string data)
{
	mpkgErrorReturn errRet;
write_file:
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
		printf("Unable to write file %s\n", filename.c_str());
		errRet = waitResponce(MPKG_SUBSYS_FILE_WRITE_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
			goto write_file;

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
		int tar_ret = system(check.c_str());
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
	if (ext ==".rpm")
	{
		debug("RPM package detected");
		return PKGTYPE_RPM;
	}
	debug("Unknown package type "+ext);
	return PKGTYPE_UNKNOWN;
}

