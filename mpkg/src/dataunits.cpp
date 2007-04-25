/*
	MOPSLinux packaging system
	Data types descriptions
	$Id: dataunits.cpp,v 1.30 2007/04/25 23:47:44 i27249 Exp $
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
	for (unsigned int i=0;i<str.length();i++)
	{
		if (str[i]=='\'') ret+="\'";
		ret+=str[i];
	}
	return ret;
}

bool SERVER_TAG::operator != (SERVER_TAG stag)
{
	if (server_tag_id!=stag.get_id()) return true;
	if (server_tag_name!=stag.get_name(false)) return true;
	return false;
}

bool SERVER_TAG::operator == (SERVER_TAG stag)
{
	if (server_tag_id!=stag.get_id()) return false;
	if (server_tag_name!=stag.get_name(false)) return false;
	return true;
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


bool SERVER_TAG_LIST::operator != (SERVER_TAG_LIST slist)
{
	if (size()!=slist.size()) return true;
	for (int i=0; i<size(); i++)
	{
		if (server_tags[i]!=*slist.get_server_tag(i)) return true;
	}
	return false;
}

bool SERVER_TAG_LIST::operator == (SERVER_TAG_LIST slist)
{
	if (size()!=slist.size()) return false;
	for (int i=0; i<size(); i++)
	{
		if (server_tags[i]!=*slist.get_server_tag(i)) return false;
	}
	return true;
}



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

bool SERVER::operator != (SERVER nserv)
{
	//if (server_id!=nserv.get_id()) return true;
	if (server_url!=nserv.get_url(false)) return true;
	if (server_priority!=nserv.get_priority(false)) return true;
	if (server_tags!=*nserv.get_tags()) return true;
	return false;
}

bool SERVER::operator == (SERVER nserv)
{
	//if (server_id!=nserv.get_id()) return false;
	if (server_url!=nserv.get_url(false)) return false;
	if (server_priority!=nserv.get_priority(false)) return false;
	if (server_tags!=*nserv.get_tags()) return false;
	return true;
}



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
	unsigned int i;
	i=0;
	debug("get_type()");

	if (server_url.find("file")!=std::string::npos) return SRV_FILE;
	if (server_url.find("cache://")!=std::string::npos) return SRV_CACHE;
	if (server_url.find("cdrom://")!=std::string::npos) return SRV_CDROM;
	if (server_url.find("http://")!=std::string::npos) return SRV_HTTP;
	if (server_url.find("ftp://")!=std::string::npos) return SRV_FTP;
	return SRV_BADURL;


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
	if (url[url.length()-1]!='/') url+="/";
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
	return 0;
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

bool SERVER_LIST::operator != (SERVER_LIST serv_list)
{
	if (size()!=serv_list.size()) return true;
	for (int i=0; i<size(); i++)
	{
		if (servers[i]!=*serv_list.get_server(i)) return true;
	}
	return false;
}

bool SERVER_LIST::operator == (SERVER_LIST serv_list)
{
	if (size()!=serv_list.size()) return false;
	for (int i=0; i<size(); i++)
	{
		if (servers[i]!=*serv_list.get_server(i)) return false;
	}
	return true;
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
	if (path[path.length()-1]!='/') path+="/";
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

bool LOCATION::operator != (LOCATION location)
{
//	if (location_id!=location.get_id()) return true;
	if (location_path!=location.get_path()) return true;
	if (server!=*location.get_server()) return true;
	return false;
}

bool LOCATION::operator == (LOCATION location)
{
/*	if (location_id!=location.get_id())
	{
		printf("dataunits.cpp: LOCATION::operator !=(): id mismatch\n");
		return false;
	}*/
	if (location_path!=location.get_path())
	{
		//printf("dataunits.cpp: LOCATION::operator !=(): path mismatch\n");
	       	return false;
	}
	if (server!=*location.get_server())
	{
		//printf("dataunits.cpp: LOCATION::operator !=(): server mismatch\n");
		return false;
	}
	return true;
}


