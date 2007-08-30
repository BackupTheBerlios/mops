/*
	MOPSLinux packaging system
	Data types descriptions
	$Id: dataunits.cpp,v 1.69 2007/08/30 21:46:48 i27249 Exp $
*/



#include "dataunits.h"
#include "dependencies.h"
#include "conditions.h"
#include "debug.h"
#include "constants.h"

// LOCATION class functions

// Comparsion
bool LOCATION::equalTo(LOCATION *location)
{
	if (server_url!=*location->get_server_url()) return false;
	if (location_path!=*location->get_path()) return false;
	return true;
}

// Data retrieving

int LOCATION::get_id()
{
	return location_id;
}

int LOCATION::get_type()
{
	if (server_url.find("file://")!=std::string::npos) return SRV_FILE;
	if (server_url.find("local://")!=std::string::npos) return SRV_LOCAL;
	if (server_url.find("cdrom://")!=std::string::npos) return SRV_CDROM;
	return SRV_NETWORK;
}

bool LOCATION::get_local()
{
	return local;
}

string* LOCATION::get_path()
{
	return &location_path;
}

string* LOCATION::get_server_url()
{
	return &server_url;
}

string LOCATION::get_full_url()
{
	return server_url + location_path;
}

// Data writing
void LOCATION::set_id(int id)
{
	location_id=id;
}

void LOCATION::set_path(string* path)
{
	if (path->at(path->length()-1)!='/') location_path=*path + "/";
	else location_path=*path;
}

void LOCATION::set_local()
{
	local=true;
}

void LOCATION::unset_local()
{
	local=false;
}

void LOCATION::set_server_url(string* new_url)
{
	server_url=*new_url;
}


// Dimensions, empty, etc
bool LOCATION::IsEmpty()
{
	return location_path.empty();
}

void LOCATION::clear()
{
	location_id=0;
	server_url.clear();
	location_path.clear();
	local=false;
}


// Constructor & destructor
LOCATION::LOCATION()
{
	local=false;
	location_id=0;
}
LOCATION::~LOCATION()
{
}


// DEPENDENCY class functions

// Comparsion
bool DEPENDENCY::equalTo(DEPENDENCY *dependency)
{
	if (dependency_package_name!=*dependency->get_package_name()) return false;
	if (version_data.version!=*dependency->get_package_version()) return false;
	if (version_data.condition!=*dependency->get_condition()) return false;
	if (dependency_type!=*dependency->get_type()) return false;
	if (dependency_broken!=dependency->get_broken()) return false;
	return true;
}

// Data retrieving
int DEPENDENCY::get_id()
{
	return dependency_id;
}

string* DEPENDENCY::get_condition()
{
	return &version_data.condition;
}

string* DEPENDENCY::get_type()
{
	return &dependency_type;
}

string* DEPENDENCY::get_package_name()
{
	return &dependency_package_name;
}

string* DEPENDENCY::get_package_version()
{
	return &version_data.version;
}

int DEPENDENCY::get_broken()
{
	return dependency_broken;
}

versionData* DEPENDENCY::get_version_data()
{
	return &version_data;
}

// Visual data retriveal (UI purposes)
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

string DEPENDENCY::getDepInfo()
{
	return dependency_package_name + " " + get_vcondition() + " " + *get_package_version();
};

// Data writing
void DEPENDENCY::set_id(int id)
{
	dependency_id=id;
}

void DEPENDENCY::set_condition(string* condition)
{
	version_data.condition=*condition;
}

void DEPENDENCY::set_type(string* type)
{
	dependency_type=*type;
}

void DEPENDENCY::set_package_name(string* package_name)
{
	dependency_package_name=*package_name;
}

void DEPENDENCY::set_package_version(string* package_version)
{
	version_data.version=*package_version;
}

void DEPENDENCY::set_broken(int broken)
{
	dependency_broken=broken;
}

// Emptyness, etc

bool DEPENDENCY::IsEmpty()
{
	return dependency_package_name.empty();
}


