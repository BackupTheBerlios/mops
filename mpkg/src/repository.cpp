/******************************************************************
 * Repository class: build index, get index...etc.
 * $Id: repository.cpp,v 1.74 2007/11/03 01:08:15 i27249 Exp $
 * ****************************************************************/
#include "repository.h"
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/debugXML.h>

Repository::Repository(){}
Repository::~Repository(){}

//XMLNode _root; 
xmlNodePtr _rootNode;
xmlDocPtr __doc;
//XMLNode _rootFList;
int slackpackages2list (string *packageslist, string *md5list, PACKAGE_LIST *pkglist, string server_url)
{
	if (actionBus._abortActions)
	{
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		say("aborted\n");
		return MPKGERROR_ABORTED;
	}

	if (packageslist->length()<20)
	{
		say("aborted\n");
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
	PACKAGE *pkg = new PACKAGE;
	LOCATION *tmplocation = new LOCATION;
#ifdef ENABLE_INTERNATIONAL
	DESCRIPTION *tmpDesc = new DESCRIPTION;
	tmpDesc->set_language("en");
#endif
	string *tmpDescStr = new string;
	string *tmpstr = packageslist;
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
	//tmpstr = packageslist;

	// Visualization
	actionBus.setActionProgress(ACTIONID_DBUPDATE, 0);
	unsigned int tmp_max = tmpstr->length();
	actionBus.setActionProgressMaximum(ACTIONID_DBUPDATE, (double) tmpstr->length());
	//progressEnabled = true;

	while (!endReached)
	{
		if (actionBus._abortActions)
		{
			actionBus._abortComplete=true;
			actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		say("aborted\n");
			return MPKGERROR_ABORTED;
		}

		actionBus.setActionProgress (ACTIONID_DBUPDATE, (double) tmp_max - tmpstr->length());
		mDebug("Parsing "+IntToStr(num)+" package");
		// Stage 1: retrieving dirty info
		pos = tmpstr->find(pkgNameKeyword);
		*tmpstr=tmpstr->substr(pos+pkgNameKeyword.length()); // Cuts off a header and keyword
		pos = tmpstr->find("\n"); // Searching end of line
		slackPackageName = tmpstr->substr(0,pos); // Filling package name (in full form)
		mDebug("slackPackageName = "+slackPackageName);
		
		pos = tmpstr->find(pkgLocationKeyword);
		*tmpstr = tmpstr->substr(pkgLocationKeyword.length()+pos);
		pos = tmpstr->find("\n");
		slackPackageLocation = tmpstr->substr(0,pos);
		mDebug("slackPackageLocation = "+ slackPackageLocation);
	
		pos = tmpstr->find(pkgCompressedKeyword);
		*tmpstr = tmpstr->substr(pos + pkgCompressedKeyword.length());
		pos = tmpstr->find("\n");
		slackCompressedSize = tmpstr->substr(0,pos);
		mDebug("slackCompressedSize = " + slackCompressedSize);

		pos = tmpstr->find(pkgUncompressedKeyword);
		*tmpstr = tmpstr->substr(pos + pkgUncompressedKeyword.length());
		pos = tmpstr->find("\n");
		slackUncompressedSize = tmpstr->substr(0,pos);
		mDebug("slackUncompressedSize = " + slackUncompressedSize);
	
		pos = tmpstr->find(pkgRequiredKeyword);
		if (pos < tmpstr->find(pkgNameKeyword))
		{
			mDebug("required list present!");
			*tmpstr = tmpstr->substr(pos + pkgRequiredKeyword.length());
			pos = tmpstr->find("\n");
			slackRequired = tmpstr->substr(0,pos);
			mDebug("slackRequired = " + slackRequired);
		}
		else
		{
			mDebug("no required list");
			slackRequired.clear();
		}

		pos = tmpstr->find(pkgSuggestsKeyword);
		if (pos < tmpstr->find(pkgNameKeyword))
		{
			mDebug("suggest list present!");
			*tmpstr = tmpstr->substr(pos + pkgSuggestsKeyword.length());
			pos = tmpstr->find("\n");
			slackSuggests = tmpstr->substr(0,pos);
			mDebug("slackSuggests = " + slackSuggests);
		}
		else
		{
			slackSuggests.clear();
			mDebug("no suggest list");
		}

		pos = tmpstr->find(pkgDescriptionKeyword);
		*tmpstr = tmpstr->substr(pos + pkgSuggestsKeyword.length()+1); // +1 because there are newline
		pos = tmpstr->find(pkgNameKeyword);
		mDebug("searched end");
		if (pos == std::string::npos)
		{
			mDebug("description end reached");
			slackDescription = *tmpstr;
			
			mDebug("slackDescription = " + slackDescription);
			endReached = true;
		}
		else
		{
			slackDescription = tmpstr->substr(0,pos-1);

			mDebug("slackDescription = "+ slackDescription);
		}
		
		// Stage 2: info cleanup
		
		// Filename
		pkg->set_filename(&slackPackageName);
		
		if (md5list->find(slackPackageName) == std::string::npos)
		{
			mError("MD5 checksum not found for package " +  slackPackageName +", skipping");
		}
		else
		{
			mDebug("md5 found");
			md5tmp = md5list->substr(0,md5list->find(slackPackageName));
		}
		md5tmp = md5tmp.substr(0, md5tmp.find_last_of(" \t"));
		md5tmp = md5tmp.substr(md5tmp.rfind("\n")+1);
		md5tmp = cutSpaces(md5tmp);
		pkg->set_md5(&md5tmp);
		mDebug("MD5 = " + md5tmp);
		
		filename = slackPackageName;

		// Name, version, arch, build
		pos = slackDescription.find(":");
		mDebug("pos = "+IntToStr(pos));
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
						pkg->set_name(&tmp);
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
					pkg->set_version(&tmp);
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
					pkg->set_arch(&tmp);
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
			pkg->set_build(&tmp);

			tmp.clear();
		}
		
		mDebug("package name: "+ *pkg->get_name());
		mDebug("package version: "+ *pkg->get_version());
		mDebug("package arch: "+ *pkg->get_arch());
		mDebug("package build: "+ *pkg->get_build());
		// Location
		if (slackPackageLocation.find("./") == 0)
		{
			mDebug("DOTCUT:");
			slackPackageLocation = slackPackageLocation.substr(2);
		}
		tmplocation->set_path(&slackPackageLocation);
		tmplocation->set_server_url(&server_url);
		pkg->get_locations()->push_back(*tmplocation);
		//mDebug("LOC_SET: "+*pkg->get_locations()->at(0)->get_path());

		// Size
		tmpSize = atoi(slackCompressedSize.substr(0, slackCompressedSize.length()-2).c_str());
		*pkg->get_compressed_size()=IntToStr(tmpSize*1024);
		mDebug("package size (compressed): "+ *pkg->get_compressed_size());
		tmpSize = atoi(slackUncompressedSize.substr(0, slackUncompressedSize.length()-2).c_str());
		*pkg->get_installed_size()=IntToStr(tmpSize*1024);
		mDebug("package size (uncompressed): "+ *pkg->get_installed_size());

		mDebug("reached description");
		// Description
		tmpDescStr->clear();
		
		if (slackDescription.length()>0)
		{
			slackDescription = slackDescription.substr(1);
			if (slackDescription.length() >= slackDescription.find(*pkg->get_name()+": ") + pkg->get_name()->length()+2)
			{
				slackDescription = slackDescription.substr(slackDescription.find(*pkg->get_name()+": ")+pkg->get_name()->length()+2);
				//tmpDesc->set_shorttext(slackDescription.substr(0, slackDescription.find_first_of("\n")));
				*pkg->get_short_description()=slackDescription.substr(0, slackDescription.find_first_of("\n"));
				//mDebug("short description: "+slackDescription.substr(0, slackDescription.find_first_of("\n")));
			}
			pos = slackDescription.find("\n");
			lines = 0;
			while (pos != std::string::npos && lines < 11)
			{
				pos = slackDescription.find(*pkg->get_name()+": ");
				if (pos == std::string::npos)
				{
					mDebug("Description end");
				}
				else
				{
					slackDescription = slackDescription.substr(pos+pkg->get_name()->length()+2);
					*tmpDescStr = *tmpDescStr + slackDescription.substr(0,slackDescription.find("\n"))+"\n";
					lines++;
				}
			}
			mDebug("Description: "+ *tmpDescStr);
			//tmpDesc->set_text(*tmpDescStr);
			pkg->set_description(tmpDescStr);
#ifdef ENABLE_INTERNATIONAL			
			pkg->get_descriptions()->add(tmpDesc);
#endif
		}
		pkglist->add(pkg);
		pkg->clear();
		num++;
		mDebug("done");
	}
	delete pkg;
	delete tmplocation;
	//delete tmpDesc;
	delete tmpDescStr;
	//delete tmpstr;

	return 0;
}	// End slackpackages2list()


