/*******************************************************
 * File operations
 * $Id: file_routines.cpp,v 1.35 2007/07/20 11:14:16 i27249 Exp $
 * ****************************************************/

#include "file_routines.h"
#include <assert.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fstream>
vector<string> temp_files;
extern int errno;

bool backupDatabase()
{
	return true; // Backup is disabled for now

	string cmd = "cp -f " + DB_FILENAME + " " + DB_FILENAME+".backup 2>&1 >/dev/null";
	mDebug(cmd);
	if (system(cmd)==0) return true;
	else return false;
}

bool restoreDatabaseFromBackup()
{
	return true; // Backup is disabled for now
	mDebug("restoring");
	if (system("cp -f " + DB_FILENAME + ".backup " + DB_FILENAME + " 2>&1 >/dev/null")==0) return true;
	else return false;
}

int system(string cmd)
{
	int ret = system(cmd.c_str());
	mDebug("[returned " + IntToStr(ret) + "] " + cmd);
	return ret;
}
double get_disk_freespace(string point)
{
	struct statfs buf;
	int s_ret = statfs(point.c_str(), &buf);
	if (s_ret!=0)
	{
		mError("Unable to determine FS parameters of " + point);
		return 0;
	}
	printf("f_bsize = %d, f_bfree = %ld\n", buf.f_bsize, buf.f_bfree);
	long double dfree;
       dfree	= (long double) buf.f_bfree * (long double) buf.f_bsize;
       printf("%Lf\n", dfree);
	mDebug("Free on " + point + ": " + IntToStr((long long) dfree));
	return (double) dfree;
}


string get_file_md5(string filename)
{
	string tmp_md5=get_tmp_file();

	string sys="md5sum "+filename+" > "+tmp_md5 + " 2>/dev/null";
	system(sys.c_str());
	FILE* md5=fopen(tmp_md5.c_str(), "r");
	if (!md5)
	{
		mError("Unable to open md5 temp file");
		return "";
	}
	char _c_md5[1000];
	memset(&_c_md5, 0, sizeof(_c_md5));
	fscanf(md5, "%s", &_c_md5);
	string md5str;
	md5str=_c_md5;
	fclose(md5);
	unlink(tmp_md5.c_str());
	return md5str;
}


string get_tmp_file()
{
	string tmp_fname;
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
	temp_files.push_back(tmp_fname);
	close(fd);
	return tmp_fname;
}

void delete_tmp_files()
{
	string fname;
	for ( unsigned int i = 0; i < temp_files.size(); i++ ) {
		fname=temp_files[i]+".gz";
		unlink(fname.c_str());
		unlink( temp_files[i].c_str() );
	}
	temp_files.clear(); // Clean-up list - for future use
}

bool FileExists(string filename, bool *broken_symlink)
{
	if (broken_symlink != NULL) *broken_symlink=false;
	if (access(filename.c_str(), F_OK)==0) return true;
	else 
	{
		struct stat st;
		lstat(filename.c_str(), &st);
		//{
		//	printf("NOT EXIST AT ALL");
		//}
		//else
		//{
			if (S_ISLNK(st.st_mode) && broken_symlink!=NULL) *broken_symlink=true;
		//}
	}
	return false;
}

bool FileNotEmpty(string filename)
{
	ifstream test(filename.c_str(), ios::in);
	ifstream::pos_type nullsize;
	if (test.is_open())
	{
		nullsize = test.tellg();
		test.close();
	}
	else {
		return false;
	}
	ifstream test2(filename.c_str(), ios::in | ios::ate);
	ifstream::pos_type endsize;
	if (test2.is_open())
	{
		endsize = test2.tellg();
		test2.close();
	}
	else {
       	       return false;
	}
	if (endsize == nullsize)
	{
		return false;
	}
	else
	{
		return true;
	}
}
		

