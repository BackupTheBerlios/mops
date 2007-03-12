/*
Local package installation functions

$Id: local_package.cpp,v 1.27 2007/03/12 00:39:44 i27249 Exp $
*/

#include "local_package.h"
#include "mpkg.h"
//#include "oldstyle.h"

int extractFromTgz(string filename, string file_to_extract, string output)
{
	string cmd = "tar zxf "+filename+" "+ file_to_extract + " --to-stdout > " + output + " 2>/dev/null";
	return system(cmd.c_str());
}
 

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

	// DEPENDENCIES
	// Dependencies
	int dep_num=0;
	string tmpDepStr;
	string slackRequired = ReadFile(slackRequiredFile);
	while (slackRequired.find_first_of(",")!=std::string::npos)
	{
		debug("Proceeding dep, slackRequired = "+ slackRequired);
		pos = slackRequired.find_first_of("=><");
		if (pos < slackRequired.find_first_of(",\n"))
		{
			_node.getChildNode("dependencies").addChild("dep");
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).addChild("name");
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).getChildNode("name").addText(slackRequired.substr(0, pos).c_str());
			slackRequired = slackRequired.substr(pos);
			pos = slackRequired.find_first_not_of("=><");
			tmpDepStr = slackRequired.substr(0,pos);
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).addChild("condition");
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).getChildNode("condition").addText(hcondition2xml(tmpDepStr).c_str());
			slackRequired = slackRequired.substr(pos);
			pos = slackRequired.find_first_of(" ,");
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).addChild("version");
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).getChildNode("version").addText(slackRequired.substr(0,pos).c_str());
		}
		else
		{
			pos = slackRequired.find_first_of(",")+1;
			_node.getChildNode("dependencies").addChild("dep");
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).addChild("name");
			_node.getChildNode("dependencies").getChildNode("dep",dep_num).getChildNode("name").addText(slackRequired.substr(0,pos).c_str());
			slackRequired = slackRequired.substr(pos);
		}
		dep_num++;
	}

	debug("reached suggestions");
	// Suggestions
	dep_num = 0;
	string slackSuggests = ReadFile(slackSuggestsFile);
	while (slackSuggests.find_first_of(",")!=std::string::npos)
	{
		debug("Proceeding dep, slackSuggests = "+ slackSuggests);
		pos = slackSuggests.find_first_of("=><");
		if (pos < slackSuggests.find_first_of(",\n"))
		{
			_node.getChildNode("suggests").addChild("suggest");
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).addChild("name");
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).getChildNode("name").addText(slackSuggests.substr(0, pos).c_str());
			slackSuggests = slackSuggests.substr(pos);
			pos = slackSuggests.find_first_not_of("=><");
			tmpDepStr = slackSuggests.substr(0,pos);
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).addChild("condition");
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).getChildNode("condition").addText(hcondition2xml(tmpDepStr).c_str());
			slackSuggests = slackSuggests.substr(pos);
			pos = slackSuggests.find_first_of(" ,");
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).addChild("version");
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).getChildNode("version").addText(slackSuggests.substr(0,pos).c_str());
		}
		else
		{
			pos = slackSuggests.find_first_of(",")+1;
			_node.getChildNode("suggests").addChild("suggest");
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).addChild("name");
			_node.getChildNode("suggests").getChildNode("suggest",dep_num).getChildNode("name").addText(slackSuggests.substr(0,pos).c_str());
			slackSuggests = slackSuggests.substr(pos);
		}
		dep_num++;
	}

	debug("reached description");
	// Description
	string tmpDescStr;
	string slackDescription = ReadFile(slackDescFile);
	// Stripping trash =)
	int cut=0;
	for (int i=0; cut != std::string::npos; i++)
	{
		if (i==0) cut = slackDescription.find("#");
		else cut = slackDescription.find("\n#");
		if (cut!=std::string::npos)
		{
			slackDescription.substr(slackDescription.find_first_of("\n"));
		}
	}
	cut = slackDescription.find("--|");
	if (cut != std::string::npos) slackDescription = slackDescription.substr(cut+3);

	if (slackDescription.length()>0)
	{
	slackDescription = slackDescription.substr(1);
		if (slackDescription.length()>=slackDescription.find(pkgName+": ")+pkgName.length()+2)
		{
			slackDescription = slackDescription.substr(slackDescription.find(pkgName+": ")+pkgName.length()+2);
			_node.getChildNode("short_description").addText(slackDescription.substr(0, slackDescription.find_first_of("\n")).c_str());
			debug("short description: "+(string)_node.getChildNode("short_description").getText());
		}
		pos = slackDescription.find("\n");
		int lines = 0;
		while (pos != std::string::npos && lines < 11)
		{
			pos = slackDescription.find(pkgName+": ");
			if (pos == std::string::npos)
			{
				debug("Description end");
			}
			else
			{
				slackDescription = slackDescription.substr(pos+pkgName.length()+2);
				tmpDescStr+=slackDescription.substr(0,slackDescription.find("\n"))+"\n";
				lines++;
			}
		}
		debug("Description: "+ tmpDescStr);
		_node.getChildNode("description").addText(tmpDescStr.c_str());
	}
	_node.writeToFile(xml_output.c_str());
	return 0;
}



