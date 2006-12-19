/*
	MOPSLinux packaging system
	Data types descriptions
	$Id: dataunits.cpp,v 1.3 2006/12/19 17:29:09 i27249 Exp $
*/



#include "dataunits.h"
#include "dependencies.h"
#include "conditions.h"
#include "debug.h"
#include "constants.h"

string PrepareSql(string str)
{
	string ret;
	if (str.empty()) return "0";
	for (int i=0;i<str.length();i++)
	{
		if (str[i]=='\'') ret+="\'";
		ret+=str[i];
	}
	return ret;
}


int SERVER_TAG::get_id()
{
	return server_tag_id;
}

string SERVER_TAG::get_name(bool sql)
{
	if (sql) return PrepareSql(server_tag_name);
	return server_tag_name;
}

int SERVER_TAG::set_id(int id)
{
	server_tag_id=id;
       	return 0;
}

int SERVER_TAG::set_name(string name)
{
	server_tag_name=name;
	return 0;
}

bool SERVER_TAG::IsEmpty()
{
	if (server_tag_name.empty()) return true;
	else return false;
}

SERVER_TAG::SERVER_TAG()
{
	server_tag_id=0;
}
SERVER_TAG::~SERVER_TAG(){}


SERVER_TAG* SERVER_TAG_LIST::get_server_tag(int num)
{
	if (num >=0 && num<size())
	{
		return &server_tags[num];
	}
	else
		return NULL;
}

int SERVER_TAG_LIST::set_server_tag(int num, SERVER_TAG serv_tag)
{
	if (num>=0 && num<size())
	{
		server_tags[num]=serv_tag;
		return 0;
	}
	else return 1;
}

int SERVER_TAG_LIST::size()
{
	return server_tags.size();
}

int SERVER_TAG_LIST::add(SERVER_TAG server_tag)
{
    int ret;
    ret=server_tags.size();
    server_tags.resize(ret+1);
    server_tags[ret]=server_tag;
    return ret;
}

void SERVER_TAG_LIST::clear()
{
	server_tags.resize(0);
}

bool SERVER_TAG_LIST::IsEmpty()
{
	if (server_tags.empty()) return true;
	else return false;
}

SERVER_TAG_LIST::SERVER_TAG_LIST(){}
SERVER_TAG_LIST::~SERVER_TAG_LIST(){}

int SERVER::get_id()
{
	return server_id;
}

string SERVER::get_url(bool sql)
{
	if (sql) return PrepareSql(server_url);
	return server_url;
}

int SERVER::get_type()
{
	string tmp;
	int i;
	i=0;
	debug("get_type()");
	while (i<server_url.length() && \
			(tmp!="file://" || \
			tmp!="cache://" || \
			tmp!="cdrom://" || \
			tmp!="http://" || \
			tmp!="ftp://" || \
			tmp!="smb://" || \
			tmp!="https://") \
	      )
	{
		tmp+=server_url[i];
		debug(tmp);
		i++;
	}
	debug("Server type: "+tmp);

	if (tmp=="file://") return SRV_FILE;
	if (tmp=="cache://") return SRV_CACHE;
	if (tmp=="cdrom://") return SRV_CDROM;
	if (tmp=="http://") return SRV_HTTP;
	if (tmp=="ftp://") return SRV_FTP;
	if (tmp=="smb://") return SRV_SMB;
	if (tmp=="https://") return SRV_HTTPS;

	return SRV_BADURL;
}

string SERVER::get_priority(bool sql)
{
	if (sql) return PrepareSql(server_priority);
	return server_priority;
}

SERVER_TAG_LIST* SERVER::get_tags()
{
	return &server_tags;
}
int SERVER::set_id(int id)
{
	server_id=id;
	return 0;
}

int SERVER::set_url(string url)
{
	server_url=url;
	return 0;
}

int SERVER::set_priority(string priority)
{
	server_priority=priority;
	return 0;
}

int SERVER::set_tags(SERVER_TAG_LIST tags)
{
	server_tags=tags;
}


bool SERVER::IsEmpty()
{
	if (server_url.empty()) return true; // The server without URL means nothing, so we call it empty
	else return false;
}

SERVER::SERVER()
{
	server_id=0;
}
SERVER::~SERVER()
{
}

