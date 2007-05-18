/*
Local package installation functions

$Id: local_package.cpp,v 1.44 2007/05/18 12:04:53 i27249 Exp $
*/

#include "local_package.h"
#include "mpkg.h"
//#include "oldstyle.h"

 

int slack2xml(string filename, string xml_output)
{
	string slackDescFile = get_tmp_file();
	string slackRequiredFile = get_tmp_file();
	string slackSuggestsFile = get_tmp_file();
	extractFromTgz(filename, "install/slack-desc", slackDescFile);
	extractFromTgz(filename, "install/slack-required", slackRequiredFile);
	extractFromTgz(filename, "install/slack-suggest", slackSuggestsFile);
	XMLNode _node = XMLNode::createXMLTopNode("package");
	_node.addChild("name");
	string pkgName;
	_node.addChild("version");
	_node.addChild("arch");
	_node.addChild("build");
	_node.addChild("short_description");
	_node.addChild("description");
	_node.addChild("dependencies");
	_node.addChild("suggests");
	_node.addChild("filename");
	int pos;
	int name_start=0;
	string tmp;
	for (int i=filename.length()-1; filename[i]!='/' && i>=0; i--)
	{
		name_start=i;
	}
	for (unsigned int i=name_start; i<filename.length()-1; i++)
	{
		if (filename[i]=='-')
		{
			if (filename[i+1]=='0' || \
				filename[i+1] == '1' || \
				filename[i+1] == '2' || \
				filename[i+1] == '3' || \
				filename[i+1] == '4' || \
				filename[i+1] == '5' || \
				filename[i+1] == '6' || \
				filename[i+1] == '7' || \
				filename[i+1] == '8' || \
				filename[i+1] == '9')
			{
				pkgName = tmp;
				_node.getChildNode("name").addText(tmp.c_str());
				pos=i+2;
				break;
			}
		}
		tmp+=filename[i];
	}
	tmp.clear();
	//VERSION
	for (unsigned int i=pos-1; i< filename.length(); i++)
	{
		if (filename[i]=='-')
		{
			_node.getChildNode("version").addText(tmp.c_str());
			pos=i+2;
			break;
		}
		tmp+=filename[i];
	}
	tmp.clear();
	//ARCH
	for (unsigned int i=pos-1; i< filename.length(); i++)
	{
		if (filename[i]=='-')
		{
			_node.getChildNode("arch").addText(tmp.c_str());
			pos=i+2;
			break;
		}
		tmp+=filename[i];
	}
	tmp.clear();
	//BUILD
	for (unsigned int i=pos-1; i<filename.length()-4; i++)
	{
		tmp+=filename[i];
	}
	_node.getChildNode("build").addText(tmp.c_str());
	tmp.clear();
	_node.writeToFile(xml_output.c_str());
	return 0;
}



LocalPackage::LocalPackage(string _f, unsigned int pkgType)
{
	mDebug("LocalPackage created");
	this->filename=_f;
	this->packageType=pkgType;
}

LocalPackage::~LocalPackage(){}

string LocalPackage::files2xml(string input)
{
	mstring output;
	output="<?xml version=\"1.0\" encoding=\"utf-8\"?><package><filelist><file>";
	for (int i=0; i<input.length(); i++)
	{
		if (input[i]=='\n') 
		{
			output+="</file>\n";
			if (i<input.length()-1) output+="<file>";
		}
		else output+=input[i];
	}
	output+="</filelist></package>";
	return output.s_str();
}

