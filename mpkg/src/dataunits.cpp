/*
	MOPSLinux packaging system
	Data types descriptions
	$Id: dataunits.cpp,v 1.47 2007/05/15 07:08:46 i27249 Exp $
*/



#include "dataunits.h"
#include "dependencies.h"
#include "conditions.h"
#include "debug.h"
#include "constants.h"

string PrepareSql(string str)
{
	// Need optimizations, if possible
	mstring ret;
	if (str.empty()) return "0";
	for (unsigned int i=0;i<str.length();i++)
	{
		if (str[i]=='\'') ret+="\'";
		ret+=str[i];
	}
	return ret.s_str();
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
	if (server_url.find("file://")!=std::string::npos) return SRV_FILE;
	if (server_url.find("local://")!=std::string::npos) return SRV_LOCAL;
	if (server_url.find("cdrom://")!=std::string::npos) return SRV_CDROM;
	return SRV_NETWORK;
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
	if (location_path!=location.get_path())
	{
	       	return false;
	}
	if (server!=*location.get_server())
	{
		return false;
	}
	return true;
}


bool LOCATION_LIST::operator != (LOCATION_LIST nloc)
{
	if (size()!=nloc.size())
	{
		return true;
	}
	bool r;

	for (int i=0; i<size(); i++)
	{
		r=false;
		for (int k=0; k<size(); k++)
		{
			if (*nloc.get_location(i)==locations[k]) 
			{
				r=true;
				break;
			}
		}
		if (!r) 
		{
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
	if (version_data.condition!=ndep.get_condition(false)) return true;
	if (dependency_package_name!=ndep.get_package_name(false)) return true;
	if (version_data.version!=ndep.get_package_version(false)) return true;
	if (dependency_broken!=ndep.get_broken()) return true;
	return false;
}

bool DEPENDENCY::operator == (DEPENDENCY ndep)
{
//	if (dependency_id!=ndep.get_id()) return false;
	if (version_data.condition!=ndep.get_condition(false)) return false;
	if (dependency_package_name!=ndep.get_package_name(false)) return false;
	if (version_data.version!=ndep.get_package_version(false)) return false;
	if (dependency_broken!=ndep.get_broken()) return false;
	return true;
}

string DEPENDENCY::getDepInfo()
{
	string ret;
	ret = dependency_package_name + " " + get_vcondition() + " " +get_package_version();
	return ret;
};


versionData DEPENDENCY::get_version_data()
{
	return version_data;
}

void DEPENDENCY::clear()
{
	dependency_broken=0;
	version_data.version.clear();
	dependency_package_name.clear();
	dependency_type.clear();
	version_data.condition.clear();
	dependency_id=0;
}

int DEPENDENCY::get_id()
{
	return dependency_id;
}

string DEPENDENCY::get_condition(bool sql)
{
	if (sql) return PrepareSql(version_data.condition);
	return version_data.condition;
}

string DEPENDENCY::get_vcondition()
{
	if (version_data.condition==IntToStr(VER_MORE)) return ">";
	if (version_data.condition==IntToStr(VER_LESS)) return "<";
	if (version_data.condition==IntToStr(VER_EQUAL)) return "==";
	if (version_data.condition==IntToStr(VER_NOTEQUAL)) return "!=";
	if (version_data.condition==IntToStr(VER_XMORE)) return ">=";
	if (version_data.condition==IntToStr(VER_XLESS)) return "<=";
	if (version_data.condition==IntToStr(VER_ANY)) return "(any)";
	string tmp = "(unknown condition " + version_data.condition + ")";
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
	if (sql) return PrepareSql(version_data.version);
	return version_data.version;
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
	version_data.condition=condition;
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
	version_data.version=package_version;
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

DEPENDENCY* DEPENDENCY_LIST::at(int num)
{
	return get_dependency(num);
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

void LOCATION_LIST::sortLocations()
{
	// Sorting order:
	// 0: local://
	// 1: file://
	// 2: <all network>
	// 3: cdrom://
	vector<LOCATION> sorted;
	for (int t=0; t<=3; t++)
	{
		for (int i=0; i<locations.size(); i++)
		{
			if (locations.at(i).get_server()->get_type()==t)
			{
				sorted.push_back(locations.at(i));
			}
		}
	}
	locations=sorted;
}
				

void PACKAGE::sortLocations()
{
	package_locations.sortLocations();
}

bool PACKAGE::operator != (PACKAGE npkg)
{
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
bool PACKAGE::isItRequired(PACKAGE *testPackage)
{
	for (int i=0; i<package_dependencies.size(); i++)
	{
		if (package_dependencies.at(i)->get_package_name() == testPackage->get_name() && \
				meetVersion(package_dependencies.at(i)->get_version_data(), testPackage->get_version()))
			return true;
	}
	return false;
}
void PACKAGE::set_broken(bool flag)
{
	isBroken=flag;
}
void PACKAGE::set_requiredVersion(versionData reqVersion)
{
	requiredVersion = reqVersion;
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
	package_id=-1;
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

bool PACKAGE::available(bool includeLocal)
{
	if (package_locations.IsEmpty()) return false;
	else
	{
		if (includeLocal) return true;
		for (int i=0; i<package_locations.size(); i++)
		{
			if (package_locations.get_location(i)->get_server()->get_type()!=SRV_LOCAL)
			{
				return true;
			}
		}
	}
	return false;
}
bool PACKAGE::reachable(bool includeConfigFiles)
{
	if (includeConfigFiles)
	{
		if (available() || installed() || configexist()) return true;
		else return false;
	}
	else
	{
		if (available() || installed()) return true;
		else return false;
	}
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
	fprintf(stderr, "set_available is deprecated. Please do not use this\n");
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
	for (int i=0; i< config_files.size(); i++)
	{
		for (int t=0; t<package_files.size(); t++)
		{
			if (config_files.get_file(i)->get_name()=='/' + package_files.get_file(t)->get_name())
			{
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
	isBroken = false;
	isRequirement = false;
	package_id=-1;
	package_available=false;
	package_installed=false;
	package_configexist=false;
	package_action=ST_NONE;
	newPackage = false;

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

void PACKAGE_LIST::sortByPriority(bool reverse_order)
{
	
	if (!priorityInitialized) buildDependencyOrder();
	int min_priority = 0;
	for (int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).priority>min_priority) min_priority = packages.at(i).priority;
	}

	vector<PACKAGE> sorted;
	if (!reverse_order)
	{	
		for (int p=0; p<=min_priority; p++)
		{
			for (int i=0; i<packages.size(); i++)
			{
				if (packages.at(i).priority==p) sorted.push_back(packages.at(i));
			}
		}
	}
	else
	{
		for (int p=min_priority; p>=0; p--)
		{
			for (int i=0; i<packages.size(); i++)
			{
				if (packages.at(i).priority==p) sorted.push_back(packages.at(i));
			}
		}
	}


	packages = sorted;
}




double PACKAGE_LIST::totalCompressedSize()
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		ret+=strtod(packages.at(i).get_compressed_size().c_str(), NULL);
	}
	return ret;
}


double PACKAGE_LIST::totalInstalledSize()
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		ret+=strtod(packages.at(i).get_installed_size().c_str(), NULL);
	}
	return ret;
}

double PACKAGE_LIST::totalCompressedSizeByAction(int select_action)
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).action()==select_action) ret+=strtod(packages.at(i).get_compressed_size().c_str(), NULL);
	}
	return ret;
}
double PACKAGE_LIST::totalInstalledSizeByAction(int select_action)
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).action()==select_action)
		{
			ret+=strtod(packages.at(i).get_installed_size().c_str(), NULL);
		}
	}
	return ret;
}