SERVER* SERVER_LIST::get_server(int num)
{
	if (num<size() && num>=0) return &servers[num];
	else return NULL;
}
int SERVER_LIST::set_server(int num, SERVER server)
{
	if (num>=0 && num<size())
	{
		servers[num]=server;
		return 0;
	}
	else return 1;
}

int SERVER_LIST::size()
{
	return servers.size();
}

int SERVER_LIST::add(SERVER server)
{
    int ret;
    ret=servers.size();
    servers.resize(ret+1);
    servers[ret]=server;
    return ret;
}

void SERVER_LIST::clear()
{
    servers.resize(0);
}

bool SERVER_LIST::IsEmpty()
{
	if (servers.empty()) return true;
	else return false;
}

SERVER_LIST::SERVER_LIST(){}
SERVER_LIST::~SERVER_LIST(){}

SERVER* LOCATION::get_server()
{
	return &server;
}

int LOCATION::set_server(SERVER new_server)
{
	server=new_server;
	return 0;
}

long LOCATION::get_id()
{
	return location_id;
}

string LOCATION::get_path(bool sql)
{
	if (sql) return PrepareSql(location_path);
	return location_path;
}

int LOCATION::set_id(long id)
{
	location_id=id;
	return 0;
}

int LOCATION::set_path(string path)
{
	location_path=path;
	return 0;
}

void LOCATION::destroy()
{
}
bool LOCATION::IsEmpty()
{
	if (location_path.empty()) return true;
	else return false;
}

void LOCATION::set_local()
{
	local=true;
}

void LOCATION::unset_local()
{
	local=false;
}

bool LOCATION::get_local()
{
	return local;
}


LOCATION::LOCATION()
{
	local=false;
	location_id=0;
}
LOCATION::~LOCATION()
{
}

LOCATION* LOCATION_LIST::get_location(int num)
{
	if (num>=0 && num<size()) return &locations[num];
	else return NULL;
}

int LOCATION_LIST::set_location(int num, LOCATION location)
{
	if (num>=0 && num<size())
	{
		locations[num]=location;
		return 0;
	}
	else return 1;
}

int LOCATION_LIST::size()
{
	return locations.size();
}
int LOCATION_LIST::add(LOCATION location)
{
    int ret;
    ret=locations.size();
    locations.resize(ret+1);
    locations[ret]=location;
    return ret;
}

void LOCATION_LIST::clear()
{
    locations.resize(0);
}

bool LOCATION_LIST::IsEmpty()
{
	if (locations.empty()) return true;
	else return false;
}

LOCATION_LIST::LOCATION_LIST(){}
LOCATION_LIST::~LOCATION_LIST(){}

void DEPENDENCY::clear()
{
	dependency_broken=0;
	dependency_package_version.clear();
	dependency_package_name.clear();
	dependency_type.clear();
	dependency_condition.clear();
	dependency_id=0;
}

int DEPENDENCY::get_id()
{
	return dependency_id;
}

string DEPENDENCY::get_condition(bool sql)
{
	if (sql) return PrepareSql(dependency_condition);
	return dependency_condition;
}

string DEPENDENCY::get_vcondition()
{
	if (dependency_condition==IntToStr(VER_MORE)) return ">";
	if (dependency_condition==IntToStr(VER_LESS)) return "<";
	if (dependency_condition==IntToStr(VER_EQUAL)) return "==";
	if (dependency_condition==IntToStr(VER_NOTEQUAL)) return "!=";
	if (dependency_condition==IntToStr(VER_XMORE)) return ">=";
	if (dependency_condition==IntToStr(VER_XLESS)) return "<=";
	return "(unknown condition)";
}

string DEPENDENCY::get_type(bool sql)
{
	if (sql) return PrepareSql(dependency_type);
	return dependency_type;
}

string DEPENDENCY::get_package_name(bool sql)
{
	if (sql) return PrepareSql(dependency_package_name);
	return dependency_package_name;
}

string DEPENDENCY::get_package_version(bool sql)
{
	if (sql) return PrepareSql(dependency_package_version);
	return dependency_package_version;
}

int DEPENDENCY::get_broken()
{
	return dependency_broken;
}