unsigned int pkgcounter;
vector<string> pkgDupeNames;

int pkgcount_ftw;
int ProcessPackage(const char *filename, const struct stat *file_status, int filetype)
{
	//XMLNode tmpX;
	unsigned short x=0, y=0;

	if (file_status->st_ino!=0) x=y;

	mDebug("processing package "+ (string) filename);
       	string ext = getExtension(filename);

	if (filetype==FTW_F)
	{
		if (ext!="tgz" && ext!="spkg") return 0;

		pkgcounter++;
		printf("[%d/%d] indexing file %s\n",pkgcounter,pkgcount_ftw, filename);
//		cout<< "indexing file " << filename << "..."<<endl;
		FILE *log=fopen("index.log", "a");
		LocalPackage lp(filename);
		mDebug("PP 0-0");
		int errCode = lp.injectFile(true);
		//printf("injectFile returned %d\n", errCode);
		mDebug("PP 00-1");
		if (log)
		{
			if (errCode==0)
			{
				fprintf(log, "indexing package %s: OK\n", filename);
				fclose(log);
			}
			else 
			{
				fprintf(log, "indexing file %s FAILED: error code %d\n", filename, errCode);
				fclose(log);
			//	printf("Returning FAILURE\n");
				return 0;
			}

		}
		else mError("unable to open log file");

		mDebug("PP 0-2");
		if (_rootNode == NULL) {
			mDebug("[3]_rootNode == NULL");
		} else {
			mDebug("[3]_rootNode != NULL");
		}
		if (__doc == NULL) {
			mDebug("[2] _root == NULL");
		} else {
			mDebug("[2] _root != NULL");
		}

		int bufsize;
		xmlChar * membuf = lp.getPackageXMLNodeXPtr(&bufsize);
		xmlDocPtr __tmpXmlDoc = xmlParseMemory((const char *) membuf, bufsize);//"UTF-8", 0);
		xmlFree(membuf);
		xmlNodePtr __packagesRootNode = xmlDocGetRootElement(__tmpXmlDoc);

		if (__tmpXmlDoc == 0) {
			mDebug("Temp xml doc error");
		} else {
			mDebug("temp xml doc ok");
		}

		if (__packagesRootNode == NULL) {
			mDebug("package xml root node error");
		} else {
			mDebug("package xml root node ok");
			const xmlChar * __root_node_name = __packagesRootNode->name;
			mDebug(" __packagesRootNode->name = " + (string) (const char *)__root_node_name);
		}

		xmlNodePtr __node  = xmlAddChild(_rootNode, __packagesRootNode);
		if (__node == NULL) {
			mDebug("new package xml node error");
		} else {
			mDebug("new package xml node ok");
			const xmlChar * __st = __node->name;
			mDebug("new package xml node name = " + (string) (const char *)__st);
		}

		if (__packagesRootNode == NULL) {
			mDebug("root package xml after apped error");
		} else {
			mDebug("root package xml after apped ok");
			const xmlChar * __st = __packagesRootNode->name;
			mDebug("root package xml node name = " + (string) (const char *)__st);
		}

		/*mDebug("Saving temp repo xml dump ");
		FILE *__xmlDump = fopen("/tmp/xmldump-repo.xml", "w");
		if (xmlDocDump(__xmlDump, __doc) != -1) {
			fclose(__xmlDump);
			mDebug("temp Xml dump saved");
		} else {
			fclose(__xmlDump);
			mDebug("temp Xml dump failed");
		}*/

		mDebug("PP 0-3");
		
		// Dupe check
		for (unsigned int i=0; i<pkgDupeNames.size(); i++)
		{
			if (*lp.data.get_name()==pkgDupeNames[i])
			{
				// Dupe found, notify!
				say("%sWarning!%s duplicate package found for %s\n", CL_RED, CL_WHITE, lp.data.get_name()->c_str());
				FILE *duplog = fopen("dupes.log", "a");
				if (duplog)
				{
					fprintf(duplog, "%s%s\n", lp.data.get_locations()->at(0).get_full_url().c_str(), filename);
					fclose(duplog);
				}
			}
		}
		pkgDupeNames.push_back(*lp.data.get_name());
	}
	//printf("Normal return\n");
	return 0;
}