LocalPackage::LocalPackage(string _f, unsigned int pkgType)
{
	debug("LocalPackage created");
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
	debug("get_scripts start");
	
	string scripts_dir=SCRIPTS_DIR+"/" + package->get_filename() + "_" + package->get_md5();
	string tmp_preinstall=scripts_dir+"/preinstall.sh";
	string tmp_postinstall=scripts_dir+"/doinst.sh";
	string tmp_preremove=scripts_dir+"/preremove.sh";
	string tmp_postremove=scripts_dir+"/postremove.sh";
	string mkdir_pkg="mkdir "+scripts_dir+" 2>/dev/null";
	system(mkdir_pkg.c_str());
	string sys_cache=SYS_CACHE;
	string filename=sys_cache+package->get_filename();
#ifdef DEBUG
	printf("extracting scripts for %s, filename: %s\n", package->get_name().c_str(), filename.c_str());
#endif
	string sys_preinstall = "tar zxf "+filename+" install/preinstall.sh --to-stdout > "+tmp_preinstall+" 2>/dev/null";
	string sys_postinstall ="tar zxf "+filename+" install/doinst.sh --to-stdout > "+tmp_postinstall+" 2>/dev/null";
	string sys_preremove =  "tar zxf "+filename+" install/preremove.sh --to-stdout > "+tmp_preremove+" 2>/dev/null";
	string sys_postremove = "tar zxf "+filename+" install/postremove.sh --to-stdout > "+tmp_postremove+" 2>/dev/null";

	system(sys_preinstall.c_str());
	system(sys_postinstall.c_str());
	system(sys_preremove.c_str());
	system(sys_postremove.c_str());

	package->get_scripts()->set_preinstall(tmp_preinstall);
	package->get_scripts()->set_postinstall(tmp_postinstall);
	package->get_scripts()->set_preremove(tmp_preremove);
	package->get_scripts()->set_postremove(tmp_postremove);
	return 0;
}



int LocalPackage::get_scripts()
{
	debug("get_scripts start");
	
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

	if (!str_preinstall.empty()) data.get_scripts()->set_preinstall(str_preinstall);
	if (!str_postinstall.empty()) data.get_scripts()->set_postinstall(str_postinstall);
	if (!str_preremove.empty()) data.get_scripts()->set_preremove(str_preremove);
	if (!str_postremove.empty()) data.get_scripts()->set_postremove(str_postremove);
	return 0;
}

int LocalPackage::get_xml()
{
	printf("get_xml\n");
	debug("get_xml start");
	string tmp_xml=get_tmp_file();
	string sys="tar zxf "+filename+" install/data.xml --to-stdout > "+tmp_xml+" 2>/dev/null";
	system(sys.c_str());
	
	// Checking for XML presence. NOTE: this procedure DOES NOT check validity of this XML!
	if (!FileNotEmpty(tmp_xml))
	{
		// In most cases it means that we have legacy Slackware package.
		// Trying to convert:
		if (slack2xml(filename, tmp_xml) != 0)
		{
			fprintf(stderr, "Totally invalid package! Cannot work with it\n");
			return -1;
		}
	}

	PackageConfig p(tmp_xml);
	_packageXMLNode = p.getXMLNode(); // To be indexing work

	data.set_name(p.getName());
	data.set_version(p.getVersion());
	data.set_arch(p.getArch());
	data.set_build(p.getBuild());
	data.set_packager(p.getAuthorName());
	data.set_packager_email(p.getAuthorEmail());
	data.set_description(p.getDescription());
	data.set_short_description(p.getShortDescription());
	data.set_changelog(p.getChangelog());
	
	// This line enables international descriptions
	data.set_descriptions(p.getDescriptions());
	
	DEPENDENCY dep_tmp;
	DEPENDENCY suggest_tmp;
	TAG tag_tmp;

	vector<string> vec_tmp_names;
	vector<string> vec_tmp_conditions;
	vector<string> vec_tmp_versions;

	vec_tmp_names=p.getDepNames();
	vec_tmp_conditions=p.getDepConditions();
	vec_tmp_versions=p.getDepVersions();

	for (unsigned int i=0;i<vec_tmp_names.size();i++)
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

	for (unsigned int i=0;i<vec_tmp_names.size();i++)
	{
		suggest_tmp.set_package_name(vec_tmp_names[i]);
		suggest_tmp.set_package_version(vec_tmp_versions[i]);
		suggest_tmp.set_condition(IntToStr(condition2int(vec_tmp_conditions[i])));
		suggest_tmp.set_type("SUGGEST");
		data.get_dependencies()->add(suggest_tmp);
		suggest_tmp.clear();
	}

	vec_tmp_names=p.getTags();
	for (unsigned int i=0;i<vec_tmp_names.size();i++)
	{
		tag_tmp.set_name(vec_tmp_names[i]);
		data.get_tags()->add(tag_tmp);
		tag_tmp.clear();
	}

	vec_tmp_names=p.getConfigFilelist();
	FILES configfile_tmp;
	for (unsigned int i=0; i<vec_tmp_names.size(); i++)
	{
		configfile_tmp.set_name(vec_tmp_names[i]);
		data.get_config_files()->add(configfile_tmp);
	}
	vec_tmp_names.clear();
	vec_tmp_conditions.clear();
	vec_tmp_versions.clear();
	debug("get_xml end");
	return 0;
}

