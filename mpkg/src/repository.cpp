/******************************************************************
 * Repository class: build index, get index...etc.
 * $Id: repository.cpp,v 1.14 2007/03/11 03:22:27 i27249 Exp $
 * ****************************************************************/
#include "repository.h"
#include <iostream>
#include "DownloadManager.h"
Repository::Repository(){}
Repository::~Repository(){}

XMLNode _root;

int slackpackages2list (string packageslist, PACKAGE_LIST *pkglist)
{
	if (packageslist.length()<20)
	{
		printf("Empty package list!\n");
		return -1; // It's impossible if it has less size...
	}
	// Parsing slackware PACKAGES.TXT file
	// Keywords (in [] braces to show spaces:
	// [PACKAGE NAME:  ] 		going first or after \n\n symbols.  Until \n, contains package filename (that contains name, version, arch and build information)
	// [PACKAGE LOCATION:  ]	location of package (from repository root). Going after \n
	// [PACKAGE SIZE (compressed):  ]
	// [PACKAGE SIZE (uncompressed):  ]
	// [PACKAGE REQUIRED:  ]
	// [PACKAGE SUGGESTS:  ]
	// [PACKAGE DESCRIPTION:\n]	Package description (max 11 lines starting with [package_name:]) 
	PACKAGE pkg;
	LOCATION tmplocation;
	DEPENDENCY tmpDep;
	DESCRIPTION tmpDesc;
	string tmpDescStr;
	string tmpDepStr;
	tmpDesc.set_language("en");
	string tmpstr;
	int tmpSize;
	int lines = 0;
	int name_start = 0;
	bool endReached = false;

	string slackPackageName;
	string slackPackageLocation;
	string slackCompressedSize;
	string slackUncompressedSize;
	string slackRequired;
	string slackSuggests;
	string slackDescription;
	string filename;
	string tmp;
	string pkgNameKeyword = "PACKAGE NAME:  ";
	string pkgLocationKeyword = "PACKAGE LOCATION:  ";
	string pkgCompressedKeyword = "PACKAGE SIZE (compressed):  ";
	string pkgUncompressedKeyword = "PACKAGE SIZE (uncompressed):  ";
	string pkgRequiredKeyword = "PACKAGE REQUIRED:  ";
	string pkgSuggestsKeyword = "PACKAGE SUGGESTS:  ";
	string pkgDescriptionKeyword = "PACKAGE DESCRIPTION:";
	unsigned int pos;
	unsigned int num=0;
	tmpstr = packageslist;
	while (!endReached)
	{
		printf("Parsing %d package\n", num);
		// Stage 1: retrieving dirty info
		pos = tmpstr.find(pkgNameKeyword);
		tmpstr=tmpstr.substr(pos+pkgNameKeyword.length()); // Cuts off a header and keyword
		pos = tmpstr.find("\n"); // Searching end of line
		slackPackageName = tmpstr.substr(0,pos); // Filling package name (in full form)
		printf("slackPackageName = %s\n", slackPackageName.c_str());
		
		pos = tmpstr.find(pkgLocationKeyword);
		tmpstr = tmpstr.substr(pkgLocationKeyword.length()+pos);
		pos = tmpstr.find("\n");
		slackPackageLocation = tmpstr.substr(0,pos);
		printf("slackPackageLocation = %s\n", slackPackageLocation.c_str());
	
		pos = tmpstr.find(pkgCompressedKeyword);
		tmpstr = tmpstr.substr(pos + pkgCompressedKeyword.length());
		pos = tmpstr.find("\n");
		slackCompressedSize = tmpstr.substr(0,pos);
		printf("slackCompressedSize = %s\n", slackCompressedSize.c_str());

		pos = tmpstr.find(pkgUncompressedKeyword);
		tmpstr = tmpstr.substr(pos + pkgUncompressedKeyword.length());
		pos = tmpstr.find("\n");
		slackUncompressedSize = tmpstr.substr(0,pos);
		printf("slackUncompressedSize = %s\n", slackUncompressedSize.c_str());
	
		pos = tmpstr.find(pkgRequiredKeyword);
		if (pos < tmpstr.find(pkgNameKeyword))
		{
			printf("required list present!\n");
			tmpstr = tmpstr.substr(pos + pkgRequiredKeyword.length());
			pos = tmpstr.find("\n");
			slackRequired = tmpstr.substr(0,pos);
			printf("slackRequired = %s\n", slackRequired.c_str());
		}
		else
		{
			printf("no required list\n");
			slackRequired.clear();
		}

		pos = tmpstr.find(pkgSuggestsKeyword);
		if (pos < tmpstr.find(pkgNameKeyword))
		{
			printf("suggest list present!\n");
			tmpstr = tmpstr.substr(pos + pkgSuggestsKeyword.length());
			pos = tmpstr.find("\n");
			slackSuggests = tmpstr.substr(0,pos);
			printf("slackSuggests = %s\n", slackSuggests.c_str());
		}
		else
		{
			slackSuggests.clear();
			printf("no suggest list\n");
		}

		pos = tmpstr.find(pkgDescriptionKeyword);
		tmpstr = tmpstr.substr(pos + pkgSuggestsKeyword.length()+1); // +1 because there are newline
		pos = tmpstr.find(pkgNameKeyword);
		printf("searched end\n");
		if (pos == std::string::npos)
		{
			printf("description end reached\n");
			slackDescription = tmpstr;
			
			printf("slackDescription = %s\n", slackDescription.c_str());
			endReached = true;
		}
		else
		{
			slackDescription = tmpstr.substr(0,pos-1);

			printf("slackDescription = %s\n", slackDescription.c_str());
		}
		
		// Stage 2: info cleanup
		
		// Filename
		pkg.set_filename(slackPackageName);
		filename = slackPackageName;
		// Name, version, arch, build
		pos = slackDescription.find(": ");
		if (pos != std::string::npos)
		{
			pkg.set_name(slackDescription.substr(1,pos-1));
			slackPackageName = slackPackageName.substr(slackPackageName.find("-")+1);
			pkg.set_version(slackPackageName.substr(0, slackPackageName.find("-")));
			slackPackageName = slackPackageName.substr(slackPackageName.find("-")+1);
			pkg.set_arch(slackPackageName.substr(0, slackPackageName.find("-")));
			slackPackageName = slackPackageName.substr(slackPackageName.find("-")+1);
			pkg.set_build(slackPackageName.substr(0, slackPackageName.find(".tgz")));
		}
		else
		{
			name_start=0;
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
						pkg.set_name(tmp);
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
					pkg.set_version(tmp);
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
					pkg.set_arch(tmp);
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
			pkg.set_build(tmp);

			tmp.clear();
		}
		
		printf("package name: %s\n", pkg.get_name().c_str());
		printf("package version: %s\n", pkg.get_version().c_str());
		printf("package arch: %s\n", pkg.get_arch().c_str());
		printf("package build: %s\n", pkg.get_build().c_str());
		// Location
		tmplocation.set_path(slackPackageLocation);
		pkg.get_locations()->add(tmplocation);

		// Size
		tmpSize = atoi(slackCompressedSize.substr(0, slackCompressedSize.length()-2).c_str());
		pkg.set_compressed_size(IntToStr(tmpSize*1024));
		printf("package size (compressed): %s\n", pkg.get_compressed_size().c_str());
		tmpSize = atoi(slackUncompressedSize.substr(0, slackUncompressedSize.length()-2).c_str());
		pkg.set_installed_size(IntToStr(tmpSize*1024));
		printf("package size (uncompressed): %s\n", pkg.get_installed_size().c_str());

		// Dependencies
		while (slackRequired.find_first_of(",")!=std::string::npos)
		{
			printf("Proceeding dep, slackRequired = %s\n", slackRequired.c_str());
			tmpDep.clear();
			tmpDep.set_type("DEPENDENCY");
			pos = slackRequired.find_first_of(" =><");
			if (pos < slackRequired.find_first_of(",\n"))
			{
				tmpDep.set_package_name(slackRequired.substr(0, pos));
				slackRequired = slackRequired.substr(pos);
				pos = slackRequired.find_first_not_of(" =><");
				tmpDepStr = slackRequired.substr(0,pos);
				tmpDep.set_condition(hcondition2xml(tmpDepStr));
				slackRequired = slackRequired.substr(pos);
				pos = slackRequired.find_first_of(",");
				tmpDep.set_package_version(slackRequired.substr(0,pos));
			}
			else
			{
				pos = slackRequired.find_first_of(",")+1;
				tmpDep.set_package_name(slackRequired.substr(0,pos));
				slackRequired = slackRequired.substr(pos);
			}

			pkg.get_dependencies()->add(tmpDep);
		}

		printf("reached suggestions\n");
		// Suggestions
		while (slackRequired.find_first_of(",")!=std::string::npos)
		{
			tmpDep.clear();
			tmpDep.set_type("SUGGEST");
			pos = slackRequired.find_first_of(" =><");
			if (pos < slackRequired.find_first_of(","))
			{
				tmpDep.set_package_name(slackRequired.substr(0, pos));
				slackRequired = slackRequired.substr(pos);
				pos = slackRequired.find_first_not_of(" =><");
				tmpDepStr = slackRequired.substr(0,pos);
				tmpDep.set_condition(hcondition2xml(tmpDepStr));
				slackRequired = slackRequired.substr(pos);
				pos = slackRequired.find_first_of(",");
				tmpDep.set_package_version(slackRequired.substr(0,pos));
			}
			else
			{
				pos = slackRequired.find_first_of(",");
				tmpDep.set_package_name(slackRequired.substr(0,pos));
				tmpDep.set_condition("any");
				slackRequired = slackRequired.substr(pos);
			}

			pkg.get_dependencies()->add(tmpDep);
		}
		
		printf("reached description\n");
		// Description
		tmpDescStr.clear();
		
		if (slackDescription.length()>0)
		{
			slackDescription = slackDescription.substr(1);
			if (slackDescription.length()>=slackDescription.find(pkg.get_name()+": ")+pkg.get_name().length()+2)
			{
				slackDescription = slackDescription.substr(slackDescription.find(pkg.get_name()+": ")+pkg.get_name().length()+2);
				tmpDesc.set_shorttext(slackDescription.substr(0, slackDescription.find_first_of("\n")));
				pkg.set_short_description(tmpDesc.get_shorttext());
				printf("short description: %s\n", tmpDesc.get_shorttext().c_str());
			}
			pos = slackDescription.find("\n");
			lines = 0;
			while (pos != std::string::npos && lines < 11)
			{
				pos = slackDescription.find(pkg.get_name()+": ");
				if (pos == std::string::npos)
				{
					printf("Description end.\n");
				}
				else
				{
					slackDescription = slackDescription.substr(pos+pkg.get_name().length()+2);
					tmpDescStr+=slackDescription.substr(0,slackDescription.find("\n"))+"\n";
					lines++;
				}
			}
			printf("Description: %s\n", tmpDescStr.c_str());
			tmpDesc.set_text(tmpDescStr);
			pkg.set_description(tmpDesc.get_text());
			pkg.get_descriptions()->add(tmpDesc);
		}
		pkglist->add(pkg);
		pkg.clear();
		num++;
		//if (num == 1) return 0;
		printf("done\n");
	}

return 0;
}	