int countPackage(const char *filename, const struct stat *file_status, int filetype)
{
	
	//XMLNode tmpX;
	unsigned short x=0, y=0;

	if (file_status->st_ino!=0) x=y;

	mDebug("processing package "+ (string) filename);
       	string ext = getExtension(filename);

	if (filetype==FTW_F && ext=="tgz")
	{
		pkgcount_ftw++;
	}
	return 0;
}

int Repository::build_index(string server_url, string server_name, bool rebuild)
{
	if (rebuild) server_name=server_url;  // Temp warning workaround
	pkgDupeNames.clear();
	unlink("index.log");
	unlink("dupes.log");
	unlink("legacy.log");
	pkgcounter=0;
	
	__doc = xmlNewDoc((const xmlChar *)"1.0");
	if (__doc == NULL) {
		mDebug("_root == NULL");
	} else {
		mDebug("_root != NULL");
	}
	_rootNode = xmlNewNode(NULL, (const xmlChar *)"repository");
	if (_rootNode == NULL) {
		mDebug("_rootNode == NULL");
	} else {
		mDebug("_rootNode != NULL");
	}
	xmlDocSetRootElement(__doc, _rootNode);
	if (_rootNode == NULL) {
		mDebug("[2]_rootNode == NULL");
	} else {
		mDebug("[2]_rootNode != NULL");
	}

#ifdef DEBUG
	printf("OMFGDUMP\n");
	mDebug("Saving repo xml dump");
	FILE *__xmlDump = fopen("/tmp/xmldump-repo.xml", "w");
    if (xmlDocDump(__xmlDump, __doc) != -1) {
		fclose(__xmlDump);
		mDebug("Xml dump saved");
	} else {
		fclose(__xmlDump);
		mDebug("Xml dump failed");
	}
#endif
    	
	// Next, run thru files and extract data.
	// We consider that repository root is current directory. So, what we need to do:
	// Enter each sub-dir, get each file which name ends with .tgz, extracts xml (and other) data from them, 
	// and build an XML tree for whole repository, then write it to ./packages.xml
	
	
	// for the first, let's calculate the package count
	pkgcount_ftw=0;
	ftw(".", countPackage,600);
	ftw(".", ProcessPackage, 600);
	// Finally, write our XML tree to file
	xmlSaveFileEnc("packages.xml", __doc, "UTF-8");
	// Compress file
	mDebug("Compressing files");
	say("Compressing files\n");
	if (/*system("gzip -f filelist.xml")==0 && */system("gzip -f packages.xml")==0)
	       say("\n-------------SUMMARY------------------\nTotal: %d packages\n\nRepository index created successfully\n",pkgcounter);
	else mError("Error creating repository index!");
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
	mDebug("get_index!");
	// First: detecting repository type
	// Trying to download in this order (if successful, we have detected a repository type):
	// 1. packages.xml.gz 	(Native MOPSLinux)
	// 2. PACKAGES.TXT	(Legacy Slackware)
	// 3. Packages.gz	(Debian)
	// (and something else for RPM, in future)
	string index_filename = get_tmp_file();
	string md5sums_filename = get_tmp_file();
	if (!dialogMode) say("[%s] ...\r",server_url.c_str());
	string cm = "gunzip -f "+index_filename+".gz 2>/dev/null";
	if (type == TYPE_MPKG || type == TYPE_AUTO)
	{
		actionBus.getActionState(0);
		mDebug("trying MPKG, type = "+ IntToStr(type));
	       if (CommonGetFile(server_url + "packages.xml.gz", index_filename+".gz")==DOWNLOAD_OK)
		{
			actionBus.getActionState(0);
			mDebug("download ok, validating contents...");
			if (system(cm.c_str())==0 && \
					ReadFile(index_filename).find("<?xml version=\"1.0\"")!=std::string::npos && ReadFile(index_filename).find("<repository")!=std::string::npos)
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
		
		mDebug("trying SLACK, type = "+ IntToStr(type));
		if (CommonGetFile(server_url + "PACKAGES.TXT", index_filename)==DOWNLOAD_OK)
		{
			mDebug("download ok, validating contents...");
			if (ReadFile(index_filename).find("PACKAGE NAME:  ")!=std::string::npos)
			{
				currentStatus = _("Detected legacy Slackware repository");
				if (CommonGetFile(server_url + "CHECKSUMS.md5", md5sums_filename) == DOWNLOAD_OK)
				{
					type = TYPE_SLACK;
				}
				else 
				{
					mError(_("Error downloading checksums"));
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
		currentStatus = _("Error updating data from ") +server_url+_(": download error or unsupported type");
		mError(_("Error downloading package index: download error, or unsupported repository type"));
		return -1;
	}
	mDebug("Starting to parse index");
	PACKAGE *pkg = new PACKAGE;
	string xml_name=index_filename;
//	XMLNode *repository_root = new XMLNode;
	
	xmlDocPtr indexDoc;
	xmlNodePtr indexRootNode;
	
//	int pkg_count;
	int ret=0;
	currentStatus = "["+server_url+"] Importing data...";
	if (actionBus._abortActions)
	{
		actionBus._abortComplete=true;
		actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
		return MPKGERROR_ABORTED;
	}
	string *pList = new string;
	string *mList = new string;
	//XMLNode *tmp = new XMLNode;
	//xmlDocPtr indexDoc;
	//xmlNodePtr indexRootNode;
	switch(type)
	{
		case TYPE_MPKG:
			indexDoc = xmlReadFile(xml_name.c_str(), "UTF-8", 0);
			if (indexDoc == NULL) {
				xmlFreeDoc(indexDoc);
				mError("ппц...");
				return -1;
			}
			else mDebug("indexDoc read successfully");
			
			indexRootNode = xmlDocGetRootElement(indexDoc);
			if (indexRootNode == NULL) {
				mError(_("Failed to get index"));
				xmlFreeDoc(indexDoc);
			}
			else mDebug("indexRootNode read successfully");
			
			if (xmlStrcmp(indexRootNode->name, (const xmlChar *) "repository") ) {
				mError(_("Invalid index file"));
				xmlFreeDoc(indexDoc);
			}
			else mDebug("Found valid repository index");
			
			xmlXPathContextPtr xContext;
			xmlXPathObjectPtr xResult;
			
			xContext = xmlXPathNewContext(indexDoc);
			if (xContext == NULL) {
				mError("ппц");
			}
			
			xResult = xmlXPathEvalExpression((const xmlChar *)"/repository/package", xContext);
			if (xResult == NULL) {
				mError("XPath expression error");
			}
			
			if (xmlXPathNodeSetIsEmpty(xResult->nodesetval)) {
				xmlXPathFreeObject(xResult);

				printf(_("[%s] ... Nothing found\n"), server_url.c_str());
				//mError("No packages found");
				return 0;
			}
			
			xmlNodeSetPtr xNodeSet;
			int xi;
			
			actionBus.setActionProgress(ACTIONID_DBUPDATE, 0);
			
			xNodeSet = xResult->nodesetval;
			xmlXPathFreeContext(xContext);

			actionBus.setActionProgressMaximum(ACTIONID_DBUPDATE, xNodeSet->nodeNr);
			if (xNodeSet->nodeNr==0) printf("[%s] ... Nothing found", server_url.c_str());
			for (xi = 0; xi < xNodeSet->nodeNr; xi++) {
				printf("[%s] ... Importing received data: %d/%d\r",server_url.c_str(), xi+1, xNodeSet->nodeNr);
				actionBus.setActionProgress(ACTIONID_DBUPDATE, xi);
				mDebug("Processing " + IntToStr(xi) + " node");
				if (actionBus._abortActions) {
					actionBus._abortComplete = true;
					actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
					
					return MPKGERROR_ABORTED;
				}
				
				actionBus.setActionProgress(ACTIONID_DBUPDATE, xi);
				pkg->clear();
				mDebug("Calling xml2Package");
				if (xml2package(xNodeSet->nodeTab[xi], pkg)<0) {
					mError("Failed to parse");
					abort();
				}
				else mDebug("xml2package OK");
				// Adding location data
				pkg->get_locations()->at(0).set_server_url(&server_url);
				packages->add(pkg);
			}
			printf("\n");
			xmlCleanupMemory();
			xmlCleanupParser();
			/*
			
 				*repository_root=XMLNode::openFileHelper(xml_name.c_str(), "repository");
				pkg_count=repository_root->nChildNode("package");
				if (pkg_count==0)
				{
					mError("Repository has no packages\n");
					delete pkg;
					return 0;
				}
				
				actionBus.setActionProgress(ACTIONID_DBUPDATE, 0);
				for (int i=0; i<pkg_count; i++)
				{
					if (actionBus._abortActions)
					{
						actionBus._abortComplete=true;
						actionBus.setActionState(ACTIONID_DBUPDATE, ITEMSTATE_ABORTED);
						return MPKGERROR_ABORTED;
					}

					actionBus.setActionProgress(ACTIONID_DBUPDATE, i);
					pkg->clear();
					*tmp = repository_root->getChildNode("package", i);
					xml2package(tmp, pkg);
					// Adding location data
					pkg->get_locations()->at(0).set_server_url(&server_url);
					packages->add(pkg);
				}*/
				//delete tmp;
			break;
		case TYPE_SLACK:
			*pList = ReadFile(index_filename);
			*mList = ReadFile(md5sums_filename);

			ret = slackpackages2list(pList, mList, packages, server_url);
			if (pList!=NULL) delete pList;
			if (mList!=NULL) delete mList;
			break;

		case TYPE_DEBIAN:
			break;
		default:
			break;
	}
	delete pkg;
	return ret;
}


