/*******************************************************
 * File operations
 * $Id: file_routines.cpp,v 1.47 2007/11/02 17:45:45 i27249 Exp $
 * ****************************************************/

#include "file_routines.h"
#include <assert.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fstream>
#include <iostream>
vector<string> temp_files;
extern int errno;
vector<string> getDirectoryList(string directory_name)
{
	vector<string> ret;
	DIR *patchDir = opendir(directory_name.c_str());
	if (!patchDir) {
		mWarning("Folder doesn't exist");
		return ret;
	}
	struct dirent *dEntry = readdir(patchDir);
	string d;
	while (dEntry != NULL) {
		d = (string) dEntry->d_name;
		if (d!="." && d!="..") {
			printf("Entry: %s\n", d.c_str());
			ret.push_back(d);
		}
		dEntry = readdir(patchDir);
	}
	closedir(patchDir);
	return ret;
}

bool isProcessRunning(string pid)
{
	string tmp_file = get_tmp_file();
	system("ps -p " + pid + " > " + tmp_file);
	string result = ReadFile(tmp_file);
	if (result.find(pid)!=std::string::npos)
	{
		// Process is running
		return true;
	}
	else return false;
}
bool lockDatabase()
{
	// Lock file will be /var/run/mpkg.lock
	if (FileExists("/var/run/mpkg.lock"))
	{
		mError("Database is already locked by process " + ReadFile("/var/run/mpkg.lock"));
		return false;
	}
	WriteFile("/var/run/mpkg.lock", IntToStr(getpid()));
	return true;
}
bool isDirectory(string dir_name)
{
	struct stat fStat;
	stat(dir_name.c_str(), &fStat);
	if (S_ISDIR(fStat.st_mode)) return true;
	else return false;
}

bool unlockDatabase()
{
	if (FileExists("/var/run/mpkg.lock"))
	{
		if (ReadFile("/var/run/mpkg.lock")==IntToStr(getpid()))
		{
			// Mean that the database is locked by this process
			unlink("/var/run/mpkg.lock");
			return true;
		}
		else {
			//mError("Database is locked by another process, cannot unlock");
			return false;
		}
	}
	else {
		return true;
	}
}

bool isDatabaseLocked()
{
	if (FileExists("/var/run/mpkg.lock"))
	{
		// Check if the process created this id is alive
		if (!isProcessRunning(ReadFile("/var/run/mpkg.lock"))) {
			unlink("/var/run/mpkg.lock");
			return false;
		}
		mError("Database is locked by process " + ReadFile("/var/run/mpkg.lock"));
		return true;
	}
	else return false;
}
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
long double get_disk_freespace(string point)
{
	mDebug("Checking free space");
	struct statfs buf;
	int s_ret = statfs(point.c_str(), &buf);
	if (s_ret!=0)
	{
		mError("Unable to determine FS parameters of " + point);
		return 0;
	}
	long double dfree;
        dfree	= (long double) buf.f_bfree * (long double) buf.f_bsize;
	mDebug("Free on " + point + ": " + IntToStr((long long) dfree));
	return (long double) dfree;
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
	char *_c_md5 = (char *) malloc(1000);
	memset(_c_md5, 0, 1000);
	fscanf(md5, "%s", _c_md5);
	string md5str;
	md5str= (string) _c_md5;
	fclose(md5);
	free(_c_md5);
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
	mDebug("Created temp file " + tmp_fname);
	return tmp_fname;
}

void delete_tmp_files()
{
	mDebug("Deleting temporary files");
	string fname;
	for ( unsigned int i = 0; i < temp_files.size(); i++ ) {
		fname=temp_files[i]+".gz";
		unlink(fname.c_str());
		mDebug("removing " + temp_files[i]);
		unlink( temp_files[i].c_str() );
	}
	temp_files.clear(); // Clean-up list - for future use
}

