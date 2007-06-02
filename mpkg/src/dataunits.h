/*
	MOPSLinux packaging system
	Basic data types descriptions
	Second edition: RISC architecture =)
	$Id: dataunits.h,v 1.41 2007/06/02 23:26:06 i27249 Exp $
*/


#ifndef DATAUNITS_H_
#define DATAUNITS_H_

#include <string>
#include <vector>
#include <map>
using namespace std;

// Server type definitions
// Note: the definition order is important. Set this to comply the sorting order (less value comes first)
#define SRV_LOCAL 0
#define SRV_FILE 1
#define SRV_NETWORK 2
#define SRV_CDROM 3

// class LOCATION: describes one location unit
class LOCATION
{
    private:
	// INTERNAL DATA //
	int location_id;
	string server_url;
	string location_path;
	bool local;
	
    public:
	// Comparsion
	bool equalTo (LOCATION *location);

	// Data retriveal
	int get_id();
	int get_type();
	bool get_local();
	string* get_path();
	string* get_server_url();
	string get_full_url();

	// Data writing
	void set_id(int id);
	void set_path(string* path);
	void set_local();
	void unset_local();
	void set_server_url(string * new_url);


	// Dimensions, empty, etc
	bool IsEmpty();
	void clear();

	// Constructor and destructor
	LOCATION();
	~LOCATION();
};

struct versionData
{
	string version;
	string condition;
};

class DEPENDENCY
{
    private:
	// INTERNAL DATA //
	int dependency_id;
	string dependency_type;
	string dependency_package_name;
	int dependency_broken;
	versionData version_data;

    public:
	// Comparsion
	bool equalTo(DEPENDENCY *dependency);

	// Data retriveal
	int get_id();
	string *get_condition();
	string *get_type();
	string *get_package_name();
	string *get_package_version();
	int get_broken();
	versionData *get_version_data();

	// Visual data retriveal (UI purposes)
	string get_vbroken();
	string get_vcondition();
	string getDepInfo();

	// Data writing
	void set_id(int id);
	void set_condition(string* condition);
	void set_type(string* type);
	void set_package_name(string* package_name);
	void set_package_version(string* package_version);
	void set_broken(int broken);

	// Emptyness, empty, etc
	bool IsEmpty();
	void clear();

	// Constructor & destructor
	DEPENDENCY();
	~DEPENDENCY();
};

class FILES
{
    private:
	// INTERNAL DATA //
	int file_id;
	string file_name;
	int file_type;
	string backup_file;
    public:
	// Comparsion
	bool equalTo(FILES *file);

	// Data retriveal
	int get_id();
	string* get_name();
	int get_type();
	string* get_backup_file();
	bool config();

	// Data writing
	void set_id(int id);
	void set_name(string* name);
	void set_type(int type);
	void set_backup_file(string *fname);

	// Empty, clear
	bool IsEmpty();
	void clear();

	// Constructor && destructor
	FILES();
	~FILES();
};

#ifdef ENABLE_INTERNATIONAL
// Disabled for now - because it causes uncontrolled growing of the database. Maybe will store externally?
class DESCRIPTION
{
	private:
		int description_id;
		string description_language;
		string short_description_text;
		string description_text;
	public:
		DESCRIPTION();
		~DESCRIPTION();
		void set_id(int id);
		void set_language(string* language);
		void set_text(string* text);
		void set_shorttext(string* short_text);
		int get_id();
		string* get_language();
		string* get_text();
		string* get_shorttext();
		void clear();
};
#endif

void _sortLocations(vector<LOCATION>* locations); // Location sorting


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
	bool package_installed;
	bool package_configexist;
	int package_action;	
	string package_md5;
	string package_filename;
	int package_err_type;

	// EXTERNAL DATA //
	vector<FILES> package_files;
	vector<FILES> config_files;
	vector<LOCATION> package_locations;
	vector<DEPENDENCY> package_dependencies;
	vector<string> package_tags;
#ifdef ENABLE_INTERNATIONAL
	vector<DESCRIPTION> package_descriptions;