int PACKAGE_LIST::getPackageNumberByMD5(string md5)
{
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).get_md5()==md5) return i;
	}
	return -1;
}
void PACKAGE_LIST::clearVersioning()
{
	for (int i=0; i<packages.size(); i++)
	{
		packages.at(i).clearVersioning();
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
		perror("Error while allocating memory");
	  	abort();
  	}
  	return ss;
}


bool PACKAGE_LIST::operator != (PACKAGE_LIST nlist)
{
	if (size()!=nlist.size()) return true;
	for (int i=0; i<size(); i++)
	{
		if (packages.at(i)!=*nlist.get_package(i)) return true;
	}
	return false;
}

bool PACKAGE_LIST::operator == (PACKAGE_LIST nlist)
{
	if (size()!=nlist.size()) return false;
	for (int i=0; i<size(); i++)
	{
		if (packages.at(i)!=*nlist.get_package(i)) return false;
	}
	return true;
}

PACKAGE* PACKAGE_LIST::operator [] (int num)
{
	return this->get_package(num);
}
	
bool PACKAGE_LIST::hasInstalledOnes()
{
	if (packages.size()==0) return false;
	for (int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).installed())
		{
			return true;
		}
	}
	return false;
}

PACKAGE PACKAGE_LIST::getInstalledOne()
{
	for (int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).installed()) return packages.at(i);
	}
	fprintf(stderr, "getInstalledOne: no installed ones\n");
	PACKAGE p;
	return p;
}

