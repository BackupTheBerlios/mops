/*
	MOPSLinux packaging system
	Data types descriptions
	$Id: dataunits.h,v 1.2 2006/12/17 19:34:57 i27249 Exp $
*/


#ifndef DATAUNITS_H_
#define DATAUNITS_H_

#include <string>
#include <vector>
using namespace std;

string PrepareSql(string str);

class SERVER_TAG
{
    private:
	// INTERNAL DATA //
	int server_tag_id;
	string server_tag_name;

    public:
	// Methods
	int get_id();
	string get_name(bool sql=true);

	int set_id(int id);
	int set_name(string name);
	
	// CLASS ROUTINES //
	bool IsEmpty();
	SERVER_TAG();
	~SERVER_TAG();
};

class SERVER_TAG_LIST
{
    private:
	vector <SERVER_TAG> server_tags;
    public:
	int add(SERVER_TAG server_tag);
	void clear();
	bool IsEmpty();
	int size();

	SERVER_TAG* get_server_tag(int num);
	int set_server_tag(int num, SERVER_TAG serv_tag);

	SERVER_TAG_LIST();
	~SERVER_TAG_LIST();
};

class SERVER
{

#define SRV_FILE 1
#define SRV_CACHE 2
#define SRV_CDROM 3
#define SRV_HTTP 4
#define SRV_FTP 5
#define SRV_SMB 6
#define SRV_HTTPS 7
#define SRV_BADURL 8

    private:
	// INTERNAL DATA //
	int server_id;
	string server_url;
	string server_priority;
	
	// EXTERNAL DATA //
	SERVER_TAG_LIST server_tags;

    public:
	int get_id();
	string get_url(bool sql=true);
	int get_type();
	string get_priority(bool sql=true);
	SERVER_TAG_LIST* get_tags();

	int set_id(int id);
	int set_url(string url);
	int set_priority(string priority);
	int set_tags(SERVER_TAG_LIST tags);

	// CLASS ROUTINES //
	bool IsEmpty();
	SERVER();
	~SERVER();
};

class SERVER_LIST
{
    private:
	vector <SERVER> servers;
	
    public:
	SERVER* get_server(int num);
	int set_server(int num, SERVER server);

	int add(SERVER server);
	void clear();
	bool IsEmpty();
	int size();

	SERVER_LIST();
	~SERVER_LIST();
};

class LOCATION
{
    private:
	// INTERNAL DATA //
	long location_id;
	string location_path;
	bool local;

	// EXTERNAL DATA //
	SERVER server;
	
    public:
	long get_id();
	string get_path(bool sql=true);
	void set_local();
	bool get_local();
	void unset_local();

	int set_id(long id);
	int set_path(string path);

	SERVER* get_server();
	int set_server(SERVER new_server);

	// CLASS ROUTINES //
	void destroy();
	bool IsEmpty();
	LOCATION();
	~LOCATION();
};

class LOCATION_LIST
{
    private:
	vector<LOCATION> locations;
    public:
	LOCATION* get_location(int num);
	int set_location(int num, LOCATION location);

	int add(LOCATION location);
	void clear();
	bool IsEmpty();
	int size();

	LOCATION_LIST();
	~LOCATION_LIST();
};

class DEPENDENCY
{
    private:
	// INTERNAL DATA //
	int dependency_id;
	string dependency_condition;
	string dependency_type;
	string dependency_package_name;
	string dependency_package_version;
	int dependency_broken;
    public:
	int get_id();
	string get_condition(bool sql=true);
	string get_vcondition();
	string get_type(bool sql=true);
	string get_package_name(bool sql=true);
	string get_package_version(bool sql=true);
	string get_vbroken();
	int get_broken();

	int set_id(int id);
	int set_condition(string condition);
	int set_type(string type);
	int set_package_name(string package_name);
	int set_package_version(string package_version);
	int set_broken(int broken);

	// CLASS ROUTINES //
	bool IsEmpty();
	void clear();
	DEPENDENCY();
	~DEPENDENCY();
};

class TAG
{
    private:
	// INTERNAL DATA //
	int tag_id;
	string tag_name;

    public:
	int get_id();
	string get_name(bool sql=true);
	int set_id(int id);
	int set_name(string name);

	// CLASS ROUTINES //
	void clear();
	bool IsEmpty();
	TAG();
	~TAG();
};

class FILES
{
    private:
	// INTERNAL DATA //
	int file_id;
	string file_name;
	string file_size;