void DEPENDENCY::clear()
{
	dependency_broken=0;
	version_data.version.clear();
	version_data.condition.clear();
	dependency_package_name.clear();
	dependency_type.clear();
	dependency_id=0;
}

// Constructor & destructor
DEPENDENCY::DEPENDENCY()
{
	dependency_id=0;
	dependency_broken=0;
}
DEPENDENCY::~DEPENDENCY(){
}


// FILES class

// Comparsion
bool FILES::equalTo(FILES *file)
{
	if (file_name!=*file->get_name()) return false;
	else return true;
}

// Data retriveal
int FILES::get_id()
{
	return file_id;
}

string* FILES::get_name()
{
	return &file_name;
}

int FILES::get_type()
{
	return file_type;
}

string* FILES::get_backup_file()
{
	return &backup_file;
}

bool FILES::config()
{
	if (file_type==FTYPE_CONFIG) return true;
	else return false;
}


// Data writing
void FILES::set_id(int id)
{
	file_id=id;
}

void FILES::set_name(string* name)
{
	file_name=*name;
}

void FILES::set_type(int type)
{
	file_type=type;
}

void FILES::set_backup_file(string* fname)
{
	backup_file=*fname;
}

// Empty, clear
bool FILES::IsEmpty()
{
	return file_name.empty();
}

void FILES::clear()
{
	file_id=0;
	file_type=0;
	file_name.clear();
	backup_file.clear();
}

// Constructor & destructor
FILES::FILES()
{
	file_id=0;
	file_type=FTYPE_PLAIN;
}

FILES::~FILES()
{
}
void _sortLocations(vector<LOCATION>* locations)
{
	// Sorting order:
	// 0: local://
	// 1: file://
	// 2: <all network>
	// 3: cdrom://
	vector<LOCATION> sorted;
	for (int t=0; t<=3; t++)
	{
		for (unsigned int i=0; i<locations->size(); i++)
		{
			if (locations->at(i).get_type()==t)
			{
				sorted.push_back(locations->at(i));
			}
		}
	}
	*locations=sorted;
}
				

void PACKAGE::sortLocations()
{
	_sortLocations(&package_locations);
}

bool PACKAGE::isTaggedBy(string tag)
{
	for (unsigned int i=0; i<package_tags.size(); i++)
	{
		if (package_tags[i]==tag) return true;
	}
	return false;
}

// Comparsion
bool PACKAGE::equalTo (PACKAGE *npkg)
{
	if (package_md5!=*npkg->get_md5()) return false;
	return true;
}
bool PACKAGE::locationsEqualTo(PACKAGE *pkg)
{
	if (package_locations.size()!=pkg->get_locations()->size()) return false;
	for (unsigned int i=0; i<package_locations.size(); i++)
	{
		for (unsigned int j=0; j<pkg->get_locations()->size(); j++)
		{
			if (!package_locations[i].equalTo(&pkg->get_locations()->at(j)))
				return false;
		}
	}
	return true;
}
bool PACKAGE::tagsEqualTo(PACKAGE *pkg)
{
	if (package_tags.size()!=pkg->get_tags()->size()) return false;
	for (unsigned int i=0; i<package_tags.size(); i++)
	{
		for (unsigned int j=0; j<pkg->get_tags()->size(); j++)
		{
			if (package_tags[i]!=pkg->get_tags()->at(j)) return false;
		}
	}
	return true;
}

bool PACKAGE::depsEqualTo(PACKAGE *pkg)
{
	if (package_dependencies.size()!=pkg->get_dependencies()->size()) return false;
	for (unsigned int i=0; i<package_tags.size(); i++)
	{
		for (unsigned int j=0; j<pkg->get_dependencies()->size(); j++)
		{
			if (!package_dependencies[i].equalTo(&pkg->get_dependencies()->at(j))) return false;
		}
	}
	return true;
}

// Data retriveal
bool PACKAGE::isItRequired(PACKAGE *testPackage)
{
	for (unsigned int i=0; i<package_dependencies.size(); i++)
	{
		if (*package_dependencies[i].get_package_name() == *testPackage->get_name() && \
				meetVersion(package_dependencies[i].get_version_data(), testPackage->get_version()))
			return true;
	}
	return false;
}

