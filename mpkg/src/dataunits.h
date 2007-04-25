/*
	MOPSLinux packaging system
	Data types descriptions
	$Id: dataunits.h,v 1.17 2007/04/25 08:26:12 i27249 Exp $
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

	bool operator != (SERVER_TAG stag);
	bool operator == (SERVER_TAG stag);

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

	bool operator != (SERVER_TAG_LIST slist);
	bool operator == (SERVER_TAG_LIST slist);


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

	bool operator != (SERVER nserv);
	bool operator == (SERVER nserv);
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
	bool operator != (SERVER_LIST serv_list);
	bool operator == (SERVER_LIST serv_list);

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
	bool operator != (LOCATION location);
	bool operator == (LOCATION location);

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
	bool operator != (LOCATION_LIST nloc);
	bool operator == (LOCATION_LIST nloc);
	LOCATION* get_location(int num);
	vector<LOCATION> get_locations();
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

	bool operator != (DEPENDENCY ndep);
	bool operator == (DEPENDENCY ndep);
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
	
	bool operator != (TAG ntag);
	bool operator == (TAG ntag);
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
	int file_type;

	// EXTERNAL DATA //
	//PACKAGE_LIST *file_packages;
    public:
	bool operator != (FILES nfile);
	bool operator == (FILES nfile);
	int get_id();
	string get_name(bool sql=true);
	int get_type();
	bool config();

	int set_id(int id);
	int set_name(string name);
	int set_type(int type);

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
	bool operator != (TAG_LIST ntags);
	bool operator == (TAG_LIST ntags);

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
	
	bool operator != (DEPENDENCY_LIST ndep);
	bool operator == (DEPENDENCY_LIST ndep);
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
	bool operator != (FILE_LIST nfiles);
	bool operator == (FILE_LIST nfiles);

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

class SCRIPTS
{
    private:
	int script_id;
	string preinstall;
	string postinstall;
	string preremove;
	string postremove;

    public:
	bool operator != (SCRIPTS scr);
	bool operator == (SCRIPTS scr);
	int get_id();
	string get_vid();
	string get_preinstall(bool sql=true);
	string get_postinstall(bool sql=true);
	string get_preremove(bool sql=true);
	string get_postremove(bool sql=true);

	void set_preinstall(string preinst);
	void set_postinstall(string postinst);
	void set_preremove(string prerem);
	void set_postremove(string postrem);
	void set_id(int id);
	void set_vid(string id);
	void clear();
	bool IsEmpty();

	SCRIPTS();
	~SCRIPTS();
};

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
		int set_id(int id);
		int set_language(string language);
		int set_text(string text);
		int set_shorttext(string short_text);
		int get_id();
		string get_language();
		string get_text(bool sql=true);
		string get_shorttext(bool sql=true);
		void clear();
};

class DESCRIPTION_LIST
{
	private:
		vector<DESCRIPTION> descriptions;
	public:
		DESCRIPTION *get_description(unsigned int num);
		int set_description(unsigned int num, DESCRIPTION description);
		int add(DESCRIPTION description);
		DESCRIPTION_LIST();
		~DESCRIPTION_LIST();
		unsigned int size();
		bool empty();
		void clear();
};

/*class dTreeItem
{
	public:
		int thisID;
		vector<*dTreeItem> childs;
		dTreeItem();
		~dTreeItem();
		void addChild(PACKAGE *pkg);
		PACKAGE *getChild(int package_id);
};
*/
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
	bool package_available;
	bool package_installed;
	bool package_configexist;
	int package_action;
	
	string package_md5;
	string package_filename;
	int package_err_type;
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
	bool available();
	bool installed();
	bool configexist();
	int action();
	bool reachable();	// A combination of package_available and package_installed. If at least one of them is true, package_reachable == true, otherwise false.
	string get_vstatus(bool color=false);
	string get_md5(bool sql=true);
	string get_filename(bool sql=true);
	int get_err_type();

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
	void set_available(bool flag = true);
	void set_installed(bool flag = true);
	void set_configexist(bool flag = true);
	void set_action(int new_action);
	int set_md5(string md5);
	int set_filename(string filename);
	int set_err_type(int err);

    private:
	// EXTERNAL DATA //
	FILE_LIST package_files;
	FILE_LIST config_files;
	LOCATION_LIST package_locations;
	DEPENDENCY_LIST package_dependencies;
	DESCRIPTION_LIST package_descriptions;
	TAG_LIST package_tags;
	SCRIPTS package_scripts;
    public:
	DESCRIPTION_LIST* get_descriptions();
	int set_descriptions(DESCRIPTION_LIST desclist);
	FILE_LIST* get_config_files();
	FILE_LIST* get_files();
	LOCATION_LIST* get_locations();
	DEPENDENCY_LIST* get_dependencies();
	TAG_LIST* get_tags();
	SCRIPTS* get_scripts();

	bool operator == (PACKAGE npkg);
	bool operator != (PACKAGE npkg);

	int set_config_files(FILE_LIST conf_files);
	int set_files(FILE_LIST files);
	int set_locations(LOCATION_LIST locations);
	int set_dependencies(DEPENDENCY_LIST dependencies);
	int set_tags(TAG_LIST tags);
	int set_scripts(SCRIPTS scripts);

	int add_dependency(string package_name, string dep_condition, string package_version);
	int add_file(string file_name);
	int add_tag(string tag);

	// CLASS ROUTINES //
	void destroy();
	void clear();
	bool IsEmpty();
	void sync();
	PACKAGE();
	~PACKAGE();
};
class cloneList
{
	public:
		vector<int>whoHasClones_IDs; 	// Список объектов, имеющих клоны
		vector< vector<int> >objectCloneListID; 	// Список клонов каждого объекта
		vector<int>masterCloneID;	// ID клона, имеющего максимальную версию.
		vector<int>installedCloneID;	// ID установленного клона. Если такого нету, ставится равным -1

		//cloneList(PACKAGE_LIST *pkgList); // Конструктор, здесь создается каталог клонов
		cloneList();
		bool initialized;
		void init(vector<PACKAGE> &pkgList);
		~cloneList();
		int getCloneID(int testID);
};

class PACKAGE_LIST
{
    private:
	vector<PACKAGE> packages;
    public:
	PACKAGE* get_package(int num);
	int set_package(int num, PACKAGE package);
	bool operator != (PACKAGE_LIST nlist);
	bool operator == (PACKAGE_LIST nlist);
	int add(PACKAGE package);
	int add_list(PACKAGE_LIST *pkgList, bool skip_identical=true);
	void clear(unsigned int new_size = 0);
	bool IsEmpty();
	int size();
	void set_size(unsigned int new_size);
	PACKAGE findMaxVersion();
	DEPENDENCY_LIST getDepList(int i);
	void destroy();
	void initClones();
	int getCloneID(int testID);
	cloneList cList;
	PACKAGE_LIST();
	~PACKAGE_LIST();
};


typedef int RESULT;
string IntToStr(int num);
#endif //DATAUNITS_H_