int LocalPackage::fill_scripts(PACKAGE *package)
{
	mDebug("get_scripts start");
	string scripts_dir=SCRIPTS_DIR+"/" + *package->get_filename() + "_" + *package->get_md5() + "/";
	string tmp_preinstall=scripts_dir+"preinstall.sh";
	string tmp_postinstall=scripts_dir+"doinst.sh";
	string tmp_preremove=scripts_dir+"preremove.sh";
	string tmp_postremove=scripts_dir+"postremove.sh";
	string mkdir_pkg="mkdir -p "+scripts_dir+" 2>/dev/null";
	system(mkdir_pkg.c_str());
	string sys_cache=SYS_CACHE;
	string filename=sys_cache + *package->get_filename();
	string sys_preinstall = "tar zxf "+filename+" install/preinstall.sh --to-stdout > "+ tmp_preinstall + " 2>/dev/null";
	string sys_postinstall ="tar zxf "+filename+" install/doinst.sh --to-stdout > "+ tmp_postinstall + " 2>/dev/null";
	string sys_preremove =  "tar zxf "+filename+" install/preremove.sh --to-stdout > "+ tmp_preremove + " 2>/dev/null";
	string sys_postremove = "tar zxf "+filename+" install/postremove.sh --to-stdout > "+ tmp_postremove + " 2>/dev/null";

	system(sys_preinstall.c_str());
	system(sys_postinstall.c_str());
	system(sys_preremove.c_str());
	system(sys_postremove.c_str());

	return 0;
}



int LocalPackage::get_scripts()
{
	mDebug("get_scripts start");
	
	string tmp_preinstall=get_tmp_file();
	string tmp_postinstall=get_tmp_file();
	string tmp_preremove=get_tmp_file();
	string tmp_postremove=get_tmp_file();
	
	string sys_preinstall = "tar zxf "+filename+" install/preinstall.sh --to-stdout > "+tmp_preinstall+" 2>/dev/null";
	string sys_postinstall ="tar zxf "+filename+" install/doinst.sh --to-stdout > "+tmp_postinstall+" 2>/dev/null";
	string sys_preremove =  "tar zxf "+filename+" install/preremove.sh --to-stdout > "+tmp_preremove+" 2>/dev/null";
	string sys_postremove = "tar zxf "+filename+" install/postremove.sh --to-stdout > "+tmp_postremove+" 2>/dev/null";

	system(sys_preinstall.c_str());
	system(sys_postinstall.c_str());
	system(sys_preremove.c_str());
	system(sys_postremove.c_str());

	string str_preinstall=ReadFile(tmp_preinstall);
	string str_postinstall=ReadFile(tmp_postinstall);
	string str_preremove=ReadFile(tmp_preremove);
	string str_postremove=ReadFile(tmp_postremove);

	return 0;
}

