/******************************************************************
 * Repository class: build index, get index...etc.
 * $Id: repository.cpp,v 1.39 2007/05/16 01:15:58 i27249 Exp $
 * ****************************************************************/
#include "repository.h"
#include <iostream>
Repository::Repository(){}
Repository::~Repository(){}

XMLNode _root;

int slackpackages2list (string packageslist, string md5list, PACKAGE_LIST *pkglist, string server_url)
{
	if (actionBus._abortActions)
	{
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	if (packageslist.length()<20)
	{
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
	string md5tmp;
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

	// Visualization
	actionBus.setActionProgress(ACTIONID_DBUPDATE, 0);
	unsigned int tmp_max = tmpstr.length();
	actionBus.setActionProgressMaximum(ACTIONID_DBUPDATE, (double) tmpstr.length());
	//progressEnabled = true;

	while (!endReached)
	{
		if (actionBus._abortActions)
		{
			actionBus._abortComplete=true;
			actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
			return MPKGERROR_ABORTED;
		}

		actionBus.setActionProgress (ACTIONID_DBUPDATE, (double) tmp_max - tmpstr.length());
		debug("Parsing "+IntToStr(num)+" package");
		// Stage 1: retrieving dirty info
		pos = tmpstr.find(pkgNameKeyword);
		tmpstr=tmpstr.substr(pos+pkgNameKeyword.length()); // Cuts off a header and keyword
		pos = tmpstr.find("\n"); // Searching end of line
		slackPackageName = tmpstr.substr(0,pos); // Filling package name (in full form)
		debug("slackPackageName = "+slackPackageName);
		
		pos = tmpstr.find(pkgLocationKeyword);
		tmpstr = tmpstr.substr(pkgLocationKeyword.length()+pos);
		pos = tmpstr.find("\n");
		slackPackageLocation = tmpstr.substr(0,pos);
		debug("slackPackageLocation = "+ slackPackageLocation);
	
		pos = tmpstr.find(pkgCompressedKeyword);
		tmpstr = tmpstr.substr(pos + pkgCompressedKeyword.length());
		pos = tmpstr.find("\n");
		slackCompressedSize = tmpstr.substr(0,pos);
		debug("slackCompressedSize = " + slackCompressedSize);

		pos = tmpstr.find(pkgUncompressedKeyword);
		tmpstr = tmpstr.substr(pos + pkgUncompressedKeyword.length());
		pos = tmpstr.find("\n");
		slackUncompressedSize = tmpstr.substr(0,pos);
		debug("slackUncompressedSize = " + slackUncompressedSize);
	
		pos = tmpstr.find(pkgRequiredKeyword);
		if (pos < tmpstr.find(pkgNameKeyword))
		{
			debug("required list present!");
			tmpstr = tmpstr.substr(pos + pkgRequiredKeyword.length());
			pos = tmpstr.find("\n");
			slackRequired = tmpstr.substr(0,pos);
			debug("slackRequired = " + slackRequired);
		}
		else
		{
			debug("no required list");
			slackRequired.clear();
		}

		pos = tmpstr.find(pkgSuggestsKeyword);
		if (pos < tmpstr.find(pkgNameKeyword))
		{
			debug("suggest list present!");
			tmpstr = tmpstr.substr(pos + pkgSuggestsKeyword.length());
			pos = tmpstr.find("\n");
			slackSuggests = tmpstr.substr(0,pos);
			debug("slackSuggests = " + slackSuggests);
		}
		else
		{
			slackSuggests.clear();
			debug("no suggest list");
		}

		pos = tmpstr.find(pkgDescriptionKeyword);
		tmpstr = tmpstr.substr(pos + pkgSuggestsKeyword.length()+1); // +1 because there are newline
		pos = tmpstr.find(pkgNameKeyword);
		debug("searched end");
		if (pos == std::string::npos)
		{
			debug("description end reached");
			slackDescription = tmpstr;
			
			debug("slackDescription = " + slackDescription);
			endReached = true;
		}
		else
		{
			slackDescription = tmpstr.substr(0,pos-1);

			debug("slackDescription = "+ slackDescription);
		}
		
		// Stage 2: info cleanup
		
		// Filename
		pkg.set_filename(slackPackageName);
		if (md5list.find(slackPackageName) == std::string::npos)
		{
			printf("MD5 checksum not found for package %s, it will prevent from installing this\n", slackPackageName.c_str());
		}
		else
		{
			debug("md5 found");
			md5tmp = md5list.substr(0,md5list.find(slackPackageName));
		}
		md5tmp = md5tmp.substr(0, md5tmp.find_last_of(" \t"));
		md5tmp = md5tmp.substr(md5tmp.rfind("\n")+1);
		md5tmp = cutSpaces(md5tmp);
		pkg.set_md5(md5tmp);
		debug("MD5 = " + md5tmp);
		filename = slackPackageName;

		// Name, version, arch, build
		pos = slackDescription.find(":");
		debug("pos = "+IntToStr(pos));
		tmp.clear();
		if (true)
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
		
		debug("package name: "+ pkg.get_name());
		debug("package version: "+ pkg.get_version());
		debug("package arch: "+ pkg.get_arch());
		debug("package build: "+ pkg.get_build());
		// Location
		if (slackPackageLocation.find("./") == 0)
		{
			debug("DOTCUT:");
			slackPackageLocation = slackPackageLocation.substr(2);
		}
		tmplocation.set_path(slackPackageLocation);
		tmplocation.get_server()->set_url(server_url);
		pkg.get_locations()->add(tmplocation);
		debug("LOC_SET: "+pkg.get_locations()->get_location(0)->get_path());

		// Size
		tmpSize = atoi(slackCompressedSize.substr(0, slackCompressedSize.length()-2).c_str());
		pkg.set_compressed_size(IntToStr(tmpSize*1024));
		debug("package size (compressed): "+ pkg.get_compressed_size());
		tmpSize = atoi(slackUncompressedSize.substr(0, slackUncompressedSize.length()-2).c_str());
		pkg.set_installed_size(IntToStr(tmpSize*1024));
		debug("package size (uncompressed): "+ pkg.get_installed_size());

		debug("reached description");
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
				debug("short description: "+tmpDesc.get_shorttext());
			}
			pos = slackDescription.find("\n");
			lines = 0;
			while (pos != std::string::npos && lines < 11)
			{
				pos = slackDescription.find(pkg.get_name()+": ");
				if (pos == std::string::npos)
				{
					debug("Description end");
				}
				else
				{
					slackDescription = slackDescription.substr(pos+pkg.get_name().length()+2);
					tmpDescStr+=slackDescription.substr(0,slackDescription.find("\n"))+"\n";
					lines++;
				}
			}
			debug("Description: "+ tmpDescStr);
			tmpDesc.set_text(tmpDescStr);
			pkg.set_description(tmpDesc.get_text());
//#ifdef ENABLE_INTERNATIONAL			
			pkg.get_descriptions()->add(tmpDesc);
//#endif
		}
		pkglist->add(pkg);
		pkg.clear();
		num++;
		debug("done");
	}

	return 0;
}	// End slackpackages2list()