PACKAGE PACKAGE_LIST::getMaxVersion()
{
	if (packages.size()>0)	return packages.at(getMaxVersionNumber(packages.at(0).get_name()));
	else
	{
		printf("Error in getMaxVersion: list empty\n");
		PACKAGE p;
		return p;
	}
}


PACKAGE* PACKAGE_LIST::get_package(int num)
{
	//PACKAGE s_package;
	if (num>=0 && num<size())
	{
		return &packages.at(num);
	}
	else return NULL;
}

int PACKAGE_LIST::set_package(int num, PACKAGE package)
{
	if (num>=0 && num<size())
	{
		packages.at(num)=package;
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
int PACKAGE_LIST::add(PACKAGE *package)
{
	return add(*package);
}

int PACKAGE_LIST::add(PACKAGE_LIST *pkgList)
{
	return add_list(pkgList, false);
}
int PACKAGE_LIST::add(PACKAGE_LIST pkgList)
{
	return add(&pkgList);
}
PACKAGE_LIST PACKAGE_LIST::operator + (PACKAGE_LIST pkgList)
{
	PACKAGE_LIST tmp=*this;
	tmp.add(pkgList);
	return tmp;
}
bool PACKAGE_LIST::operator += (PACKAGE_LIST pkgList)
{
	add(pkgList);
}
int PACKAGE_LIST::add_list(PACKAGE_LIST *pkgList, bool skip_identical)
{
	// IMPORTANT NOTE!
	// If skip_identical is true, the locations will be MERGED together!
	int ret;
	ret=packages.size()+pkgList->size();
	bool identical_found=false;
	bool location_found=false;
	for (int i=0; i<pkgList->size();i++)
	{
		if (skip_identical)
		{
			identical_found=false;
			// Checking if lists have identical items, remove it
			for (unsigned int s=0; s<packages.size(); s++)
			{
				if (packages[s].get_md5()==pkgList->get_package(i)->get_md5())
				{
					identical_found=true;
					// Comparing locations and merging
					for (int l=0; l<pkgList->get_package(i)->get_locations()->size(); l++)
					{
						location_found=false;
						for (int ls=0; ls<packages[s].get_locations()->size(); ls++)
						{
							if (*packages[s].get_locations()->get_location(ls)==*pkgList->get_package(i)->get_locations()->get_location(l))
							{
								location_found=true;
								break;
							}
						}
						if (!location_found)
						{
							packages[s].get_locations()->add(*pkgList->get_package(i)->get_locations()->get_location(l));
						}
					}
					break;
				}

			}
		}
		if (!identical_found)
		{
			this->add(*pkgList->get_package(i));
		}
	}
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
	string max_version="";
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

int PACKAGE_LIST::getMaxVersionID(string package_name)
{
	if (!versioningInitialized) initVersioning();
	for (int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).get_name() == package_name && packages.at(i).hasMaxVersion)
		{
			return packages.at(i).get_id();//i;
		}
	}
	return MPKGERROR_NOPACKAGE;
}

int PACKAGE_LIST::getMaxVersionNumber(string package_name)
{
	if (!versioningInitialized) initVersioning();
	for (int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).get_name() == package_name && packages.at(i).hasMaxVersion)
		{
			return i;//packages.at(i).get_id();//i;
		}
	}
	return MPKGERROR_NOPACKAGE;
}