bool FileExists(string filename, bool *broken_symlink)
{
	
	mDebug("checking the existance of " + filename);
	if (broken_symlink != NULL) *broken_symlink=false;
	if (access(filename.c_str(), F_OK)==0) {
		mDebug("File exists, returning true"); 
		return true; 
	}
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
	mDebug("File [" + filename + "] doesn't exist");
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
		

string ReadFile(string filename) // Reads the text file
{
	mDebug("Reading file " + filename);
	string ret;

	ifstream filestr;
	long size;
	filebuf *pbuf;
	char * buffer;

	mDebug("Opening file");
	filestr.open(filename.c_str());
	if (!filestr.is_open()) {
		mDebug("File not found");
		return "";
	}
	mDebug("Reading file");
	pbuf=filestr.rdbuf();
	size=pbuf->pubseekoff(0,ios::end,ios::in);
	pbuf->pubseekpos(0,ios::in);
	buffer = new char[size+1];
	pbuf->sgetn(buffer,size);
	filestr.close();
//	cout.write(buffer,size);
	mDebug("Returning data");
	// Terminating the string
	buffer[size]=0;
	ret = (string) buffer;
	delete[] buffer;
	return ret;
	/*



	mDebug("Reading file " + filename);
	struct stat fStat;
	if (stat(filename.c_str(), &fStat)!=0)
	{
		//perror("ReadFile");
		mDebug("No such file "+filename);
		return "";
	}
	else mDebug("File exist, reading");
	long size=fStat.st_size;
	char *memblock = new char [size];
	

	string ret;
	ifstream inputFile(filename.c_str(), ios::in|ios::ate);
	if (!inputFile.is_open()) {delete[] memblock; return "";}
	else
	{
		inputFile.seekg(0, ios::beg);
		inputFile.get(memblock, size);
		inputFile.close();
		ret = (string) memblock;
		if (max_count > 0) ret = ret.substr(0, max_count);
		delete[] memblock;
		mDebug("Returning data of " + filename);
		return ret;
	}*/
}

#ifdef NEW_RS
vector<string> ReadFileStringsN(string filename)
{
	mDebug("Trying to read the strings from "+filename);
	return 
}
#endif

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
int extractFromTar(string filename, string file_to_extract, string output)
{
	string cmd = "tar xf "+filename+" "+ file_to_extract + " --to-stdout > " + output + " 2>/dev/null";
	return system(cmd.c_str());
}
int extractFromTbz2(string filename, string file_to_extract, string output)
{
	string cmd = "tar jxf "+filename+" "+ file_to_extract + " --to-stdout > " + output + " 2>/dev/null";
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
bool isMounted(string mountpoint)
{
	if (mountpoint.find_last_of("/")>=mountpoint.length()-1) mountpoint = mountpoint.substr(0,mountpoint.length()-1);
//	mountpoint = mountpoint.substr(0,find_last_of("/")-1);
	mDebug("Checking if [" + mountpoint + "] is mounted");
#ifdef _MNTENTMCHECK
	// First, check if device mounted in proper directory. 
	bool mounted=false;
	struct mntent *mountList;
	FILE *mtab = fopen("/proc/mounts", "r");
	//char volname[2000];
	if (mtab)
	{
		mountList = getmntent(mtab);
		while ( !mounted && mountList != NULL )
		{
			if (strcmp(mountList->mnt_dir, mountpoint.c_str())==0)
			{
				/*if (strcmp(mountList->mnt_dir, CDROM_DEVICE.c_str())!=0)
				{
					umount(mountpoint.c_str());
				}
				else*/ mounted = true;
			}
			mountList = getmntent(mtab);
		}
		fclose(mtab);
	}
	if (mounted) mDebug(mountpoint + " is mounted");
	else mDebug(mountpoint + " isn't mounted");
	return mounted;
#else
	string out = get_tmp_file();
	system("cat /proc/mounts | grep " + mountpoint + " | wc -l >" + out );
	string ret = ReadFile(out);
	if (ret[0]=='0') {
		mDebug(mountpoint + " isn't mounted");
		return false;
	}
	else {
		mDebug(mountpoint + " is already mounted");
		return true;
	}
#endif

}

string getCdromVolname(string *rep_location)
{
	mDebug("checking in location " + CDROM_MOUNTPOINT);
	bool hasMountedHere=false;
	if (!isMounted(CDROM_MOUNTPOINT))
	{
		system("mount " + CDROM_DEVICE + " " + CDROM_MOUNTPOINT);
		hasMountedHere=true;
	}
	string Svolname, repLoc;
	// check_volname:
	if (FileExists(CDROM_MOUNTPOINT + "/.volume_id")) Svolname = cutSpaces(ReadFile(CDROM_MOUNTPOINT + "/.volume_id"));
	if (rep_location!=NULL)
	{
		if (FileExists(CDROM_MOUNTPOINT + "/.repository")) repLoc = cutSpaces(ReadFile(CDROM_MOUNTPOINT + "/.repository"));
	}
	if (hasMountedHere) system("umount " + CDROM_MOUNTPOINT + " 2>/dev/null");
	// Validating
	if (Svolname.find_first_of("\n\t/><| !@#$%%^&*()`\"\'")!=std::string::npos)
	{
		mError("Invalid volname [" + Svolname+"]");
		return "";
	}
	if (rep_location!=NULL)
	{
		if (repLoc.find_first_of("<>|!@#$%^&*()`\"\'")!=std::string::npos)
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
	mDebug("Caching index for ["+vol_id+"] in with location [" + rep_location+"]");
	if (system("mkdir -p /var/mpkg/index_cache/"+vol_id)==0 && system("cp -f /var/log/mount/" +rep_location+"/packages.xml.gz /var/mpkg/index_cache/"+vol_id)==0) return true;
	return false;
}

bool copyFile(string source, string destination)
{
	if (system("cp -f " + source + " " + destination)==0) return true;
	else return false;
}