int LocalPackage::fill_filelist(PACKAGE *package)
{
	debug("fill_filelist start");
	string tmp_flist=get_tmp_file();
	string tmp_xml_flist=get_tmp_file();
	FILES file_tmp;
	CreateFlistNode(tmp_flist, tmp_xml_flist);
	PackageConfig ftree(tmp_xml_flist);
	vector <string> vec_tmp_names=ftree.getFilelist();
	for (unsigned int i=2;i<vec_tmp_names.size();i++)
	{
		file_tmp.set_name(vec_tmp_names[i]);
		package->get_files()->add(file_tmp);
	}
	vec_tmp_names.clear();
	debug("fill_filelist end");
	package->sync();
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
	_packageXMLNode.addChild("filelist");
	for (unsigned int i=2;i<vec_tmp_names.size();i++)
	{
		_packageXMLNode.getChildNode("filelist").addChild("file");
		_packageXMLNode.getChildNode("filelist").getChildNode("file",i-2).addText(vec_tmp_names[i].c_str());
		file_tmp.set_name(vec_tmp_names[i]);
		data.get_files()->add(file_tmp);
	}
	vec_tmp_names.clear();
	printf("get_filelist\n");
	data.sync();
	debug("get_filelist end");
	return 0;
}
	
int LocalPackage::create_md5()
{
	debug("create_md5 start");
	string tmp_md5=get_tmp_file();

	string sys="md5sum "+filename+" > "+tmp_md5;
	system(sys.c_str());
	string md5str=ReadFile(tmp_md5, 32);
	debug("MD5 = "+md5str);
	if (md5str.empty())
	{
		printf(_("ERROR: Unable to read md5 temp file\n"));
		return 1;
	}
	data.set_md5(md5str);
	_packageXMLNode.addChild("md5");
	_packageXMLNode.getChildNode("md5").addText(md5str.c_str());
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
	if (!zdata)
	{
		printf(_("Unable to extract size of package\n"));
		return 1;
	}
	char c_size[40000]; //FIXME: Overflow are possible here
	char i_size[40000]; //FIXME: Same problem
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
	_packageXMLNode.addChild("compressed_size");
	_packageXMLNode.getChildNode("compressed_size").addText(csize.c_str());
	_packageXMLNode.addChild("installed_size");
	_packageXMLNode.getChildNode("installed_size").addText(isize.c_str());
	debug("get_size end");
	return 0;
}
	
int LocalPackage::set_additional_data()
{
	LOCATION location;
	SERVER server;
	location.set_local();
	char pwd[MAXPATHLEN];
	getcwd(pwd, MAXPATHLEN);
	string fpath;
	string fname;
	int fname_start=0;
	for(int i=data.get_filename().length()-1;i>=0 && data.get_filename()[i]!='/'; i--)
	{
		fname_start=i;
	}
	if (fname_start!=1)
	{

		for (int i=0;i<fname_start;i++)
		{
			fpath+=data.get_filename()[i];
		}
		for (unsigned int i=fname_start;i<data.get_filename().length();i++)
		{
			fname+=data.get_filename()[i];
		}
	}
	debug("filename: "+fname);
	string ffname;
	if (fpath[0]!='/')
	{
		ffname=pwd;
		ffname+="/";
		ffname+=fpath;
	}
	else ffname=fpath;
	debug("file path: "+ffname);
	data.set_filename(fname);
	server.set_url("file://");
	location.set_server(server);
	location.set_path(ffname);
	data.get_locations()->add(location);
	_packageXMLNode.addChild("filename");
	_packageXMLNode.getChildNode("filename").addText(data.get_filename().c_str());
	_packageXMLNode.addChild("location");
	_packageXMLNode.getChildNode("location").addText(fpath.c_str());


	return 0;
}