int LocalPackage::get_xml()
{
	mDebug("get_xml start");
	string tmp_xml=get_tmp_file();
	string sys="tar zxf "+filename+" install/data.xml --to-stdout > "+tmp_xml+" 2>/dev/null";
	system(sys.c_str());
	
	if (!FileNotEmpty(tmp_xml))
	{
		// In most cases it means that we have legacy Slackware package.
		// Trying to convert:
		if (slack2xml(filename, tmp_xml) != 0)
		{
			mError("Infernally invalid package! Cannot work with it at all");
			return -1;
		}
	}

	PackageConfig p(tmp_xml);
	if (!p.parseOk)
	{
		return -100;
	}
	_packageXMLNode = p.getXMLNode(); // To be indexing work

	*data.get_name()=p.getName();
	*data.get_version()=p.getVersion();
	*data.get_arch()=p.getArch();
	*data.get_build()=p.getBuild();
	*data.get_packager()=p.getAuthorName();
	*data.get_packager_email()=p.getAuthorEmail();
	*data.get_description()=p.getDescription();
	*data.get_short_description()=p.getShortDescription();
	*data.get_changelog()=p.getChangelog();
	
#ifdef ENABLE_INTERNATIONAL
	// This line enables international descriptions
	data.get_descriptions(&p.getDescriptions());
#endif	
	DEPENDENCY dep_tmp;
	DEPENDENCY suggest_tmp;

	vector<string> vec_tmp_names;
	vector<string> vec_tmp_conditions;
	vector<string> vec_tmp_versions;

	vec_tmp_names=p.getDepNames();
	vec_tmp_conditions=p.getDepConditions();
	vec_tmp_versions=p.getDepVersions();

	for (unsigned int i=0;i<vec_tmp_names.size();i++)
	{
		dep_tmp.set_package_name(&vec_tmp_names[i]);
		dep_tmp.set_package_version(&vec_tmp_versions[i]);
		*dep_tmp.get_condition()=IntToStr(condition2int(vec_tmp_conditions[i]));
		*dep_tmp.get_type()==(string) "DEPENDENCY";
		data.get_dependencies()->push_back(dep_tmp);
		dep_tmp.clear();
	}
	vec_tmp_names=p.getSuggestNames();
	vec_tmp_conditions=p.getSuggestConditions();
	vec_tmp_versions=p.getSuggestVersions();

	for (unsigned int i=0;i<vec_tmp_names.size();i++)
	{
		suggest_tmp.set_package_name(&vec_tmp_names[i]);
		suggest_tmp.set_package_version(&vec_tmp_versions[i]);
		*suggest_tmp.get_condition()=IntToStr(condition2int(vec_tmp_conditions[i]));
		*suggest_tmp.get_type()=(string) "SUGGEST";
		data.get_dependencies()->push_back(suggest_tmp);
		suggest_tmp.clear();
	}

	vec_tmp_names=p.getTags();
	for (unsigned int i=0;i<vec_tmp_names.size();i++)
	{
		data.get_tags()->push_back(vec_tmp_names[i]);
	}

	vec_tmp_names=p.getConfigFilelist();
	FILES configfile_tmp;
	for (unsigned int i=0; i<vec_tmp_names.size(); i++)
	{
		configfile_tmp.set_name(&vec_tmp_names[i]);
		configfile_tmp.set_type(FTYPE_CONFIG);
		data.get_config_files()->push_back(configfile_tmp);
	}
	vec_tmp_names.clear();
	vec_tmp_conditions.clear();
	vec_tmp_versions.clear();
	mDebug("get_xml end");
	return 0;
}

int LocalPackage::fill_filelist(PACKAGE *package)
{
	mDebug("fill_filelist start");
	FILES file_tmp;
	// Retrieving regular files
	string fname=get_tmp_file();
	string tar_cmd="tar ztf "+filename+" --exclude install " +" > "+fname;
	system(tar_cmd.c_str());
	vector<string>file_names=ReadFileStrings(fname);
	for (unsigned int i=2; i<file_names.size(); i++)
	{
		file_tmp.set_name(&file_names[i]);
		package->get_files()->push_back(file_tmp);
	}
	// Retrieving symlinks (from doinst.sh)
	string lnfname=get_tmp_file();
	string sed_cmd = "sed -n 's,^( *cd \\([^ ;][^ ;]*\\) *; *rm -rf \\([^ )][^ )]*\\) *) *$,\\1/\\2,p' < " + \
			  package->get_scriptdir() + "doinst.sh > "+lnfname;
	system(sed_cmd.c_str());
	vector<string>link_names=ReadFileStrings(lnfname);
	for (unsigned int i=0; i<link_names.size();i++)
	{
		file_tmp.set_name(&link_names[i]);
		package->get_files()->push_back(file_tmp);
	}
	return 0;
}


int LocalPackage::get_filelist()
{
	mDebug("get_filelist start");
	string tmp_flist=get_tmp_file();
	string tmp_xml_flist=get_tmp_file();
	FILES file_tmp;
	CreateFlistNode(tmp_flist, tmp_xml_flist);
	PackageConfig ftree(tmp_xml_flist);
	if (!ftree.parseOk) return -100;
	vector <string> vec_tmp_names=ftree.getFilelist();
	_packageXMLNode.addChild("filelist");
	for (unsigned int i=2;i<vec_tmp_names.size();i++)
	{
		_packageXMLNode.getChildNode("filelist").addChild("file");
		_packageXMLNode.getChildNode("filelist").getChildNode("file",i-2).addText(vec_tmp_names[i].c_str());
		file_tmp.set_name(&vec_tmp_names[i]);
		data.get_files()->push_back(file_tmp);
	}
	vec_tmp_names.clear();
	data.sync();
	mDebug("get_filelist end");
	return 0;
}
	