bool LOCATION_LIST::operator != (LOCATION_LIST nloc)
{
#ifdef DEBUG
	printf("dataunits.cpp: LOCATION_LIST::operator !=(): size()=%d, nloc.size()=%d\n",size(), nloc.size());
#endif
	if (size()!=nloc.size())
	{
#ifdef DEBUG
		printf("returned true due to size mismatch");
#endif
		return true;
	}
	bool r;

	for (int i=0; i<size(); i++)
	{
#ifdef DEBUG
		printf("dataunits.cpp: LOCATION_LIST::operator !=(): cycle, i=%d\n", i);
#endif
		r=false;
		for (int k=0; k<size(); k++)
		{
			if (*nloc.get_location(i)==locations[k]) 
			{
				r=true;
				debug("dataunits.cpp: LOCATION_LIST::operator !=(): location match, r=true\n");
				break;
			}
		}
		if (!r) 
		{
#ifdef DEBUG
			printf("dataunits.cpp: LOCATION_LIST::operator !=(): returned true due to location missing or new\n");
#endif
			return true;
		}
	}
	return false;
}

bool LOCATION_LIST::operator == (LOCATION_LIST nloc)
{
	if (size()!=nloc.size()) return false;
	for (int i=0; i<size(); i++)
	{
		if (*nloc.get_location(i)!=locations[i]) return false;
	}
	return true;
}