bool PACKAGE::isUpdate()
{
	if (!installed() && hasMaxVersion && !installedVersion.empty() && installedVersion != maxVersion)
	{
		return true;
	}
	else return false;
}

bool PACKAGE::deprecated()
{
	return !hasMaxVersion;
}

int PACKAGE::get_id()
{
	return package_id;
}

string* PACKAGE::get_name()
{
	return &package_name;
}

string* PACKAGE::get_version()
{
	return &package_version;
}

string* PACKAGE::get_arch()
{
	return &package_arch;
}

string* PACKAGE::get_build()
{
	return &package_build;
}

string* PACKAGE::get_compressed_size()
{
	return &package_compressed_size;
}

string* PACKAGE::get_installed_size()
{
	return &package_installed_size;
}

string* PACKAGE::get_short_description()
{
	return &package_short_description;
}

string* PACKAGE::get_description()
{
	return &package_description;
}

string* PACKAGE::get_changelog()
{
	return &package_changelog;
}

string* PACKAGE::get_packager()
{
	return &package_packager;
}

string* PACKAGE::get_packager_email()
{
	return &package_packager_email;
}

bool PACKAGE::available(bool includeLocal)
{
	if (package_locations.empty()) return false;
	else
	{
		if (includeLocal) return true;
		for (unsigned int i=0; i<package_locations.size(); i++)
		{
			if (package_locations[i].get_type()!=SRV_LOCAL)
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

	string stat;
	if (available())
	{
		if (color) stat+=CL_6;
		stat +="A";
		if (color) stat+=CL_WHITE;
	}
	else stat+="_";

	if (installed()) 
	{
		if (color) stat+=CL_GREEN;
		stat += "I";
		if (color) stat+=CL_WHITE;
	}
	else {
		stat += "_";
	}
	if (configexist())
	{
		if (color) stat+=CL_BLUE;
		stat += "C";
		if (color) stat+=CL_WHITE;
	}
	else stat+="_";
	
	switch(action())
	{
		case ST_INSTALL:
			if (color) stat+=CL_YELLOW;
			stat+="i";
			if (color) stat+=CL_WHITE;
			break;
		case ST_REMOVE:
			if (color) stat+=CL_RED;
			stat+="r";
			if (color) stat+=CL_WHITE;
			break;
		case ST_PURGE:
			if (color) stat+=CL_8;
			stat+="p";
			if (color) stat+=CL_WHITE;
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


string* PACKAGE::get_md5()
{
	return &package_md5;
}

string* PACKAGE::get_filename()
{
	return &package_filename;
}

int PACKAGE::get_err_type()
{
	return package_err_type;
}

void PACKAGE::set_err_type(int err)
{
	package_err_type=err;
}


void PACKAGE::set_broken(bool flag)
{
	isBroken=flag;
}
void PACKAGE::set_requiredVersion(versionData *reqVersion)
{
	requiredVersion = *reqVersion;
}

string PACKAGE::get_fullversion()
{
	return *get_version() + " build " + *get_build();
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
	package_installed=0;
	package_configexist=0;
	package_action=0;
	package_md5.clear();
	package_filename.clear();
	package_files.clear();
	package_locations.clear();
	package_dependencies.clear();
	package_tags.clear();
}

void PACKAGE::add_dependency(string *package_name, string* dep_condition, string *package_version)
{
	DEPENDENCY dep;
	dep.set_package_name(package_name);
	dep.set_package_version(package_version);
	dep.set_condition(dep_condition);
	for (unsigned int i=0;i<package_dependencies.size();i++)
	{
		if (*package_dependencies.at(i).get_package_name()==*dep.get_package_name() && \
		    *package_dependencies.at(i).get_package_version()==*dep.get_package_version())
		       	return;
	}
	package_dependencies.push_back(dep);
}

void PACKAGE::add_file(string* file_name)
{
	FILES file;
	file.set_name(file_name);
	package_files.push_back(file);
}

void PACKAGE::add_tag(string* tag)
{
	package_tags.push_back(*tag);
}

void PACKAGE::set_id(int id)
{
	package_id=id;
}

void PACKAGE::set_name(string* name)
{
	package_name=*name;
}

void PACKAGE::set_version(string* version)
{
	package_version=*version;
}

void PACKAGE::set_arch(string* arch)
{
	package_arch=*arch;
}

void PACKAGE::set_build(string* build)
{
	package_build=*build;
}

void PACKAGE::set_compressed_size(string* compressed_size)
{
	package_compressed_size=*compressed_size;
}

void PACKAGE::set_installed_size(string* installed_size)
{
	package_installed_size=*installed_size;
}

void PACKAGE::set_short_description(string* short_description)
{
	package_short_description=*short_description;
}

void PACKAGE::set_description(string* description)
{
	package_description=*description;
}

void PACKAGE::set_changelog(string* changelog)
{
	package_changelog=*changelog;
}

void PACKAGE::set_packager(string* packager)
{
	package_packager=*packager;
}

void PACKAGE::set_packager_email(string* packager_email)
{
	package_packager_email=*packager_email;
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

void PACKAGE::set_md5(string* md5)
{
	package_md5=*md5;
}

void PACKAGE::set_filename(string* filename)
{
	package_filename=*filename;
}

#ifdef ENABLE_INTERNATIONAL
vector<DESCRIPTION>* PACKAGE::get_descriptions()
{
	return &package_descriptions;
}

int PACKAGE::set_descriptions(vector<DESCRIPTION>* desclist)
{
	package_descriptions=*desclist;
}
#endif

vector<FILES>* PACKAGE::get_files()
{
	return &package_files;
}

vector<FILES>* PACKAGE::get_config_files()
{
	return &config_files;
}


vector<DEPENDENCY>* PACKAGE::get_dependencies()
{
	return &package_dependencies;
}

vector<LOCATION>* PACKAGE::get_locations()
{
	return &package_locations;
}

vector<string>* PACKAGE::get_tags()
{
	return &package_tags;
}

string PACKAGE::get_scriptdir()
{
	return SCRIPTS_DIR + package_filename + "_" + package_md5 + "/";
}


void PACKAGE::set_files(vector<FILES>* files)
{
	package_files=*files;
}

void PACKAGE::set_config_files(vector<FILES>* conf_files)
{
	config_files=*conf_files;
}

void PACKAGE::sync()
{
	for (unsigned int i=0; i< config_files.size(); i++)
	{
		for (unsigned int t=0; t<package_files.size(); t++)
		{
			if (*config_files[i].get_name()=='/' + *package_files[t].get_name())
			{
				printf("set file %s as config\n", config_files[i].get_name()->c_str());
				package_files[t].set_type(FTYPE_CONFIG);
				break;
			}
		}
	}

	if (config_files.empty())
	{
		for (unsigned int i=0; i < package_files.size(); i++)
		{
			if (package_files[i].get_type()==FTYPE_CONFIG)
			{
				config_files.push_back(package_files[i]);
			}
		}
	}
}

void PACKAGE::set_dependencies(vector<DEPENDENCY>* dependencies)
{
	package_dependencies=*dependencies;
}

void PACKAGE::set_locations(vector<LOCATION>* locations)
{
	package_locations=*locations;
}

void PACKAGE::set_tags(vector<string>* tags)
{
	package_tags=*tags;
}

bool PACKAGE::IsEmpty()
{
	return package_name.empty();
}

bool PACKAGE::isRemoveBlacklisted()
{
	for (unsigned int i=0; i<removeBlacklist.size(); i++)
	{
		if (removeBlacklist[i]==this->package_name) return true;
	}
	return false;
}


PACKAGE::PACKAGE()
{
	isUpdating=false;
	isBroken = false;
	isRequirement = false;
	package_id=-1;
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


PACKAGE * PACKAGE_LIST::getPackageByID(int id)
{
	for (int i=0; i<packages.size(); i++)
	{
		if (packages[i].get_id()==id) return &packages[i];
	}
	mError("No such id " + IntToStr(id) + " in package list");
	return NULL;
}
void PACKAGE_LIST::sortByPriority(bool reverse_order)
{
//	printf("sorting priority\n");
	
	if (!priorityInitialized) buildDependencyOrder();
	int min_priority = 0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages[i].priority>min_priority) min_priority = packages[i].priority;
	}

//	printf("continue sorting, packages.size = %d, min_priority = %d...\n",packages.size(),min_priority);
	vector<PACKAGE> sorted;
	if (!reverse_order)
	{	
		for (int p=0; p<=min_priority; p++)
		{
			//printf("!rev_order, p=%d, min_priority=%d\n",p,min_priority);
			for (unsigned int i=0; i<packages.size(); i++)
			{
			//	printf("pkg[%d]\n",i);
				if (packages[i].priority==p) sorted.push_back(packages[i]);
			}
		}
	}
	else
	{
		for (int p=min_priority; p>=0; p--)
		{
			for (unsigned int i=0; i<packages.size(); i++)
			{
				if (packages[i].priority==p) sorted.push_back(packages[i]);
			}
		}
	}
	//printf("sorting finished\n");
	packages = sorted;
}




double PACKAGE_LIST::totalCompressedSize()
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		ret+=strtod(packages[i].get_compressed_size()->c_str(), NULL);
	}
	return ret;
}


double PACKAGE_LIST::totalInstalledSize()
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		ret+=strtod(packages[i].get_installed_size()->c_str(), NULL);
	}
	return ret;
}