int LocalPackage::create_md5()
{
	mDebug("create_md5 start");
	string tmp_md5=get_tmp_file();

	string sys="md5sum "+filename+" > "+tmp_md5;
	system(sys.c_str());
	string md5str=ReadFile(tmp_md5, 32);
	mDebug("MD5 = "+md5str);
	if (md5str.empty())
	{
		mError("Unable to read md5 temp file");
		return 1;
	}
	data.set_md5(&md5str);
	_packageXMLNode.addChild("md5");
	_packageXMLNode.getChildNode("md5").addText(md5str.c_str());
	mDebug("create_md5 end");
	return 0;
}

int LocalPackage::get_size()
{
	mDebug("get_size start");
	string tmp=get_tmp_file();
	string sys="gzip -l "+filename+" > "+tmp + " 2>/dev/null";
	if (system(sys.c_str())!=0)
	{
		mError("Zero-length package " + filename);
		return -2;
	}
	FILE *zdata=fopen(tmp.c_str(), "r");
	if (!zdata)
	{
		mError("Unable to extract size of package");
		return -1;
	}
	char c_size[40000]; //FIXME: Overflow are possible here
	char i_size[40000]; //FIXME: Same problem
	mDebug("reading file...");

	for (int i=1; i<=5; i++)
	{
		if (fscanf(zdata, "%s", &c_size)==EOF)
		{
			mError("Unexcepted EOF while reading gzip data");
			return -1;
		}
	}
	fscanf(zdata, "%s", &i_size);
	fclose(zdata);
	string csize;
	csize=c_size;
	string isize;
	isize=i_size;
	data.set_compressed_size(&csize);
	data.set_installed_size(&isize);
	_packageXMLNode.addChild("compressed_size");
	_packageXMLNode.getChildNode("compressed_size").addText(csize.c_str());
	_packageXMLNode.addChild("installed_size");
	_packageXMLNode.getChildNode("installed_size").addText(isize.c_str());
	mDebug("get_size end");
	return 0;
}
	
int LocalPackage::set_additional_data()
{
	LOCATION location;
	location.set_local();
	char pwd[MAXPATHLEN];
	getcwd(pwd, MAXPATHLEN);
	string fpath;
	string fname;
	int fname_start=0;
	// OMG@@@!!!
	for(int i=data.get_filename()->length()-1;i>=0 && data.get_filename()->at(i)!='/'; i--)
	{
		fname_start=i;
	}
	if (fname_start!=1)
	{

		for (int i=0;i<fname_start;i++)
		{
			fpath+=data.get_filename()->at(i);
		}
		for (unsigned int i=fname_start;i<data.get_filename()->length();i++)
		{
			fname+=data.get_filename()->at(i);
		}
	}
	mDebug("filename: "+fname);
	string ffname;
	if (fpath[0]!='/')
	{
		ffname=pwd;
		ffname+="/";
		ffname+=fpath;
	}
	else ffname=fpath;
	mDebug("file path: "+ffname);
	data.set_filename(&fname);
	*location.get_server_url()=(string) "local://";
	location.set_path(&ffname);
	data.get_locations()->push_back(location);
	_packageXMLNode.addChild("filename");
	_packageXMLNode.getChildNode("filename").addText(data.get_filename()->c_str());
	_packageXMLNode.addChild("location");
	_packageXMLNode.getChildNode("location").addText(fpath.c_str());


	return 0;
}