vector<LOCATION> LOCATION_LIST::get_locations()
{
	return locations;
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

bool DEPENDENCY::operator != (DEPENDENCY ndep)
{
//	if (dependency_id!=ndep.get_id()) return true;
	if (dependency_condition!=ndep.get_condition(false)) return true;
	if (dependency_package_name!=ndep.get_package_name(false)) return true;
	if (dependency_package_version!=ndep.get_package_version(false)) return true;
	if (dependency_broken!=ndep.get_broken()) return true;
	return false;
}

bool DEPENDENCY::operator == (DEPENDENCY ndep)
{
//	if (dependency_id!=ndep.get_id()) return false;
	if (dependency_condition!=ndep.get_condition(false)) return false;
	if (dependency_package_name!=ndep.get_package_name(false)) return false;
	if (dependency_package_version!=ndep.get_package_version(false)) return false;
	if (dependency_broken!=ndep.get_broken()) return false;
	return true;
}


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
	if (dependency_condition==IntToStr(VER_ANY)) return "(any)";
	string tmp = "(unknown condition " + dependency_condition + ")";
	return tmp;
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

bool TAG::operator != (TAG ntag)
{
//	if (tag_id!=ntag.get_id()) return true;
	if (tag_name!=ntag.get_name(false)) return true;
	return false;
}

bool TAG::operator == (TAG ntag)
{
//	if (tag_id!=ntag.get_id()) return false;
	if (tag_name!=ntag.get_name(false)) return false;
	return true;
}



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

bool FILES::operator != (FILES nfile)
{
//	if (file_id!=nfile.get_id()) return true;
	if (file_name!=nfile.get_name(false)) return true;
	return false;
}

bool FILES::operator == (FILES nfile)
{
//	if (file_id!=nfile.get_id()) return false;
	if (file_name!=nfile.get_name(false)) return false;
	return true;
}


int FILES::get_id()
{
	return file_id;
}

string FILES::get_name(bool sql)
{
	if (sql) return PrepareSql(file_name);
	return file_name;
}

int FILES::get_type()
{
	return file_type;
}

bool FILES::config()
{
	if (file_type==FTYPE_CONFIG) return true;
	else return false;
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

int FILES::set_type(int type)
{
	file_type=type;
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
	file_type=FTYPE_PLAIN;
}
FILES::~FILES(){}

bool TAG_LIST::operator != (TAG_LIST ntags)
{
	if (size()!=ntags.size()) return true;
	for (int i=0; i<size(); i++)
		if (tags[i]!=*ntags.get_tag(i)) return true;
	return false;
}

bool TAG_LIST::operator == (TAG_LIST ntags)
{
	if (size()!=ntags.size()) return false;
	for (int i=0; i<size(); i++)
		if (tags[i]!=*ntags.get_tag(i)) return false;
	return true;
}


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

bool DEPENDENCY_LIST::operator != (DEPENDENCY_LIST ndep)
{
	if (size()!=ndep.size()) return true;
	for (int i=0; i<size();i++)
	{
		if (dependencies[i]!=*ndep.get_dependency(i)) return true;
	}
	return false;
}

bool DEPENDENCY_LIST::operator == (DEPENDENCY_LIST ndep)
{
	if (size()!=ndep.size()) return false;
	for (int i=0; i<size();i++)
	{
		if (dependencies[i]!=*ndep.get_dependency(i)) return false;
	}
	return true;
}

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


bool FILE_LIST::operator != (FILE_LIST nfiles)
{
	if (size()!=nfiles.size()) return true;
	for (int i=0; i<size(); i++)
	{
		if (files[i]!=*nfiles.get_file(i)) return true;
	}
	return false;
}

bool FILE_LIST::operator == (FILE_LIST nfiles)
{
	if (size()!=nfiles.size()) return false;
	for (int i=0; i<size(); i++)
	{
		if (files[i]!=*nfiles.get_file(i)) return false;
	}
	return true;
}


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
	for (unsigned int i=0;i<filelist.files.size();i++)
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

bool SCRIPTS::operator != (SCRIPTS scr)
{
//	if (script_id!=scr.get_id()) return true;
	if (preinstall!=scr.get_preinstall(false)) return true;
	if (postinstall!=scr.get_postinstall(false)) return true;
	if (preremove!=scr.get_preremove(false)) return true;
	if (postremove!=scr.get_postremove(false)) return true;
	return false;
}

bool SCRIPTS::operator == (SCRIPTS scr)
{
//	if (script_id!=scr.get_id()) return false;
	if (preinstall!=scr.get_preinstall(false)) return false;
	if (postinstall!=scr.get_postinstall(false)) return false;
	if (preremove!=scr.get_preremove(false)) return false;
	if (postremove!=scr.get_postremove(false)) return false;
	return true;
}

void SCRIPTS::clear()
{
	script_id=0;
	preinstall.clear();
	postinstall.clear();
	preremove.clear();
	postremove.clear();
}

bool SCRIPTS::IsEmpty()
{
	if (script_id==0 \
			&& preinstall.empty() \
			&& postinstall.empty() \
			&& preremove.empty() \
			&& postremove.empty())
		return true;
	else return false;
}

int SCRIPTS::get_id()
{
	return script_id;
}

string SCRIPTS::get_vid()
{
	return IntToStr(script_id);
}

string SCRIPTS::get_preinstall(bool sql)
{
	if (sql) return PrepareSql(preinstall);
	return preinstall;
}

string SCRIPTS::get_postinstall(bool sql)
{
	if (sql) return PrepareSql(postinstall);
	return postinstall;
}

string SCRIPTS::get_preremove(bool sql)
{
	if (sql) return PrepareSql(preremove);
	return preremove;
}

string SCRIPTS::get_postremove(bool sql)
{
	if (sql) return PrepareSql(postremove);
	return postremove;
}

void SCRIPTS::set_preinstall(string preinst)
{
	preinstall=preinst;
}

void SCRIPTS::set_postinstall(string postinst)
{
	postinstall=postinst;
}

void SCRIPTS::set_preremove(string prerem)
{
	preremove=prerem;
}

void SCRIPTS::set_postremove(string postrem)
{
	postremove=postrem;
}

void SCRIPTS::set_id(int id)
{
	script_id=id;
}

void SCRIPTS::set_vid(string id)
{
	script_id=atoi(id.c_str());
}

SCRIPTS::SCRIPTS()
{
	script_id=0;
	preinstall = "#!/bin/sh\n";
	postinstall ="#!/bin/sh\n";
	preremove =  "#!/bin/sh\n";
	postremove = "#!/bin/sh\n";

}
SCRIPTS::~SCRIPTS()
{
}

bool PACKAGE::operator != (PACKAGE npkg)
{
//	if (package_id!=npkg.get_id()) return true;
	if (package_name!=npkg.get_name(false)) return true;
	if (package_version!=npkg.get_version(false)) return true;
	if (package_arch!=npkg.get_arch(false)) return true;
	if (package_build!=npkg.get_build(false)) return true;
	if (package_compressed_size!=npkg.get_compressed_size(false)) return true;
	if (package_installed_size!=npkg.get_installed_size(false)) return true;
	if (package_short_description!=npkg.get_short_description(false)) return true;
	if (package_description!=npkg.get_description(false)) return true;
	if (package_changelog!=npkg.get_changelog(false)) return true;
	if (package_packager!=npkg.get_packager(false)) return true;
	if (package_packager_email!=npkg.get_packager_email(false)) return true;
	if (package_available!=npkg.available()) return true;
	if (package_installed!=npkg.installed()) return true;
	if (package_configexist!=npkg.configexist()) return true;
	if (package_action!=npkg.action()) return true;
	if (package_md5!=npkg.get_md5(false)) return true;
	if (package_filename!=npkg.get_filename(false)) return true;
	if (package_dependencies!=*npkg.get_dependencies()) return true;
	if (package_locations!=*npkg.get_locations()) return true;
	if (package_tags!=*npkg.get_tags()) return true;
	if (package_scripts!=*npkg.get_scripts()) return true;
	if (package_files!=*npkg.get_files()) return true;
	return false;
}	

bool PACKAGE::operator == (PACKAGE npkg)
{
//	if (package_id!=npkg.get_id()) return false;
	if (package_name!=npkg.get_name(false)) return false;
	if (package_version!=npkg.get_version(false)) return false;
	if (package_arch!=npkg.get_arch(false)) return false;
	if (package_build!=npkg.get_build(false)) return false;
	if (package_compressed_size!=npkg.get_compressed_size(false)) return false;
	if (package_installed_size!=npkg.get_installed_size(false)) return false;
	if (package_short_description!=npkg.get_short_description(false)) return false;
	if (package_description!=npkg.get_description(false)) return false;
	if (package_changelog!=npkg.get_changelog(false)) return false;
	if (package_packager!=npkg.get_packager(false)) return false;
	if (package_packager_email!=npkg.get_packager_email(false)) return false;
	if (package_available!=npkg.available()) return false;
	if (package_installed!=npkg.installed()) return false;
	if (package_configexist!=npkg.configexist()) return false;
	if (package_action!=npkg.action()) return false;

	if (package_md5!=npkg.get_md5(false)) return false;
	if (package_filename!=npkg.get_filename(false)) return false;
	if (package_dependencies!=*npkg.get_dependencies()) return false;
	if (package_locations!=*npkg.get_locations()) return false;
	if (package_tags!=*npkg.get_tags()) return false;
	if (package_scripts!=*npkg.get_scripts()) return false;
	if (package_files!=*npkg.get_files()) return false;
	return true;
}

bool PACKAGE::isUpdate()
{
	if (!installed() && hasMaxVersion && !installedVersion.empty() && installedVersion != maxVersion)
	{
		return true;
	}
	else return false;
}
string PACKAGE::get_fullversion()
{
	string ret = get_version() + " build " + get_build();
	return ret;
}

void PACKAGE::clear()
{
	package_id=0;
	package_name.clear();
	package_version.clear();
	package_arch.clear();
	package_build.clear();
	package_compressed_size.clear();
	package_installed_size.clear();
	package_short_description.clear();
	package_description.clear();
	package_changelog.clear();
	package_packager.clear();
	package_packager_email.clear();
	package_available=0;
	package_installed=0;
	package_configexist=0;
	package_action=0;
	package_md5.clear();
	package_filename.clear();
	package_files.clear();
	package_locations.clear();
	package_dependencies.clear();
	package_tags.clear();
	package_scripts.clear();
}

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

int PACKAGE::add_file(string file_name)
{
	FILES file;
	file.set_name(file_name);
	//file.set_size(file_size);
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

bool PACKAGE::available()
{
	return package_available;
}
bool PACKAGE::reachable()
{
	if (package_available || package_installed) return true;
}
bool PACKAGE::installed()
{
	return package_installed;
}

bool PACKAGE::configexist()
{
	return package_configexist;
}

int PACKAGE::action()
{
	return package_action;
}
string PACKAGE::get_vstatus(bool color)
{
	char *CL_WHITE=	"\033[22;39m";
	char *CL_RED =	"\033[22;31m";
	char *CL_GREEN ="\033[22;32m";
	char *CL_YELLOW ="\033[22;33m";
	char *CL_BLUE =	"\033[22;34m";

	string stat;
	if (available()) stat +="A";
	else stat+="_";

	if (installed()) stat += "I";
	else {
		stat += "_";
		if (configexist()) stat += "C";
		else stat+="_";
	}
	switch(action())
	{
		case ST_INSTALL:
			stat+="i";
			break;
		case ST_REMOVE:
			stat+="r";
			break;
		case ST_PURGE:
			stat+="p";
			break;
		case ST_NONE:
			stat+="_";
			break;
		default:
			stat+="?";
			break;
	}
	return stat;
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

int PACKAGE::get_err_type()
{
	return package_err_type;
}

int PACKAGE::set_err_type(int err)
{
	package_err_type=err;
	return 0;
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


void PACKAGE::set_available(bool flag)
{
	package_available = flag;
}

void PACKAGE::set_installed(bool flag)
{
	package_installed = flag;
}

void PACKAGE::set_configexist(bool flag)
{
	package_configexist = flag;
}

void PACKAGE::set_action(int new_action)
{
	package_action = new_action;
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

DESCRIPTION_LIST* PACKAGE::get_descriptions()
{
	return &package_descriptions;
}

int PACKAGE::set_descriptions(DESCRIPTION_LIST desclist)
{
	package_descriptions=desclist;
	return 0;
}

FILE_LIST* PACKAGE::get_files()
{
	return &package_files;
}

FILE_LIST* PACKAGE::get_config_files()
{
	return &config_files;
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

SCRIPTS* PACKAGE::get_scripts()
{
	return &package_scripts;
}

int PACKAGE::set_files(FILE_LIST files)
{
	package_files=files;
	return 0;
}

int PACKAGE::set_config_files(FILE_LIST conf_files)
{
	config_files=conf_files;
	return 0;
}

void PACKAGE::sync()
{
	//printf("sync start: %d config files, %d package files\n", config_files.size(), package_files.size());
	for (int i=0; i< config_files.size(); i++)
	{
		//printf("Searching file %s\n", config_files.get_file(i)->get_name().c_str());
		for (int t=0; t<package_files.size(); t++)
		{
			if (config_files.get_file(i)->get_name()=='/' + package_files.get_file(t)->get_name())
			{
				//printf("config file %s\n", config_files.get_file(i)->get_name().c_str());
				package_files.get_file(t)->set_type(FTYPE_CONFIG);
				break;
			}
		}
	}

	if (config_files.IsEmpty())
	{
		for (int i=0; i < package_files.size(); i++)
		{
			if (package_files.get_file(i)->get_type()==FTYPE_CONFIG)
			{
				config_files.add(*package_files.get_file(i));
			}
		}
	}
	//printf("sync end\n");
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

int PACKAGE::set_scripts(SCRIPTS scripts)
{
	package_scripts=scripts;
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
	package_available=0;
	package_installed=0;
	package_configexist=0;
	package_action=0;

	package_err_type=DEP_OK;
}
PACKAGE::~PACKAGE()
{
}
void PACKAGE::clearVersioning()
{
	hasMaxVersion=false;
	maxVersion.clear();
	installedVersion.clear();
	alternateVersions.clear();
}

void PACKAGE_LIST::clearVersioning()
{
	for (int i=0; i<packages.size(); i++)
	{
		packages[i].clearVersioning();
	}
}

void PACKAGE::destroy()
{
}
// Helpful function ))
string IntToStr(int num)
{
  	char *s = (char *) malloc(2000);
  	string ss;
  	if (s)
  	{
		sprintf(s,"%d",num);
	  	ss=s;
	  	free(s);
  	}
  	else 
  	{
		printf("Error: malloc() failed!!!\n");
	  	abort();
  	}
  	return ss;
}


bool PACKAGE_LIST::operator != (PACKAGE_LIST nlist)
{
	if (size()!=nlist.size()) return true;
	for (int i=0; i<size(); i++)
	{
		if (packages[i]!=*nlist.get_package(i)) return true;
	}
	return false;
}

bool PACKAGE_LIST::operator == (PACKAGE_LIST nlist)
{
	if (size()!=nlist.size()) return false;
	for (int i=0; i<size(); i++)
	{
		if (packages[i]!=*nlist.get_package(i)) return false;
	}
	return true;
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


int PACKAGE_LIST::add_list(PACKAGE_LIST *pkgList, bool skip_identical)
{
//	printf("dataunits.cpp:add_list() started\n");
//	printf("add_list: adding %d packages\n", pkgList->size());
	// IMPORTANT NOTE!
	// If skip_identical is true, the locations will be MERGED together!
	int ret;
//	int old_size=packages.size();
	ret=packages.size()+pkgList->size();
	//packages.resize(ret);
	bool identical_found=false;
	//int identical_id;
	bool location_found;
	for (int i=0; i<pkgList->size();i++)
	{
#ifdef DEBUG
		printf("add_list: adding package %d\n", i);
		if (skip_identical) printf("add_list: skip_identical=true)\n");
#endif
		if (skip_identical)
		{
#ifdef DEBUG
			printf("add_list: Skipping identical packages, just adding locations for them\n");
#endif
			identical_found=false;
			// Checking if lists have identical items, remove it
			for (unsigned int s=0; s<packages.size(); s++)
			{
#ifdef DEBUG
				printf("add_list: comparing with package %d\n", s);
#endif
#ifndef NO_MD5_COMPARE
				if (packages[s].get_md5()==pkgList->get_package(i)->get_md5())
#endif
#ifdef NO_MD5_COMPARE
				if (packages[s].get_name()==pkgList->get_package(i)->get_name() \
					&& packages[s].get_version()==pkgList->get_package(i)->get_version() \
					&& packages[s].get_arch()==pkgList->get_package(i)->get_arch() \
					&& packages[s].get_build()==pkgList->get_package(i)->get_build())
#endif
				{
					identical_found=true;
#ifdef DEBUG
					printf("add_list: Found identical package %s with %d locations, merging locations\n", \
							pkgList->get_package(i)->get_name().c_str(), \
							pkgList->get_package(i)->get_locations()->size());
#endif
					// Comparing locations and merging
					for (int l=0; l<pkgList->get_package(i)->get_locations()->size(); l++)
					{
#ifdef DEBUG
						printf("add_list: Testing location %d\n", l);
#endif
						location_found=false;
						for (int ls=0; ls<packages[s].get_locations()->size(); ls++)
						{
							if (*packages[s].get_locations()->get_location(ls)==*pkgList->get_package(i)->get_locations()->get_location(l))
							{
#ifdef DEBUG
								printf("Location already exists\n");
#endif
								location_found=true;
								break;
							}
						}
#ifdef DEBUG
						if (location_found) printf("add_list: location_found=true, so -> location is not new\n");
						else printf("add_list: location_found=false, so location is NEW\n");
#endif
						if (!location_found)
						{
#ifdef DEBUG
							printf("New location, adding\n");
#endif
							packages[s].get_locations()->add(*pkgList->get_package(i)->get_locations()->get_location(l));
						}
					}
					break;
				}
#ifdef DEBUG
				else	// end: if packages seems identical
				{
					printf("add_list: package is NEW, adding fully");
				}
#endif

			}
		}
#ifdef DEBUG
		else
		{
			printf("add_list: simply adding packages\n");
		}
#endif
		if (!identical_found)
		{
#ifdef DEBUG
			printf("add_list: (at end of cycle) identical_found=false, so -> New package with %d locations, adding\n", \
					pkgList->get_package(i)->get_locations()->size());
#endif
			this->add(*pkgList->get_package(i));
		}
	}
//	printf("dataunits.cpp:add_list() end\n");
	return 0;
}


void PACKAGE_LIST::clear(unsigned int new_size)
{
	packages.clear();
    	packages.resize(new_size);
}

void PACKAGE_LIST::set_size(unsigned int new_size)
{
	packages.resize(new_size);
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
	string tmp_ver;
	for (unsigned int i=0;i<packages.size();i++)
	{
		tmp_ver = packages[i].get_fullversion();// + packages[i].get_build();
		if (strverscmp(tmp_ver.c_str(), max_version.c_str())>=0)
		{
			max_version=tmp_ver;
			id=i;
		}
	}
	return packages[id];
}
void PACKAGE_LIST::destroy()
{
	packages.clear();
}
/*int PACKAGE_LIST::getCloneID(int testID)
{
	return cList.getCloneID(testID);
}*/
PACKAGE_LIST::PACKAGE_LIST(){}
PACKAGE_LIST::~PACKAGE_LIST()
{
}


int DESCRIPTION::set_id(int id)
{
	description_id=id;
	return 0;
}

int DESCRIPTION::set_language(string language)
{
	description_language=language;
	return 0;
}

int DESCRIPTION::set_text(string text)
{
	description_text=text;
	return 0;
}

int DESCRIPTION::set_shorttext(string shorttext)
{
	short_description_text=shorttext;
	return 0;
}


int DESCRIPTION::get_id()
{
	return description_id;
}

string DESCRIPTION::get_language()
{
	return description_language;
}

string DESCRIPTION::get_text( bool sql)
{
	if (sql) return PrepareSql(description_text);
	else return description_text;
}

string DESCRIPTION::get_shorttext( bool sql)
{
	if (sql) return PrepareSql(short_description_text);
	else return short_description_text;
}

void DESCRIPTION::clear()
{
	description_id=0;
	description_language.clear();
	description_text.clear();
}

DESCRIPTION::DESCRIPTION(){}
DESCRIPTION::~DESCRIPTION()
{
	description_id=0;
}

DESCRIPTION * DESCRIPTION_LIST::get_description(unsigned int num)
{
	if (num>=0 && num <descriptions.size())
	{
		return &descriptions[num];
	}
	else
	{
		printf("DESCRIPTION_LIST: Error in range!\n");
		abort();
	}
}

int DESCRIPTION_LIST::set_description(unsigned int num, DESCRIPTION description)
{
	if (num>=0 && num < descriptions.size())
	{
		descriptions[num]=description;
		return 0;
	}
	else return -1;
}

int DESCRIPTION_LIST::add(DESCRIPTION description)
{
	descriptions.push_back(description);
	return 0;
}

unsigned int DESCRIPTION_LIST::size()
{
	return descriptions.size();
}

bool DESCRIPTION_LIST::empty()
{
	return descriptions.empty();
}

void DESCRIPTION_LIST::clear()
{
	descriptions.clear();
}

DESCRIPTION_LIST::DESCRIPTION_LIST(){}
DESCRIPTION_LIST::~DESCRIPTION_LIST(){}

cloneList::cloneList(){
initialized = false;
}

void PACKAGE_LIST::initVersioning()
{
	// Что надо определить:
	// Для каждого пакета - список номеров того же пакета других версий (НЕ ВКЛЮЧАЯ этот же пакет)
	// Максимально доступную версию
	// Версию установленного пакета
	// Флаг максимальности версии (если таковых пакетов несколько, ставится у одного любого)
	//
	// Делаем пока не оптимально но надежно
	// Шаг первый. Список альтернативных версий
	for (int i=0; i<packages.size(); i++)
	{
		packages[i].clearVersioning();
		for (int j=0; j<packages.size(); j++)
		{
			// Если это не тот же пакет и имена совпадают - добавляем номер в список
			if (i!=j && packages[i].get_name() == packages[j].get_name())
			{
				packages[i].alternateVersions.push_back(j);
			}
		}
	}

	// Шаг второй. Для каждого пакета ищем максимальную версию
	string max_version; // Переменная содержащая максимальную версию
	int max_version_id; // номер пакета содержавшего максимальную версию
	string this_version;
	string installed_version;
	for (int i=0; i<packages.size();i++)
	{
		max_version.clear();
		max_version_id=-1;
		this_version.clear();
		installed_version.clear();
		if (packages[i].installed())
		{
			installed_version = packages[i].get_fullversion();// + \
					    packages[i].get_build();
		}

		// Если у пакета нет других версий - значит максимальную версию имеет он
		
		if (packages[i].alternateVersions.empty())
		{
			max_version = packages[i].get_fullversion();// + \
				      packages[i].get_build();
			packages[i].hasMaxVersion=true;
		}
		else
		{
			for (int j=0; j<packages[i].alternateVersions.size(); j++)
			{
				this_version = packages[packages[i].alternateVersions[j]].get_fullversion();// + \
					       packages[packages[i].alternateVersions[j]].get_build();
				if (packages[packages[i].alternateVersions[j]].installed())
				{
					installed_version = packages[packages[i].alternateVersions[j]].get_fullversion();// + \
							    packages[packages[i].alternateVersions[j]].get_build();
				}
				if (strverscmp(this_version.c_str(), max_version.c_str())>0)
				{
					max_version = this_version;
					max_version_id = packages[i].alternateVersions[j];
				}
			}
			if (max_version.empty()) // Если максимальной версии так и не нашлось (все пакеты - одинаковой версии) - то ставим максимум текущему
			{
				max_version = packages[i].get_version();
				max_version_id = i;
			}
			else
			{
				// Проверим - а вдруг именно этот пакет имеет максимальную версию?
				this_version = packages[i].get_fullversion();// + packages[i].get_build();
				if (strverscmp(this_version.c_str(), max_version.c_str())>0)
				{
					max_version = this_version;
					max_version_id = i;
				}
				// Устанавливаем найденному пакету нужные флаги
			}
		}
		// Запишем установленную версию
		packages[max_version_id].hasMaxVersion=true;
		packages[i].maxVersion=max_version;
		packages[i].installedVersion = installed_version;
	}
}


/*void cloneList::init(vector<PACKAGE> &pkgList)
{
	
	printf("Initializing clone list\n");
	whoHasClones_IDs.clear(); 	// Список объектов, имеющих клоны
	objectCloneListID.clear(); 	// Список клонов каждого объекта
	masterCloneID.clear();		// ID клона, имеющего максимальную версию.
	installedCloneID.clear();	// ID установленного клона. Если такого нету, ставится равным -1
	for (int i=0; i<pkgList.size(); i++)
	{
		pkgList[i].hasClone=false;
		pkgList[i].masterCloneID=i;
		pkgList[i].isMasterClone=false;
		pkgList[i].isMaxVersion=false;
		pkgList[i].hasUpdates=false;
		if (pkgList[i].installed()) pkgList[i].installedVersion=pkgList[i].get_version();
	}

	// Шаг 1. Ищем объекты, имеющие клонов. Заполняем списки.
	vector<int>thisClonesList;
	int thisInstalledID;
	int thisClonePosition;
	string maxCloneVersion;
	int lastMasterCloneID;
	bool itHasClone;
	string tmp_ver;
	string installedVersion;
	progressEnabled = true;
	progressMax = pkgList.size();
	for (int i=0; i<pkgList.size(); i++)
	{
		currentProgress = i;
		itHasClone = false;
		thisInstalledID = -1;
		thisClonePosition = -1;
		lastMasterCloneID = -1;
		thisClonesList.clear();
		installedVersion.clear();
		maxCloneVersion.clear();
		if (pkgList[i].installed()) installedVersion = pkgList[i].get_version()+pkgList[i].get_build();
		for (int k=0; k<pkgList.size(); k++)
		{
			if (k != i && pkgList[k].get_name() == pkgList[i].get_name())
			{
				itHasClone = true;
				if (pkgList[k].installed()) installedVersion = pkgList[k].get_version()+pkgList[i].get_build();

				if (thisClonesList.empty())
				{
					thisClonesList.push_back(i);
					whoHasClones_IDs.push_back(i);
					thisClonePosition = whoHasClones_IDs.size()-1;
				}
				thisClonesList.push_back(k);
				if (pkgList[k].installed()) thisInstalledID = k;
				tmp_ver = pkgList[k].get_version()+pkgList[k].get_build();
				if (maxCloneVersion.empty() || strverscmp(tmp_ver.c_str(), maxCloneVersion.c_str())>0)
				{
					maxCloneVersion = tmp_ver;
					lastMasterCloneID = k;
				}
			}
		}
		if (itHasClone)
		{
			if (maxCloneVersion!=installedVersion && !installedVersion.empty())
			{
				printf("Installed version: %s, Max version: %s\n", installedVersion.c_str(), maxCloneVersion.c_str());
				pkgList[i].hasUpdates=true;
			}
			pkgList[i].hasClone=true;
			pkgList[i].masterCloneID=lastMasterCloneID;
			installedCloneID.push_back(thisInstalledID);
			objectCloneListID.push_back(thisClonesList);
			masterCloneID.push_back(lastMasterCloneID);
		}
	}
	printf("MasterClones list:\n");
	for (int i=0; i<masterCloneID.size(); i++)
	{
		if (pkgList[masterCloneID[i]].hasUpdates) pkgList[masterCloneID[i]].isMaxVersion=true;
		pkgList[masterCloneID[i]].isMasterClone=true;
	}
	printf("(end)\n");
#ifdef DEBUG
	printf("whoHasClones_IDs: %d\nObjectCloneListID: %d\nmasterCloneID: %d\ninstalledCloneID: %d\n",\
		       	whoHasClones_IDs.size(),\
			objectCloneListID.size(),\
			masterCloneID.size(),
			installedCloneID.size());
	printf("Clone tree list:\n");
	for (unsigned int i=0; i<objectCloneListID.size(); i++)
	{
		printf("%s: ", pkgList[whoHasClones_IDs[i]].get_name().c_str());
		for (unsigned int t = 0; t<objectCloneListID[i].size(); t++)
		{
			printf("%s, ", pkgList[objectCloneListID[i][t]].get_name().c_str());
		}
		printf("\n");
	}
#endif
	//sleep(10000);
	initialized = true;
	

}
cloneList::~cloneList(){}

int cloneList::getCloneID(int testID)
{
	for (unsigned int i=0; i<whoHasClones_IDs.size(); i++)
	{
		if (whoHasClones_IDs[i]==testID)
		{
			return i;
		}
	}
	return -1;
}
*/
/*dTreeItem::dTreeItem(){}
dTreeItem::~dTreeItem(){}
void dTreeItem::addChild(PACKAGE *pkg)
{
	childs.push_back(pkg->get_id());
}

PACKAGE *getChild(int package_id)
{
	for (int i=0; i<childs.size(); i++)
	{
		//if (childs[i]==package_id) return 
	}
}
*/