double PACKAGE_LIST::totalCompressedSizeByAction(int select_action)
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages[i].action()==select_action) ret+=strtod(packages[i].get_compressed_size()->c_str(), NULL);
	}
	return ret;
}
double PACKAGE_LIST::totalInstalledSizeByAction(int select_action)
{
	double ret=0;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages[i].action()==select_action)
		{
			ret+=strtod(packages[i].get_installed_size()->c_str(), NULL);
		}
	}
	return ret;
}


int PACKAGE_LIST::getPackageNumberByMD5(string* md5)
{
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (*packages[i].get_md5()==*md5) return i;
	}
	return -1;
}
void PACKAGE_LIST::clearVersioning()
{
	for (unsigned int i=0; i<packages.size(); i++)
	{
		packages[i].clearVersioning();
	}
}

bool PACKAGE_LIST::equalTo (PACKAGE_LIST *nlist)
{
	if (size()!=nlist->size()) return false;
	for (int i=0; i<size(); i++)
	{
		if (!packages[i].equalTo(nlist->get_package(i))) return false;
	}
	return true;
}

bool PACKAGE_LIST::hasInstalledOnes()
{
	if (packages.size()==0) return false;
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages[i].installed())
		{
			return true;
		}
	}
	return false;
}

PACKAGE* PACKAGE_LIST::getInstalledOne()
{
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (packages[i].installed()) return &packages[i];
	}
	mError("getInstalledOne: no installed ones");
	return NULL;
}