string ReadFile(string filename, int max_count)
{
	mDebug("Reading file");
	struct stat fStat;
	if (stat(filename.c_str(), &fStat)!=0)
	{
		//perror("ReadFile");
		mDebug("No such file");
		return "";
	}
	long size=fStat.st_size;
	char *memblock = new char [size];

	string ret;
	ifstream inputFile(filename.c_str(), ios::in|ios::ate);
	if (!inputFile.is_open()) return "";
	else
	{
		inputFile.seekg(0, ios::beg);
		inputFile.read(memblock, size);
		inputFile.close();
		ret = (string) memblock;
		if (max_count > 0) ret = ret.substr(0, max_count);
		delete memblock;
		mDebug("Returning data");
		return ret;
	}
}

vector<string> ReadFileStrings(string filename)
{
	mDebug("Reading strings...");
	string data=ReadFile(filename);
	vector<string> ret;
	
	string chunk;

	for (unsigned int lim=data.find_first_of("\n"); lim!=std::string::npos; lim=data.find_first_of("\n"))
	{
		chunk = data.substr(0,lim);
		ret.push_back(chunk);
		if (lim<data.size()) data = data.substr(lim+1);
		else data="";
	}
	mDebug("Returning strings vector...");
	return ret;
}
int extractFromTgz(string filename, string file_to_extract, string output)
{
	string cmd = "tar zxf "+filename+" "+ file_to_extract + " --to-stdout > " + output + " 2>/dev/null";
	return system(cmd.c_str());
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
		mError("Unable to write file " + filename);
		errRet = waitResponce(MPKG_SUBSYS_FILE_WRITE_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
			goto write_file;

		abort();
		return -1;
	}
}



unsigned int CheckFileType(string fname)
{
	// Checking file existanse
	struct stat st;
	if (lstat(fname.c_str(), &st) != 0) {
		if ( errno == ENOENT ) {
			mError("file "+fname+" not found");
			return PKGTYPE_UNKNOWN;
		}
	}
	
	if ( !S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode) ) {
		mError("Not a regular file");
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
		mDebug("Tar returns "+IntToStr(tar_ret));
		if (FileNotEmpty(contCheck))
		{
			unlink(contCheck.c_str());
			return PKGTYPE_MOPSLINUX;
		}
		else
		{
			unlink(contCheck.c_str());
			return PKGTYPE_SLACKWARE;
		}
	}
	if (ext==".deb")
	{
		mDebug("Debian package detected");
		return PKGTYPE_DEBIAN;
	}
	if (ext ==".rpm")
	{
		mDebug("RPM package detected");
		return PKGTYPE_RPM;
	}
	mDebug("Unknown package type "+ext);
	return PKGTYPE_UNKNOWN;
}
string getCdromVolname(string *rep_location)
{
	string Svolname, repLoc;
	// check_volname:
	Svolname = ReadFile(CDROM_MOUNTPOINT + "/.volume_id");
	if (rep_location!=NULL)
	{
		repLoc = ReadFile(CDROM_MOUNTPOINT + "/.repository");
	}
	// Validating
	if (Svolname.find_first_of("/><| !@#$%^&*()`\"\'")!=std::string::npos)
	{
		mError("Invalid volname " + Svolname);
		return "";
	}
	if (rep_location!=NULL)
	{
		if (repLoc.find_first_of("<>| !@#$%^&*()`\"\'")!=std::string::npos || !FileExists(CDROM_MOUNTPOINT+repLoc+"/packages.xml.gz"))
		{
			mError("Invalid repository path");
			return "";
		}
		*rep_location = repLoc;
	}
	
	return Svolname;

}

bool cacheCdromIndex(string vol_id, string rep_location)
{
	if (system("mkdir -p /var/mpkg/index_cache/"+vol_id) && system("cp -f " + CDROM_MOUNTPOINT + "/"+rep_location+"/packages.xml.gz /var/mpkg/index_cache/"+vol_id)) return true;
	return false;
}


	