void PACKAGE_LIST::destroy()
{
	packages.clear();
}
/*int PACKAGE_LIST::getCloneID(int testID)
{
	return cList.getCloneID(testID);
}*/
PACKAGE_LIST::PACKAGE_LIST()
{
	versioningInitialized=false;
	priorityInitialized=false;
}
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
	
	actionBus.setCurrentAction(ACTIONID_VERSIONBUILD);
	int pkgSize = packages.size();
	for (int i=0; i<pkgSize; i++)
	{

		actionBus.actions.at(actionBus.getActionPosition(ACTIONID_VERSIONBUILD))._currentProgress=i;
		packages.at(i).clearVersioning();
		for (int j=0; j<pkgSize; j++)
		{
			// Если это не тот же пакет и имена совпадают - добавляем номер в список
			if (i!=j && strcmp(packages.at(i).get_name().c_str(), packages.at(j).get_name().c_str())==0)
			{
				if (packages.at(j).available() || packages.at(j).installed()) packages.at(i).alternateVersions.push_back(j);
			}
		}
	}

	// Шаг второй. Для каждого пакета ищем максимальную версию
	string max_version; // Переменная содержащая максимальную версию
	int max_version_id; // номер пакета содержавшего максимальную версию
	string this_version;
	string installed_version;
	try {
	for (int i=0; i<packages.size();i++)
	{
		debug("initVersioning [stage 2]: step "+IntToStr(i));
		max_version.clear();
		max_version_id=-1;
		this_version.clear();
		installed_version.clear();
		if (packages.at(i).installed())
		{
			installed_version = packages.at(i).get_fullversion();
		}

		// Если у пакета нет других версий - значит максимальную версию имеет он
		
		if (packages.at(i).alternateVersions.empty())
		{
			max_version = packages.at(i).get_fullversion();
			packages.at(i).hasMaxVersion=true;
			max_version_id = i;
		}
		else
		{
			for (int j=0; j<packages.at(i).alternateVersions.size(); j++)
			{
				this_version = packages.at(packages.at(i).alternateVersions.at(j)).get_fullversion();
				if (packages.at(packages.at(i).alternateVersions.at(j)).installed())
				{
					installed_version = packages.at(packages.at(i).alternateVersions.at(j)).get_fullversion();
				}
				if (strverscmp(this_version.c_str(), max_version.c_str())>0)
				{
					max_version = this_version;
					max_version_id = packages.at(i).alternateVersions.at(j);
				}
			}
			if (max_version.empty()) // Если максимальной версии так и не нашлось (все пакеты - одинаковой версии) - то ставим максимум текущему
			{
				max_version = packages.at(i).get_version();
				max_version_id = i;
			}
			else
			{
				// Проверим - а вдруг именно этот пакет имеет максимальную версию?
				this_version = packages.at(i).get_fullversion();// + packages[i].get_build();
				if (strverscmp(this_version.c_str(), max_version.c_str())>0)
				{
					max_version = this_version;
					max_version_id = i;
				}
				// Устанавливаем найденному пакету нужные флаги
			}
		}
		// Запишем установленную версию
		packages.at(max_version_id).hasMaxVersion=true;
		packages.at(i).maxVersion=max_version;
		packages.at(i).installedVersion = installed_version;
	}
	actionBus.setActionState(ACTIONID_VERSIONBUILD);
	versioningInitialized=true;

	} //try

	catch(...)
	{
		printf("Range check error: max_version_id=%d\n", max_version_id);
	}

}

bool meetVersion(versionData condition, string packageVersion)
{
	int iCondition=atoi(condition.condition.c_str());
	string version1=packageVersion;
	string version2=condition.version;
	switch (iCondition)
	{
		case VER_MORE:
			if (strverscmp(version1.c_str(),version2.c_str())>0)  return true;
			else return false;
			break;
		case VER_LESS:
			if (strverscmp(version1.c_str(),version2.c_str())<0) return true;
			else return false;
			break;
		case VER_EQUAL:
			if (strverscmp(version1.c_str(),version2.c_str())==0) return true;
			else return false;
			break;
		case VER_NOTEQUAL:
			if (strverscmp(version1.c_str(),version2.c_str())!=0) return true;
			else return false;
			break;
		case VER_XMORE:
			if (strverscmp(version1.c_str(),version2.c_str())>=0) return true;
			else return false;
			break;
		case VER_XLESS:
			if (strverscmp(version1.c_str(),version2.c_str())<=0) return true;
			else return false;
			break;
		default:
			printf("%s: unknown condition %d!!!!!!!!!!!!!!!!!!!!!!!!!\n",__func__, iCondition);
			return true;
	}
	return true;
}


vector<unsigned int> checkedPackages;
bool notTested(int num)
{
	for (unsigned int i=0; i<checkedPackages.size(); i++)
	{
		if (checkedPackages.at(i)==num) return false;
	}
	return true;
}
int PACKAGE_LIST::getPackageNumberByName(string name)
{
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages.at(i).get_name()==name) return i;
	}
	printf("%s: No such package %s\n", __func__, name.c_str());
//	abort();
	return -1;
}
void PACKAGE_LIST::buildDependencyOrder()
{
	int pkgSize = this->size();
	for (int i=0; i<pkgSize; i++)
	{
		get_max_dtree_length(this, i);
	}
	priorityInitialized=true;
}

int get_max_dtree_length(PACKAGE_LIST *pkgList, int package_id)
{
	PACKAGE *_p = pkgList->get_package(package_id);
	int ret=0;
	int max_ret=-1;
	int pkgNum;
	//if (dependencies.size()>0) ret = 1;
	for (int i=0; i<_p->get_dependencies()->size(); i++)
	{
		pkgNum = pkgList->getPackageNumberByName(_p->get_dependencies()->get_dependency(i)->get_package_name());
		if (pkgNum>=0)
		{
			ret = 1 + get_max_dtree_length(pkgList, pkgNum);
			if (max_ret < ret) max_ret = ret;
		}
	}
	_p->priority=ret;
	return ret;
}