PACKAGE* PACKAGE_LIST::getMaxVersion()
{
	if (packages.size()>0)	return &packages[getMaxVersionNumber(packages[0].get_name())];
	else
	{
		mError("Error in getMaxVersion: list empty");
		return NULL;
	}
}


PACKAGE* PACKAGE_LIST::get_package(int num)
{
		return &packages[num];
}

PACKAGE* PACKAGE_LIST::getPackageByTableID(unsigned int id)
{
	if (id >=packages.size())
	{
		mError("No such id");
		return &__empty;
	}
	return &packages[tableID[id]];
}

void PACKAGE_LIST::setTableID(int pkgNum, int id)
{
	tableID[id]=pkgNum;
	reverseTableID[pkgNum]=id;
}

int PACKAGE_LIST::getTableID(int pkgNum)
{
	return reverseTableID[pkgNum];
}

int PACKAGE_LIST::getRealNum(int id)
{
	return tableID[id];
}
void PACKAGE_LIST::set_package(int num, PACKAGE* package)
{
	if ((unsigned int) num>=packages.size())
	{
		mError("Incorrect num "+IntToStr(num));
		return;
	}
	packages[num]=*package;
	setTableID(num, num);
}

int PACKAGE_LIST::size()
{
	return packages.size();
}

void PACKAGE_LIST::add(PACKAGE* package)
{
    packages.push_back(*package);
    setTableID(packages.size()-1, packages.size()-1);
}
void PACKAGE_LIST::push_back(PACKAGE_LIST plist)
{
	add(&plist);
}
void PACKAGE_LIST::add(PACKAGE_LIST *pkgList)
{
	add_list(pkgList, false);
}
void PACKAGE_LIST::add_list(PACKAGE_LIST *pkgList, bool skip_identical)
{
	// IMPORTANT NOTE!
	// If skip_identical is true, the locations will be MERGED together!
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
				if (*packages[s].get_md5()==*pkgList->get_package(i)->get_md5())
				{
					identical_found=true;
					// Comparing locations and merging
					for (unsigned int l=0; l<pkgList->get_package(i)->get_locations()->size(); l++)
					{
						location_found=false;
						for (unsigned int ls=0; ls<packages[s].get_locations()->size(); ls++)
						{
							if (packages[s].get_locations()->at(ls).equalTo(&pkgList->get_package(i)->get_locations()->at(l)))
							{
								location_found=true;
								break;
							}
						}
						if (!location_found)
						{
							packages[s].get_locations()->push_back(pkgList->get_package(i)->get_locations()->at(l));
						}
					}
					break;
				}

			}
		}
		if (!identical_found)
		{
			this->add(pkgList->get_package(i));
		}
	}
}


