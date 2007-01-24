/******************************************************************
 * Repository class: build index, get index...etc.
 * $Id: repository.cpp,v 1.6 2007/01/24 15:16:26 i27249 Exp $
 * ****************************************************************/
#include "repository.h"
#include <iostream>
Repository::Repository(){}
Repository::~Repository(){}

XMLNode _root;

int xml2package(XMLNode pkgnode, PACKAGE *data)
{
	PackageConfig p(pkgnode);
	
	data->set_name(p.getName());
	data->set_version(p.getVersion());
	data->set_arch(p.getArch());
	data->set_build(p.getBuild());
	data->set_packager(p.getAuthorName());
	data->set_packager_email(p.getAuthorEmail());
	data->set_description(p.getDescription());
	data->set_short_description(p.getShortDescription());
	data->set_changelog(p.getChangelog());

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
		data->get_dependencies()->add(dep_tmp);
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
		data->get_dependencies()->add(suggest_tmp);
		suggest_tmp.clear();
	}

	vec_tmp_names=p.getTags();
	for (unsigned int i=0;i<vec_tmp_names.size();i++)
	{
		tag_tmp.set_name(vec_tmp_names[i]);
		data->get_tags()->add(tag_tmp);
		tag_tmp.clear();
	}

	vec_tmp_names.clear();
	vec_tmp_conditions.clear();
	vec_tmp_versions.clear();

	LOCATION tmp_location;
	tmp_location.set_path(p.getLocation());
	data->get_locations()->add(tmp_location);
	data->set_filename(p.getFilename());
	data->set_md5(p.getMd5());
	data->set_compressed_size(p.getCompressedSize());
	data->set_installed_size(p.getInstalledSize());
	
	vec_tmp_names=p.getFilelist();
	FILES file_tmp;
	for (unsigned int i=0;i<vec_tmp_names.size();i++)
	{
		file_tmp.set_name(vec_tmp_names[i]);
		data->get_files()->add(file_tmp);
		//file_tmp.clear();
	}



	return 0;
}


int ProcessPackage(const char *filename, const struct stat *file_status, int filetype)
{
#ifdef DEBUG
	printf("repository.cpp: processing package %s\n", filename);
#endif
	string _package=filename;
       	string ext;
	for (unsigned int i=_package.length()-4;i<_package.length();i++)
	{
		ext+=_package[i];
	}

	if (filetype==FTW_F && ext==".tgz")
	{
		cout<< "indexing file " << filename << "..."<<endl;
//		printf("indexing file %s...\t", filename);
		LocalPackage lp(_package);
		lp.injectFile(true);
		_root.addChild(lp.getPackageXMLNode());
		printf("done\n");
	}
	return 0;
}


int Repository::build_index(string server_url)
{
	// First of all, initialise main XML tree. Due to some code restrictions, we use global variable _root.
	_root=XMLNode::createXMLTopNode("repository");
	_root.addAttribute("server_url", server_url.c_str());
	
	// Next, run thru files and extract data.
	// We consider that repository root is current directory. So, what we need to do:
	// Enter each sub-dir, get each file which name ends with .tgz, extracts xml (and other) data from them, 
	// and build an XML tree for whole repository, then write it to ./packages.xml
	
	ftw(".", ProcessPackage, 100);

	// Finally, write our XML tree to file
	_root.writeToFile("packages.xml");
	// Compress file
	if (system("gzip -f packages.xml")==0) printf("Repository index created successfully\n");
	return 0;
}

PACKAGE_LIST Repository::get_index(string server_url)
{
	PACKAGE_LIST packages;
	PACKAGE pkg;
	string wget_line;
	string gzip_line;
	string xml_name=get_tmp_file();
	string gzxml_name=xml_name+".gz";
	XMLNode repository_root;
	wget_line="wget -q --output-document="+gzxml_name+" "+server_url+"packages.xml.gz";
	gzip_line="gunzip -f "+gzxml_name;
	printf("[%s]...",server_url.c_str());
	if (system(wget_line.c_str())==0 && system(gzip_line.c_str())==0)
	{
		
		printf("done\n");
		repository_root=XMLNode::openFileHelper(xml_name.c_str(), "repository");
		int pkg_count=repository_root.nChildNode("package");
		if (pkg_count==0)
		{
			printf(_("Repository has no packages, aborting"));
			return packages;
		}
		for (int i=0; i<pkg_count; i++)
		{
			pkg.clear();
			xml2package(repository_root.getChildNode("package", i), &pkg);
			packages.add(pkg);
		}

	}
	else
	{
		printf(_("Download error, check connection and URL"));
	}
	return packages;
}