#endif
    public:
	versionData requiredVersion;
	int priority;
	int itemID;	// For status purposes only, means the number in PackageData vector
	bool isBroken;
	bool isRequirement;
	vector<int>alternateVersions;
	bool hasMaxVersion;
	string maxVersion;
	bool newPackage;	// Используется пока что только при обновлении. Есть мысль использовать в GUI (еще не знаю по какому принципу)
	string installedVersion;

	bool deprecated();
	// Comparsion
	bool equalTo(PACKAGE *pkg);
	bool locationsEqualTo(PACKAGE *pkg);
	bool tagsEqualTo(PACKAGE *pkg);
	bool depsEqualTo(PACKAGE *pkg);

	// Data retrieving	
	bool isItRequired(PACKAGE *testPackage);
	bool isUpdate();
	int get_id();
	string *get_name();
	string *get_version();
	string *get_arch();
	string *get_build();
	string *get_compressed_size();
	string *get_installed_size();
	string *get_short_description();
	string *get_description();
	string *get_changelog();
	string *get_packager();
	string *get_packager_email();
	bool available(bool includeLocal=false);
	bool installed();
	bool configexist();
	int action();
	bool reachable(bool includeConfigFiles=false);	// ==(package_available || package_installed)
	string *get_md5();
	string *get_filename();
	int get_err_type();

	vector<FILES>* get_config_files();
	vector<FILES>* get_files();
	vector<LOCATION>* get_locations();
	vector<DEPENDENCY>* get_dependencies();
	vector<string>* get_tags();
	string get_scriptdir();

#ifdef ENABLE_INTERNATIONAL
	vector <DESCTIPTION> * get_descriptions();
#endif
	// UI functions
	string get_fullversion();
	string get_vstatus(bool color=false);

	// Data writing
	void set_broken(bool flag=true);
	void set_requiredVersion(versionData *reqVersion);
	void set_id(int id);
	void set_name(string *name);
	void set_version(string *version);
	void set_arch(string *arch);
	void set_build(string *build);
	void set_compressed_size(string *compressed_size);
	void set_installed_size(string *installed_size);
	void set_short_description(string *short_description);
	void set_description(string *description);
	void set_changelog(string *changelog);
	void set_packager(string *packager);
	void set_packager_email(string *packager_email);
	void set_installed(bool flag = true);
	void set_configexist(bool flag = true);
	void set_action(int new_action);
	void set_md5(string *md5);
	void set_filename(string *filename);
	void set_err_type(int err);

	void set_config_files(vector<FILES> *conf_files);
	void set_files(vector<FILES> *files);
	void set_locations(vector<LOCATION> *locations);
	void set_dependencies(vector<DEPENDENCY> *dependencies);
	void set_tags(vector<string> *tags);

	void add_dependency(string *package_name, string *dep_condition, string *package_version);
	void add_file(string *file_name);
	void add_tag(string *tag);

#ifdef ENABLE_INTERNATIONAL
	void set_descriptions(vector <DESCRIPTION> *desclist);
#endif


	// Internal structure methods
	void sortLocations();
	void clearVersioning();
	void sync();
	
	// Empty, clear, etc
	void clear();
	bool IsEmpty();
	
	// Constructor & destructor
	PACKAGE();
	~PACKAGE();
};
class PACKAGE_LIST
{
    private:
	vector<PACKAGE> packages;
	map<int, int> tableID;
	map<int, int> reverseTableID;
    public:
	bool priorityInitialized;
	void sortByPriority(bool reverse_order=false);
	void buildDependencyOrder();
	int getPackageNumberByName(string *name);
	double totalCompressedSize();
	double totalInstalledSize();
	double totalInstalledSizeByAction(int select_action);
	double totalCompressedSizeByAction(int select_action);
	bool hasInstalledOnes();
	PACKAGE *getInstalledOne();
	PACKAGE *getMaxVersion();
	bool versioningInitialized;
	PACKAGE* get_package(int num);
	PACKAGE* getPackageByTableID(unsigned int id);
	void setTableID(int pkgNum, int id);
	int getTableID(int pkgNum);
	int getRealNum(int id);
	void set_package(int num, PACKAGE* package);
	bool equalTo(PACKAGE_LIST *nlist);
	void add(PACKAGE *package);
	void push_back(PACKAGE_LIST plist);
	void add(PACKAGE_LIST *pkgList);
	void add_list(PACKAGE_LIST *pkgList, bool skip_identical=true);
	void clear(unsigned int new_size = 0);
	bool IsEmpty();
	int size();
	void set_size(unsigned int new_size);
	int getPackageNumberByMD5(string* md5);		// return number (NOT package ID!) of package in vector (if found). Else, returns -1.
	int getMaxVersionID(string* package_name); // Return package ID
	int getMaxVersionNumber(string* package_name);	// Return package number (in array)

	PACKAGE *findMaxVersion();
	vector<DEPENDENCY>* getDepList(int i);
	void initVersioning();
	void clearVersioning();
	PACKAGE_LIST();
	~PACKAGE_LIST();
};

bool meetVersion(versionData *condition, string *packageVersion);
int get_max_dtree_length(PACKAGE_LIST *pkgList, int package_id);

#endif //DATAUNITS_H_