void PACKAGE_LIST::clear(unsigned int new_size)
{
	packages.clear();
    	packages.resize(new_size);
	tableID.clear();
	reverseTableID.clear();
}

void PACKAGE_LIST::set_size(unsigned int new_size)
{
	packages.resize(new_size);
	for (unsigned int i=0; i<new_size; i++)
	{
		setTableID(i,i);
	}

}

bool PACKAGE_LIST::IsEmpty()
{
	return packages.empty();
}

vector<DEPENDENCY>* PACKAGE_LIST::getDepList(int i)
{
	return packages[i].get_dependencies();
}

PACKAGE* PACKAGE_LIST::findMaxVersion()
{
	string max_version;
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
	return &packages[id];
}

int PACKAGE_LIST::getMaxVersionID(string* package_name)
{
	if (!versioningInitialized) initVersioning();
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (*packages[i].get_name() == *package_name && packages[i].hasMaxVersion)
		{
			return packages[i].get_id();
		}
	}
	return MPKGERROR_NOPACKAGE;
}

int PACKAGE_LIST::getMaxVersionNumber(string* package_name)
{
	if (!versioningInitialized) initVersioning();
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (*packages[i].get_name() == *package_name && packages[i].hasMaxVersion)
		{
			return i;
		}
	}
	return MPKGERROR_NOPACKAGE;
}


PACKAGE_LIST::PACKAGE_LIST()
{
	versioningInitialized=false;
	priorityInitialized=false;
}
PACKAGE_LIST::~PACKAGE_LIST()
{
}