string DEPENDENCY::get_vbroken()
{
	if (dependency_broken==DEP_FILECONFLICT) return "Conflicting files found";
	if (dependency_broken==DEP_DBERROR) return "Database error";
	if (dependency_broken==DEP_UNAVAILABLE) return "Package unavailable";
	if (dependency_broken==DEP_BROKEN) return "(deprecated_error) Dependency is broken, see sources for details";
	if (dependency_broken==DEP_CHILD) return "Child dependency is broken";
	if (dependency_broken==DEP_VERSION) return "Required version not found";
	if (dependency_broken==DEP_CONFLICT) return "Dependency conflict, unable to resolve";
	if (dependency_broken==DEP_NOTFOUND) return "Required package not found";
	if (dependency_broken==DEP_OK) return "OK";
	return "Dependency NOT SET!";
}

int DEPENDENCY::set_id(int id)
{
	dependency_id=id;
	return 0;
}

int DEPENDENCY::set_condition(string condition)
{
	dependency_condition=condition;
	return 0;
}

int DEPENDENCY::set_type(string type)
{
	dependency_type=type;
	return 0;
}

int DEPENDENCY::set_package_name(string package_name)
{
	dependency_package_name=package_name;
	return 0;
}

int DEPENDENCY::set_package_version(string package_version)
{
	dependency_package_version=package_version;
	return 0;
}

int DEPENDENCY::set_broken(int broken)
{
	dependency_broken=broken;
	return 0;
}

bool DEPENDENCY::IsEmpty()
{
	if (dependency_package_name.empty()) return true;
	else return false;
}

DEPENDENCY::DEPENDENCY()
{
	dependency_id=0;
	dependency_broken=0;
}
DEPENDENCY::~DEPENDENCY(){}

void TAG::clear()
{
	tag_id=0;
	tag_name.clear();
}
int TAG::get_id()
{
	return tag_id;
}

string TAG::get_name(bool sql)
{
	if (sql) return PrepareSql(tag_name);
	return tag_name;
}

int TAG::set_id(int id)
{
	tag_id=id;
	return 0;
}

int TAG::set_name(string name)
{
	tag_name=name;
	return 0;
}

bool TAG::IsEmpty()
{
	if (tag_name.empty()) return true;
	else return false;
}

TAG::TAG()
{
	tag_id=0;
}
TAG::~TAG(){}

int FILES::get_id()
{
	return file_id;
}

string FILES::get_name(bool sql)
{
	if (sql) return PrepareSql(file_name);
	return file_name;
}

string FILES::get_size(bool sql)
{
	if (sql) return PrepareSql(file_size);
	return file_size;
}

int FILES::set_id(int id)
{
	file_id=id;
	return 0;
}

int FILES::set_name(string name)
{
	file_name=name;
	return 0;
}

int FILES::set_size(string size)
{
	file_size=size;
	return 0;
}

bool FILES::IsEmpty()
{
	if (file_name.empty()) return true;
	else return false;
}

FILES::FILES()
{
	file_id=0;
}
FILES::~FILES(){}


TAG* TAG_LIST::get_tag(int num)
{
	if (num>=0 && num<size())
	{
		return &tags[num];
	}
	else return NULL;
}

int TAG_LIST::set_tag(int num, TAG tag)
{
	if (num>=0 && num<size())
	{
		tags[num]=tag;
		return 0;
	}
	else return 1;
}

int TAG_LIST::size()
{
	return tags.size();
}

int TAG_LIST::add(TAG tag)
{
    int ret;
    ret=tags.size();
    tags.resize(ret+1);
    tags[ret]=tag;
    return ret;
}

void TAG_LIST::clear()
{
    tags.clear();
}

bool TAG_LIST::IsEmpty()
{
	if (tags.empty()) return true;
	else return false;
}

TAG_LIST::TAG_LIST(){}
TAG_LIST::~TAG_LIST(){}


DEPENDENCY* DEPENDENCY_LIST::get_dependency(int num)
{
	if (num>=0 && num < size())
	{
		return &dependencies[num];
	}
	else return NULL;
}

int DEPENDENCY_LIST::set_dependency(int num, DEPENDENCY dependency)
{
	if (num>=0 && num<size())
	{
		dependencies[num]=dependency;
		return 0;
	}
	else return 1;
}

int DEPENDENCY_LIST::size()
{
	return dependencies.size();
}

int DEPENDENCY_LIST::add(DEPENDENCY dependency)
{
    int ret;
    ret=dependencies.size();
    dependencies.resize(ret+1);
    dependencies[ret]=dependency;
    return ret;
}

