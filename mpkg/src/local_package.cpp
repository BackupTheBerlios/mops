#include "local_package.h"


string get_tmp_file()
{
	string tmp_fname;
	debug("get_tmp_file start");
	char *t=tmpnam(NULL);
	tmp_fname=t;
	debug("get_tmp_file end");
	//free(t);
	return tmp_fname;
}

LocalPackage::LocalPackage(string _f)
{
	debug("LocalPackage created");
	this->filename=_f;
}

LocalPackage::~LocalPackage(){}

int LocalPackage::get_xml()
{
	debug("get_xml start");
	string tmp_xml=get_tmp_file();
	string sys="tar zxf "+filename+" install/data.xml --to-stdout > "+tmp_xml;
	system(sys.c_str());
	PackageConfig p(tmp_xml);
	data.set_name(p.getName());
	data.set_version(p.getVersion());
	data.set_arch(p.getArch());
	data.set_build(p.getBuild());
	data.set_packager(p.getAuthorName());
	data.set_packager_email(p.getAuthorEmail());
	data.set_description(p.getDescription());
	data.set_short_description(p.getShortDescription());
	data.set_changelog(p.getChangelog());
	
	DEPENDENCY dep_tmp;
	DEPENDENCY suggest_tmp;
	TAG tag_tmp;

	vector<string> vec_tmp_names;
	vector<string> vec_tmp_conditions;
	vector<string> vec_tmp_versions;

	vec_tmp_names=p.getDepNames();
	vec_tmp_conditions=p.getDepConditions();
	vec_tmp_versions=p.getDepVersions();

	for (int i=0;i<vec_tmp_names.size();i++)
	{
		dep_tmp.set_package_name(vec_tmp_names[i]);
		dep_tmp.set_package_version(vec_tmp_versions[i]);
		dep_tmp.set_condition(IntToStr(condition2int(vec_tmp_conditions[i])));
		dep_tmp.set_type("DEPENDENCY");
		data.get_dependencies()->add(dep_tmp);
		dep_tmp.clear();
	}
	vec_tmp_names=p.getSuggestNames();
	vec_tmp_conditions=p.getSuggestConditions();
	vec_tmp_versions=p.getSuggestVersions();


	for (int i=0;i<vec_tmp_names.size();i++)
	{
		suggest_tmp.set_package_name(vec_tmp_names[i]);
		suggest_tmp.set_package_version(vec_tmp_versions[i]);
		suggest_tmp.set_condition(IntToStr(condition2int(vec_tmp_conditions[i])));
		suggest_tmp.set_type("SUGGEST");
		data.get_dependencies()->add(suggest_tmp);
		suggest_tmp.clear();
	}

	vec_tmp_names=p.getTags();
	for (int i=0;i<vec_tmp_names.size();i++)
	{
		tag_tmp.set_name(vec_tmp_names[i]);
		data.get_tags()->add(tag_tmp);
		tag_tmp.clear();
	}

	vec_tmp_names.clear();
	vec_tmp_conditions.clear();
	vec_tmp_versions.clear();
	debug("get_xml end");
	return 0;
}

int LocalPackage::get_filelist()
{
	debug("get_filelist start");
	string tmp_flist=get_tmp_file();
	string tmp_xml_flist=get_tmp_file();
	FILES file_tmp;
	CreateFlistNode(tmp_flist, tmp_xml_flist);
	PackageConfig ftree(tmp_xml_flist);
	vector <string> vec_tmp_names=ftree.getFilelist();
	for (int i=2;i<vec_tmp_names.size();i++)
	{
		file_tmp.set_name(vec_tmp_names[i]);
		file_tmp.set_size("0"); // I think store this data about each file is idiotizm
		data.get_files()->add(file_tmp);
	}
	vec_tmp_names.clear();
	debug("get_filelist end");
	return 0;
}
	
int LocalPackage::create_md5()
{
	debug("create_md5 start");
	string tmp_md5=get_tmp_file();

	string sys="md5sum "+filename+" > "+tmp_md5;
	system(sys.c_str());
	FILE* md5=fopen(tmp_md5.c_str(), "r");
	if (!md5)
	{
		fprintf(stderr, "Unable to open md5 temp file\n");
		return 1;
	}
	char _c_md5[1000];
	fscanf(md5, "%s", &_c_md5);
	string md5str;
	md5str=_c_md5;
	//free(_c_md5);
	fclose(md5);
	data.set_md5(md5str);
	debug("create_md5 end");
	return 0;
}

int LocalPackage::get_size()
{
	debug("get_size start");
	string tmp=get_tmp_file();
	string sys="gzip -l "+filename+" > "+tmp;
	system(sys.c_str());
	FILE *zdata=fopen(tmp.c_str(), "r");
	if (!zdata) return 1;
	char c_size[40000];
	char i_size[40000];
	debug("reading file...");

	for (int i=1; i<=5; i++)
	{
		if (fscanf(zdata, "%s", &c_size)==EOF)
		{
			printf("Unexcepted EOF while reading gzip data\n");
			return 1;
		}
	}
	fscanf(zdata, "%s", &i_size);
	fclose(zdata);
	string csize;
	csize=c_size;
	string isize;
	isize=i_size;
	data.set_compressed_size(csize);
	data.set_installed_size(isize);
	//free(c_size);
	//free(i_size);
	debug("get_size end");
	return 0;
}
	
int LocalPackage::set_additional_data()
{
	LOCATION location;
	location.set_local();
	string pwd_t=get_tmp_file();
	string pwd="pwd > "+pwd_t;
	system(pwd.c_str());
	FILE* _pt=fopen(pwd_t.c_str(), "r");
	char fstr[40000];
	fscanf(_pt, "%s", &fstr);
	fclose(_pt);
	string ffname;
	debug("Local location: "+ffname);
	ffname=fstr;
	ffname+="/";
	location.set_path(ffname);
	data.get_locations()->add(location);
	return 0;
}

int LocalPackage::injectFile()
{
	// Injecting data from file!
	debug("injectFile start");
	get_size();
	create_md5();
	printf("filename is %s\n", filename.c_str());
	data.set_filename(filename);
	get_xml();
	get_filelist();
	set_additional_data();
	debug("injectFile end");
	return 0;
}

int LocalPackage::CreateFlistNode(string fname, string tmp_xml)
{
	debug("CreateFlistNode start");
	string tar_cmd;
	debug("flist tmpfile: "+fname);
	tar_cmd="tar ztf "+filename+" > "+fname;
	system(tar_cmd.c_str());
	string sed_cmd;
	sed_cmd="echo '<?xml version=\"1.0\" encoding=\"utf-8\"?><package><filelist><file>file_list_header' > "+tmp_xml+" && cat "+ fname +" | sed -e i'</file>\\n<file>'  >> "+tmp_xml+" && echo '</file></filelist></package>' >> "+tmp_xml;
	system(sed_cmd.c_str());
	debug("CreateFlistNode end");
	return 0;
}