/*
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
		mError("DESCRIPTION_LIST: Error in range!");
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
*/
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
	
	int pkgSize = packages.size();
	for (int i=0; i<pkgSize; i++)
	{

		packages[i].clearVersioning();
		for (int j=0; j<pkgSize; j++)
		{
			// Если это не тот же пакет и имена совпадают - добавляем номер в список
			if (i!=j && *packages[i].get_name()==*packages[j].get_name())
			{
				if (packages[j].reachable()) packages[i].alternateVersions.push_back(j);
			}
		}
	}

	// Шаг второй. Для каждого пакета ищем максимальную версию
	string max_version; // Переменная содержащая максимальную версию
	int max_version_id; // номер пакета содержавшего максимальную версию
	string this_version;
	string installed_version;
	for (unsigned int i=0; i<packages.size();i++)
	{
		//mDebug("initVersioning [stage 2]: step "+IntToStr(i));
		max_version.clear();
		max_version_id=-1;
		this_version.clear();
		installed_version.clear();
		if (packages[i].installed())
		{
			installed_version = packages[i].get_fullversion();
		}

		// Если у пакета нет других версий - значит максимальную версию имеет он
		
		if (packages[i].alternateVersions.empty())
		{
			max_version = packages[i].get_fullversion();
			packages[i].hasMaxVersion=true;
			max_version_id = i;
		}
		else
		{
			for (unsigned int j=0; j<packages[i].alternateVersions.size(); j++)
			{
				this_version = packages[packages[i].alternateVersions[j]].get_fullversion();
				if (packages[packages[i].alternateVersions[j]].installed())
				{
					installed_version = packages[packages[i].alternateVersions[j]].get_fullversion();
				}
				if (strverscmp(this_version.c_str(), max_version.c_str())>0)
				{
					max_version = this_version;
					max_version_id = packages[i].alternateVersions[j];
				}
			}
			if (max_version.empty()) // Если максимальной версии так и не нашлось (все пакеты - одинаковой версии) - то ставим максимум текущему
			{
				max_version = packages[i].get_fullversion();	// Fix (18.05.2007): get_version() was used instead of get_fullversion()
				max_version_id = i;
			}
			else
			{
				// Проверим - а вдруг именно этот пакет имеет максимальную версию?
				this_version = packages[i].get_fullversion();
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
	versioningInitialized=true;
}

bool meetVersion(versionData *condition, string *packageVersion)
{
	int iCondition=atoi(condition->condition.c_str());
	string version1=*packageVersion;
	string version2=condition->version;
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
		case VER_ANY:
			return true;
			break;
		default:
			mError((string) __func__ + ": unknown condition " + IntToStr(iCondition));
			return true;
	}
	return true;
}

vector<unsigned int> checkedPackages;
bool notTested(unsigned int num)
{
	for (unsigned int i=0; i<checkedPackages.size(); i++)
	{
		if (checkedPackages[i]==num) return false;
	}
	return true;
}
int PACKAGE_LIST::getPackageNumberByName(string *name)
{
	for (unsigned int i=0; i<packages.size(); i++)
	{
		if (*packages[i].get_name()==*name) return i;
	}
	mDebug("No such package " + *name);
	return -1;
}
void PACKAGE_LIST::buildDependencyOrder()
{
	int pkgSize = this->size();
	vector<int> callList;
	for (int i=0; i<pkgSize; i++)
	{
		callList.clear();
		callList.push_back(i);
		//printf("%s cycle %d\n", __func__, i);
		get_max_dtree_length(this, i, callList);
	}
//	printf("%s: build complete\n",__func__);
	priorityInitialized=true;
}

int get_max_dtree_length(PACKAGE_LIST *pkgList, int package_id, vector<int> callList)
{
	// Check for looping
//	printf("callList size=%d\n", callList.size());
	bool loop=false;

	for (unsigned int i=0; i<callList.size(); i++)
	{
//		printf("callList[%d]==%d\n", i,callList[i]);
		for (unsigned int t=0; t<callList.size(); t++)
		{
//			printf("Comparing with %d\n", callList[t]);
			if (i!=t && callList[i]==callList[t])
			{
//				printf("Loop dependencies!\n");
				loop=true;
				//return pkgList->get_package(package_id)->priority;
			}
		}
	}
//	printf("%s start for package %d\n",__func__, package_id);
	PACKAGE *_p = pkgList->get_package(package_id);
	int ret=0;
	int max_ret=-1;
	int pkgNum;
	//if (dependencies.size()>0) ret = 1;
	for (unsigned int i=0; i<_p->get_dependencies()->size(); i++)
	{
		//printf("%s: cycle %d\n", __func__, i);
		pkgNum = pkgList->getPackageNumberByName(_p->get_dependencies()->at(i).get_package_name());
		if (pkgNum>=0)
		{
//			printf("callback\n");
			callList.push_back(pkgNum);
			if (!loop) ret = 1 + get_max_dtree_length(pkgList, pkgNum,callList);
			if (max_ret < ret) max_ret = ret;
		}
	}
	_p->priority=ret;
	return ret;
}