int xml2package(XMLNode pkgnode, PACKAGE *data)
{
	PackageConfig p(pkgnode);
	
	data->set_name(p.getName());
	data->set_version(p.getVersion());
	data->set_arch(p.getArch());
	data->set_build(p.getBuild());
	data->set_packager(p.getAuthorName());
	data->set_packager_email(p.getAuthorEmail());
	data->set_descriptions(p.getDescriptions());

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
	if (!server_url.empty()) _root.addAttribute("server_url", server_url.c_str());
	
	// Next, run thru files and extract data.
	// We consider that repository root is current directory. So, what we need to do:
	// Enter each sub-dir, get each file which name ends with .tgz, extracts xml (and other) data from them, 
	// and build an XML tree for whole repository, then write it to ./packages.xml
	
	ftw(".", ProcessPackage, 600);

	// Finally, write our XML tree to file
	_root.writeToFile("packages.xml");
	// Compress file
	if (system("gzip -f packages.xml")==0) printf("Repository index created successfully\n");
	else printf("Error creating repository index!\n");
	return 0;
}

// Add other such functions for other repository types.
int Repository::get_index(string server_url, PACKAGE_LIST *packages, unsigned int type)
{
	// First: detecting repository type
	// Trying to download in this order (if successful, we have detected a repository type):
	// 1. packages.xml.gz 	(Native MOPSLinux)
	// 2. PACKAGES.TXT	(Legacy Slackware)
	// 3. Packages.gz	(Debian)
	// (and something else for RPM, in future)
	string index_filename = get_tmp_file();
	printf("[%s]...",server_url.c_str());
	string cm = "gunzip -f "+index_filename+".gz 2>/dev/null";
	if (type == TYPE_MPKG || type == TYPE_AUTO)
	{

		printf("type = %d\n", type);
	       if (CommonGetFile(server_url + "packages.xml.gz", index_filename+".gz")==DOWNLOAD_OK)
		{
			printf("download ok, validating contents...\n");
			if (system(cm.c_str())==0 && ReadFile(index_filename).find("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<repository")!=std::string::npos)
			{
				printf("Native MPKG repository detected\n");
				type = TYPE_MPKG;
			}
		}
	}
	
	if (type == TYPE_SLACK || type == TYPE_AUTO)
	{
		
		printf("type = %d\n", type);
		if (CommonGetFile(server_url + "PACKAGES.TXT", index_filename)==DOWNLOAD_OK)
		{
			printf("download ok, validating contents...\n");
			if (ReadFile(index_filename).find("PACKAGE NAME:  ")!=std::string::npos)
			{
				printf("Slackware repository detected\n");
				type = TYPE_SLACK;
			}
		}
	}

	if (type == TYPE_DEBIAN || type == TYPE_AUTO)
	{
		if(CommonGetFile(server_url + "Packages.gz", index_filename+".gz")==DOWNLOAD_OK)
		{
			printf("Debian repository detected\n");
			type = TYPE_DEBIAN;
		}
	}

	if (type != TYPE_MPKG && type != TYPE_SLACK && type!=TYPE_DEBIAN)
	{
		printf("Error downloading package index: download error, or unsupported repository type\n");
		return -1;
	}
	else
	{
		printf("Repository type detected successfully, proceeding next...\n");
	}
	PACKAGE pkg;
	string gzip_line;
	string xml_name=index_filename;
	string gzxml_name=xml_name+".gz";
	XMLNode repository_root;
	int pkg_count;
	gzip_line="gunzip -f "+gzxml_name;
	
	switch(type)
	{
		case TYPE_MPKG:
			if (system(gzip_line.c_str()) == 0)
			{
				printf("done\n");
				repository_root=XMLNode::openFileHelper(xml_name.c_str(), "repository");
				pkg_count=repository_root.nChildNode("package");
				if (pkg_count==0)
				{
					printf(_("Repository has no packages\n"));
					return 0;
				}
				for (int i=0; i<pkg_count; i++)
				{
					pkg.clear();
					xml2package(repository_root.getChildNode("package", i), &pkg);
					packages->add(pkg);
				}

			}
			else
			{
				printf(_("FAILED\n"));
				return -1;
			}
			break;
		case TYPE_SLACK:
			printf("Parsing slackware repository\n");
			return slackpackages2list(ReadFile(index_filename), packages);

		case TYPE_DEBIAN:
		default:
			break;
	}
	return 0;
}