int xml2package(XMLNode pkgnode, PACKAGE *data)
{
	PackageConfig p(pkgnode);
	if (!p.parseOk) return -100;
	
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

unsigned int pkgcounter;
int ProcessPackage(const char *filename, const struct stat *file_status, int filetype)
{
	debug("processing package "+ (string) filename);
	string _package=filename;
       	string ext;
	for (unsigned int i=_package.length()-4;i<_package.length();i++)
	{
		ext+=_package[i];
	}

	if (filetype==FTW_F && ext==".tgz")
	{

		pkgcounter++;
		cout<< "indexing file " << filename << "..."<<endl;
		LocalPackage lp(_package);
		lp.injectFile(true);
		_root.addChild(lp.getPackageXMLNode());
	}
	return 0;
}


int Repository::build_index(string server_url, string server_name, bool rebuild)
{
	if (rebuild)
	{
		if (system("gunzip packages.xml.gz")!=0)
		{
			printf("No previous index found, using defaults\n");
			server_url="no_url";
			server_name="no_name";
		}
		else
		{
			XMLNode tmpNode = XMLNode::parseFile("packages.xml", "repository");
			server_url = (string) tmpNode.getAttributeValue(0);
			server_name = (string) tmpNode.getAttributeValue(1);
		}
	}	
	pkgcounter=0;
	// First of all, initialise main XML tree. Due to some code restrictions, we use global variable _root.
	_root=XMLNode::createXMLTopNode("repository");
	if (!server_url.empty()) _root.addAttribute("url", server_url.c_str());
	if (!server_name.empty()) _root.addAttribute("name", server_name.c_str());
	
	// Next, run thru files and extract data.
	// We consider that repository root is current directory. So, what we need to do:
	// Enter each sub-dir, get each file which name ends with .tgz, extracts xml (and other) data from them, 
	// and build an XML tree for whole repository, then write it to ./packages.xml
	
	ftw(".", ProcessPackage, 600);

	// Finally, write our XML tree to file
	_root.writeToFile("packages.xml");
	// Compress file
	if (system("gzip -f packages.xml")==0)
	       printf("\n-------------SUMMARY------------------\nRepository URL: %s\nRepository name: %s\nTotal: %d packages\n\nRepository index created successfully\n",\
			       server_url.c_str(), server_name.c_str(), pkgcounter);
	else printf("Error creating repository index!\n");
	return 0;
}

// Add other such functions for other repository types.
int Repository::get_index(string server_url, PACKAGE_LIST *packages, unsigned int type)
{
	if (actionBus._abortActions)
	{
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	currentStatus = "Updating data from "+ server_url+"...";
	debug("get_index!");
	// First: detecting repository type
	// Trying to download in this order (if successful, we have detected a repository type):
	// 1. packages.xml.gz 	(Native MOPSLinux)
	// 2. PACKAGES.TXT	(Legacy Slackware)
	// 3. Packages.gz	(Debian)
	// (and something else for RPM, in future)
	string index_filename = get_tmp_file();
	string md5sums_filename = get_tmp_file();
	printf("\t[%s] ...\n",server_url.c_str());
	string cm = "gunzip -f "+index_filename+".gz 2>/dev/null";
	if (type == TYPE_MPKG || type == TYPE_AUTO)
	{
		actionBus.getActionState(0);
		debug("trying MPKG, type = "+ IntToStr(type));
	       if (CommonGetFile(server_url + "packages.xml.gz", index_filename+".gz")==DOWNLOAD_OK)
		{
			actionBus.getActionState(0);
			debug("download ok, validating contents...");
			if (system(cm.c_str())==0 && \
					ReadFile(index_filename).find("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<repository")!=std::string::npos)
			{
				currentStatus = "Detected native MPKG repository";
				type = TYPE_MPKG;
			}
		}
	}
	if (actionBus._abortActions)
	{
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	if (type == TYPE_SLACK || type == TYPE_AUTO)
	{
		
		debug("trying SLACK, type = "+ IntToStr(type));
		if (CommonGetFile(server_url + "PACKAGES.TXT", index_filename)==DOWNLOAD_OK)
		{
			debug("download ok, validating contents...");
			if (ReadFile(index_filename).find("PACKAGE NAME:  ")!=std::string::npos)
			{
				currentStatus = "Detected legacy Slackware repository";
				if (CommonGetFile(server_url + "CHECKSUMS.md5", md5sums_filename) == DOWNLOAD_OK)
				{
					type = TYPE_SLACK;
				}
				else 
				{
					printf("Error downloading checksums\n");
					return -1; // Download failed: no checksums or checksums download error
				}
			}
		}
	}
	if (actionBus._abortActions)
	{
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	if (type == TYPE_DEBIAN || type == TYPE_AUTO)
	{
		if(CommonGetFile(server_url + "Packages.gz", index_filename+".gz")==DOWNLOAD_OK)
		{
			type = TYPE_DEBIAN;
		}
	}

	if (type != TYPE_MPKG && type != TYPE_SLACK && type!=TYPE_DEBIAN)
	{
		currentStatus = "Error updating data from "+server_url+": download error or unsupported type";
		printf("Error downloading package index: download error, or unsupported repository type\n");
		return -1;
	}
	
	PACKAGE pkg;
	string xml_name=index_filename;
	XMLNode repository_root;
	int pkg_count;
	int ret=0;
	currentStatus = "["+server_url+"] Importing data...";
	if (actionBus._abortActions)
	{
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}

	switch(type)
	{
		case TYPE_MPKG:
				repository_root=XMLNode::openFileHelper(xml_name.c_str(), "repository");
				pkg_count=repository_root.nChildNode("package");
				if (pkg_count==0)
				{
					printf(_("Repository has no packages\n"));
					return 0;
				}
				
				actionBus.setActionProgress(ACTIONID_DBUPDATE, 0);
				actionBus.setActionProgressMaximum(ACTIONID_DBUPDATE, pkg_count);
				for (int i=0; i<pkg_count; i++)
				{
					if (actionBus._abortActions)
					{
						actionBus._abortComplete=true;
						actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
						return MPKGERROR_ABORTED;
					}

					actionBus.setActionProgress(ACTIONID_DBUPDATE, i);
					pkg.clear();
					xml2package(repository_root.getChildNode("package", i), &pkg);
					// Adding location data
					pkg.get_locations()->get_location(0)->get_server()->set_url(server_url);
					packages->add(pkg);
				}
			break;
		case TYPE_SLACK:
			ret = slackpackages2list(ReadFile(index_filename), ReadFile(md5sums_filename), packages, server_url);
			break;

		case TYPE_DEBIAN:
			break;
		default:
			break;
	}
	return ret;
}