void DEPENDENCY_LIST::clear()
{
    dependencies.resize(0);
}

bool DEPENDENCY_LIST::IsEmpty()
{
	if (dependencies.empty()) return true;
	else return false;
}
DEPENDENCY_LIST::DEPENDENCY_LIST(){}
DEPENDENCY_LIST::~DEPENDENCY_LIST(){}


FILES* FILE_LIST::get_file(int num)
{
	if (num>=0 && num<size())
	{
		return &files[num];
	}
	else return NULL;
}

int FILE_LIST::set_file(int num, FILES file)
{
	if (num>=0 && num<size())
	{
		files[num]=file;
		return 0;
	}
	else return 1;
}

void FILE_LIST::destroy()
{
}

int FILE_LIST::size()
{
	return files.size();
}

int FILE_LIST::add(FILES file)
{
    int ret;
    ret=files.size();
    files.resize(ret+1);
    files[ret]=file;
    return ret;
}

int FILE_LIST::add_list(FILE_LIST filelist)
{
	int ret;
	ret=files.size();
	files.resize(ret+filelist.files.size());
	for (int i=0;i<filelist.files.size();i++)
	{
		files[ret+i]=filelist.files[i];
	}
	return files.size()-1;
}

void FILE_LIST::clear()
{
    files.resize(0);
}

bool FILE_LIST::IsEmpty()
{
	if (files.empty()) return true;
	else return false;
}
FILE_LIST::FILE_LIST(){}
FILE_LIST::~FILE_LIST(){}


int PACKAGE::add_dependency(string package_name, string dep_condition, string package_version)
{
	DEPENDENCY dep;
	dep.set_package_name(package_name);
	dep.set_package_version(package_version);
	dep.set_condition(dep_condition);
	for (int i=0;i<package_dependencies.size();i++)
	{
		if (package_dependencies.get_dependency(i)->get_package_name()==dep.get_package_name() && \
		    package_dependencies.get_dependency(i)->get_package_version()==dep.get_package_version())
		       	return 1;
	}
	package_dependencies.add(dep);
	return 0;
}

int PACKAGE::add_file(string file_name, string file_size)
{
	FILES file;
	file.set_name(file_name);
	file.set_size(file_size);
	package_files.add(file);
	return 0;
}

int PACKAGE::add_tag(string tag)
{
	TAG t;
	t.set_name(tag);
	package_tags.add(t);
	return 0;
}



int PACKAGE::get_id()
{
	return package_id;
}

string PACKAGE::get_name(bool sql)
{
	if (sql) return PrepareSql(package_name);
	return package_name;
}

string PACKAGE::get_version(bool sql)
{
	if (sql) return PrepareSql(package_version);
	return package_version;
}

string PACKAGE::get_arch(bool sql)
{
	if (sql) return PrepareSql(package_arch);
	return package_arch;
}

string PACKAGE::get_build(bool sql)
{
	if (sql) return PrepareSql(package_build);
	return package_build;
}

string PACKAGE::get_compressed_size(bool sql)
{
	if (sql) return PrepareSql(package_compressed_size);
	return package_compressed_size;
}

string PACKAGE::get_installed_size(bool sql)
{
	if (sql) return PrepareSql(package_installed_size);
	return package_installed_size;
}

string PACKAGE::get_short_description(bool sql)
{
	if (sql) return PrepareSql(package_short_description);
	return package_short_description;
}

string PACKAGE::get_description(bool sql)
{
	if (sql) return PrepareSql(package_description);
	return package_description;
}

string PACKAGE::get_changelog(bool sql)
{
	if (sql) return PrepareSql(package_changelog);
	return package_changelog;
}

string PACKAGE::get_packager(bool sql)
{
	if (sql) return PrepareSql(package_packager);
	return package_packager;
}

string PACKAGE::get_packager_email(bool sql)
{
	if (sql) return PrepareSql(package_packager_email);
	return package_packager_email;
}

int PACKAGE::get_status()
{
	return package_status;
}

string PACKAGE::get_md5(bool sql)
{
	if (sql) return PrepareSql(package_md5);
	return package_md5;
}

string PACKAGE::get_filename(bool sql)
{
	if (sql) return PrepareSql(package_filename);
	return package_filename;
}

int PACKAGE::set_id(int id)
{
	package_id=id;
	return 0;
}