int LocalPackage::fill_configfiles(PACKAGE *package)
{
	string tmp_xml=get_tmp_file();
	string sys="tar zxf "+filename+" install/data.xml --to-stdout > "+tmp_xml+" 2>/dev/null";
	if (system(sys.c_str())!=0)
	{
		return 0;
	}
	
	// Checking for XML presence. NOTE: this procedure DOES NOT check validity of this XML!
	if (!FileNotEmpty(tmp_xml))
	{
		return -1;
	}

	PackageConfig p(tmp_xml);
	if (!p.parseOk) return -100;
	vector<string> vec_tmp_names=p.getConfigFilelist();
	FILES configfile_tmp;
	for (unsigned int i=0; i<vec_tmp_names.size(); i++)
	{
		configfile_tmp.set_name(&vec_tmp_names[i]);
		configfile_tmp.set_type(FTYPE_CONFIG);
		package->get_config_files()->push_back(configfile_tmp);
	}
	vec_tmp_names.clear();
	package->sync();
	mDebug("PMFG...fill_configfiles end");
	return 0;

}
int LocalPackage::injectFile(bool index)
{
	// Injecting data from file!
	// If any of functions fails (e.g. return!=0) - break process and return failure code (!=0);
	//int ret=0;
	mDebug("local_package.cpp: injectFile(): start");
	mDebug("get_xml");
	if (get_xml()!=0)
	{
		mDebug("local_package.cpp: injectFile(): get_xml FAILED");
		return -3;
	}
	mDebug("get_size()\n");
	if (get_size()!=0)
	{
		mDebug("local_package.cpp: injectFile(): get_size() FAILED");
		return -1;
	}
	mDebug("create_md5\n");
	if (create_md5()!=0)
	{
		mDebug("local_package.cpp: injectFile(): create_md5 FAILED");
		return -2;
	}
	mDebug("set_additional_data\n");
	mDebug("local_packaige.cpp: injectFile(): filename is "+ filename);
	data.set_filename(&filename);
	
	if (!index)
	{
		if (get_scripts()!=0)
		{
			mDebug("local_package.cpp: injectFile(): get_scripts FAILED");
			return -4;
		}
	}
	if (!index) // NOT Building file list on server, build locally
	{
		if (get_filelist()!=0)
		{
			mDebug("local_package.cpp: injectFile(): get_filelist FAILED");
			return -5;
		}
	}
	if (set_additional_data()!=0)
	{
		mDebug("local_package.cpp: injectFile(): set_additional_data FAILED");
		return -6;
	}
	delete_tmp_files();	
	mDebug("local_package.cpp: injectFile(): end");
	return 0;
}

int LocalPackage::CreateFlistNode(string fname, string tmp_xml)
{
	mDebug("local_package.cpp: CreateFlistNode(): begin");
	string tar_cmd;
	mDebug("flist tmpfile: "+fname);
	tar_cmd="tar ztf "+filename+" --exclude install " +" > "+fname;
	if (system(tar_cmd.c_str())!=0)
	{
		mError("Unable to get file list");
		return -1;
	}
#ifdef USE_INTERNAL_SED
	WriteFile(tmp_xml, files2xml(ReadFile(fname)));
#else
	string sed_cmd;
	sed_cmd="echo '<?xml version=\"1.0\" encoding=\"utf-8\"?><package><filelist><file>file_list_header' > "+tmp_xml+" && cat "+ fname +" | sed -e i'</file>\\n<file>'  >> "+tmp_xml+" && echo '</file></filelist></package>' >> "+tmp_xml;
	if (system(sed_cmd.c_str())!=0)
	{
		mError("Parsing using sed failed");
		return -1;
	}
#endif
	mDebug("local_package.cpp: CreateFlistNode(): end");
	return 0;
}

XMLNode LocalPackage::getPackageXMLNode()
{
	return _packageXMLNode;
}