	// EXTERNAL DATA //
	//PACKAGE_LIST *file_packages;
    public:
	int get_id();
	string get_name(bool sql=true);
	string get_size(bool sql=true);

	int set_id(int id);
	int set_name(string name);
	int set_size(string size);

	// CLASS ROUTINES //
	bool IsEmpty();
	FILES();
	~FILES();
};

class TAG_LIST
{
    private:
	vector <TAG> tags;
    public:
	TAG* get_tag(int num);
	int set_tag(int num, TAG tag);

	int add(TAG tag);
	void clear();
	bool IsEmpty();
	int size();

	TAG_LIST();
	~TAG_LIST();
};

class DEPENDENCY_LIST
{
    private:
	vector <DEPENDENCY> dependencies;
    public:
	DEPENDENCY* get_dependency(int num);
	int set_dependency(int num, DEPENDENCY dependency);

	int add(DEPENDENCY dependency);
	void clear();
	bool IsEmpty();
	int size();

	DEPENDENCY_LIST();
	~DEPENDENCY_LIST();
};

class FILE_LIST
{
    private:
	vector <FILES> files;
    public:
	FILES* get_file(int num);
	int set_file(int num, FILES file);
	
	int add(FILES file);
	int add_list(FILE_LIST filelist);
	void clear();
	bool IsEmpty();
	int size();
	void destroy();

	FILE_LIST();
	~FILE_LIST();
};

class PACKAGE
{
    private:
	// INTERNAL DATA //
	int package_id;
	string package_name;
	string package_version;
	string package_arch;
	string package_build;
	string package_compressed_size;
	string package_installed_size;
	string package_short_description;
	string package_description;
	string package_changelog;
	string package_packager;
	string package_packager_email;
	int package_status;
	string package_md5;
	string package_filename;
    public:
	int get_id();
	string get_name(bool sql=true);
	string get_version(bool sql=true);
	string get_arch(bool sql=true);
	string get_build(bool sql=true);
	string get_compressed_size(bool sql=true);
	string get_installed_size(bool sql=true);
	string get_short_description(bool sql=true);
	string get_description(bool sql=true);
	string get_changelog(bool sql=true);
	string get_packager(bool sql=true);
	string get_packager_email(bool sql=true);
	int get_status();
	string get_md5(bool sql=true);
	string get_filename(bool sql=true);

	int set_id(int id);
	int set_name(string name);
	int set_version(string version);
	int set_arch(string arch);
	int set_build(string build);
	int set_compressed_size(string compressed_size);
	int set_installed_size(string installed_size);
	int set_short_description(string short_description);
	int set_description(string description);
	int set_changelog(string changelog);
	int set_packager(string packager);
	int set_packager_email(string packager_email);
	int set_status(int status);
	int set_md5(string md5);
	int set_filename(string filename);
    private:
	// EXTERNAL DATA //
	FILE_LIST package_files;
	LOCATION_LIST package_locations;
	DEPENDENCY_LIST package_dependencies;
	TAG_LIST package_tags;
    public:
	FILE_LIST* get_files();
	LOCATION_LIST* get_locations();
	DEPENDENCY_LIST* get_dependencies();
	TAG_LIST* get_tags();

	int set_files(FILE_LIST files);
	int set_locations(LOCATION_LIST locations);
	int set_dependencies(DEPENDENCY_LIST dependencies);
	int set_tags(TAG_LIST tags);

	int add_dependency(string package_name, string dep_condition, string package_version);
	int add_file(string file_name, string file_size);
	int add_tag(string tag);

	/*FILES get_package_file(int num);
	LOCATIONS get_package_location(int num);
	DEPENDENCY get_dependency(int num);
	TAG get_tag(int num);*/


	// CLASS ROUTINES //
	void destroy();
	bool IsEmpty();
	PACKAGE();
	~PACKAGE();
};

class PACKAGE_LIST
{
    private:
	vector<PACKAGE> packages;
    public:
	PACKAGE* get_package(int num);
	int set_package(int num, PACKAGE package);
	
	int add(PACKAGE package);
	void clear();
	bool IsEmpty();
	int size();
	PACKAGE findMaxVersion();
	DEPENDENCY_LIST getDepList(int i);
	void destroy();
	PACKAGE_LIST();
	~PACKAGE_LIST();
};

typedef int RESULT;
string IntToStr(int num);

#endif //DATAUNITS_H_