int LocalPackage::fill_configfiles(PACKAGE *package)
{
	string tmp_xml=get_tmp_file();
	string sys="tar zxf "+filename+" install/data.xml --to-stdout > "+tmp_xml+" 2>/dev/null";
	system(sys.c_str());
	
	// Checking for XML presence. NOTE: this procedure DOES NOT check validity of this XML!
	if (!FileNotEmpty(tmp_xml))
	{
		// In most cases it means that we have legacy Slackware package.
		// TODO: work with it =)
		printf("%s: Invalid package: no XML data. Legacy Slackware packages is not supported yet\n", filename.c_str());
		return -1;
//		data.set_name(filename);

		//	create_xml_data(tmp_xml); // This should create us a valid XML basing on old-style Slackware package format
	}

	PackageConfig p(tmp_xml);
//	_packageXMLNode = p.getXMLNode(); // To be indexing work

	vector<string> vec_tmp_names=p.getConfigFilelist();
	FILES configfile_tmp;
	for (unsigned int i=0; i<vec_tmp_names.size(); i++)
	{
		configfile_tmp.set_name(vec_tmp_names[i]);
		package->get_config_files()->add(configfile_tmp);
	}
	vec_tmp_names.clear();
	//vec_tmp_conditions.clear();
	//vec_tmp_versions.clear();
	package->sync();
	debug("get_xml end");
	return 0;

}
int LocalPackage::injectFile(bool index)
{
	// Injecting data from file!
	// If any of functions fails (e.g. return!=0) - break process and return failure code (!=0);
	//int ret=0;
	debug("local_package.cpp: injectFile(): start");
	if (get_xml()!=0)
	{
		debug("local_package.cpp: injectFile(): get_xml FAILED");
		return -3;
	}

	if (get_size()!=0)
	{
		debug("local_package.cpp: injectFile(): get_size() FAILED");
		return -1;
	}
	if (create_md5()!=0)
	{
		debug("local_package.cpp: injectFile(): create_md5 FAILED");
		return -2;
	}
	
	debug("local_package.cpp: injectFile(): filename is "+ filename);
	data.set_filename(filename);
	
	if (!index)
	{
		if (get_scripts()!=0)
		{
			debug("local_package.cpp: injectFile(): get_scripts FAILED");
			return -4;
		}
	}
	if (!index) // NOT Building file list on server, build locally
	{
		if (get_filelist()!=0)
		{
			debug("local_package.cpp: injectFile(): get_filelist FAILED");
			return -5;
		}
	}
	if (set_additional_data()!=0)
	{
		debug("local_package.cpp: injectFile(): set_additional_data FAILED");
		return -6;
	}
	delete_tmp_files();	
	debug("local_package.cpp: injectFile(): end");
	return 0;
}

int LocalPackage::CreateFlistNode(string fname, string tmp_xml)
{
	debug("local_package.cpp: CreateFlistNode(): begin");
	string tar_cmd;
	debug("flist tmpfile: "+fname);
	tar_cmd="tar ztf "+filename+" --exclude install " +" > "+fname;
	if (system(tar_cmd.c_str())!=0)
	{
		fprintf(stderr, "Unable to get file list\n");
		return -1;
	}
#ifdef USE_INTERNAL_SED
	WriteFile(tmp_xml, files2xml(ReadFile(fname)));
#endif	
#ifdef USE_SYSTEM_SED
	string sed_cmd;
	sed_cmd="echo '<?xml version=\"1.0\" encoding=\"utf-8\"?><package><filelist><file>file_list_header' > "+tmp_xml+" && cat "+ fname +" | sed -e i'</file>\\n<file>'  >> "+tmp_xml+" && echo '</file></filelist></package>' >> "+tmp_xml;
	if (system(sed_cmd.c_str())!=0)
	{
		fprintf(stderr,"Parsing using sed failed!\n");
		return -1;
	}
#endif
	debug("local_package.cpp: CreateFlistNode(): end");
	return 0;
}

XMLNode LocalPackage::getPackageXMLNode()
{
	return _packageXMLNode;
}