int PACKAGE::set_name(string name)
{
	package_name=name;
	return 0;
}

int PACKAGE::set_version(string version)
{
	package_version=version;
	return 0;
}

int PACKAGE::set_arch(string arch)
{
	package_arch=arch;
	return 0;
}

int PACKAGE::set_build(string build)
{
	package_build=build;
	return 0;
}

int PACKAGE::set_compressed_size(string compressed_size)
{
	package_compressed_size=compressed_size;
	return 0;
}

int PACKAGE::set_installed_size(string installed_size)
{
	package_installed_size=installed_size;
	return 0;
}

int PACKAGE::set_short_description(string short_description)
{
	package_short_description=short_description;
	return 0;
}

int PACKAGE::set_description(string description)
{
	package_description=description;
	return 0;
}

int PACKAGE::set_changelog(string changelog)
{
	package_changelog=changelog;
	return 0;
}

int PACKAGE::set_packager(string packager)
{
	package_packager=packager;
	return 0;
}

int PACKAGE::set_packager_email(string packager_email)
{
	package_packager_email=packager_email;
	return 0;
}

int PACKAGE::set_status(int status)
{
	package_status=status;
	return 0;
}

int PACKAGE::set_md5(string md5)
{
	package_md5=md5;
	return 0;
}

int PACKAGE::set_filename(string filename)
{
	package_filename=filename;
	return 0;
}

FILE_LIST* PACKAGE::get_files()
{
	return &package_files;
}

DEPENDENCY_LIST* PACKAGE::get_dependencies()
{
	return &package_dependencies;
}

LOCATION_LIST* PACKAGE::get_locations()
{
	return &package_locations;
}

TAG_LIST* PACKAGE::get_tags()
{
	return &package_tags;
}

int PACKAGE::set_files(FILE_LIST files)
{
	package_files=files;
	return 0;
}

int PACKAGE::set_dependencies(DEPENDENCY_LIST dependencies)
{
	package_dependencies=dependencies;
	return 0;
}

int PACKAGE::set_locations(LOCATION_LIST locations)
{
	package_locations=locations;
	return 0;
}

int PACKAGE::set_tags(TAG_LIST tags)
{
	package_tags=tags;
	return 0;
}


bool PACKAGE::IsEmpty()
{
	if (package_name.empty()) return true;
	else return false;
}

PACKAGE::PACKAGE()
{
	package_id=0;
	package_status=0;
}
PACKAGE::~PACKAGE()
{
}

void PACKAGE::destroy()
{
}
// Helpful function ))
string IntToStr(int num)
{
  char *s=(char *)malloc(2000);
  string ss;
  if (s)
  {
	  sprintf(s,"%d",num);
	  ss=s;
	  free(s);
  }
  else printf("Error: malloc() failed!!!\n");
  return ss;
}


PACKAGE* PACKAGE_LIST::get_package(int num)
{
	//PACKAGE s_package;
	if (num>=0 && num<size())
	{
		return &packages[num];
	}
	else return NULL;
}

int PACKAGE_LIST::set_package(int num, PACKAGE package)
{
	if (num>=0 && num<size())
	{
		packages[num]=package;
		return 0;
	}
	else return 1;
}

int PACKAGE_LIST::size()
{
	return packages.size();
}

int PACKAGE_LIST::add(PACKAGE package)
{
    int ret;
    ret=packages.size();
    packages.resize(ret+1);
    packages[ret]=package;
    return ret;
}

void PACKAGE_LIST::clear()
{
    packages.resize(0);
}

bool PACKAGE_LIST::IsEmpty()
{
	if (packages.empty()) return true;
	else return false;
}

DEPENDENCY_LIST PACKAGE_LIST::getDepList(int i)
{
	return *packages[i].get_dependencies();
}

PACKAGE PACKAGE_LIST::findMaxVersion()
{
	string max_version="0";
	int id=0;
	for (int i=0;i<packages.size();i++)
	{
		if (packages[i].get_version()>=max_version)
		{
			max_version=packages[i].get_version();
			id=i;
		}
	}
	return packages[id];
}
void PACKAGE_LIST::destroy()
{
	packages.clear();
}
PACKAGE_LIST::PACKAGE_LIST(){}
PACKAGE_LIST::~PACKAGE_LIST()
{
}

