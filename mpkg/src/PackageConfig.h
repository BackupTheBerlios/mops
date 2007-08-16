/************************************************************
 *
 *	MOPSLinux packaging system
 *	XML parsing helper: reads XML, creates XML for
 *	packages and whole repository
 *
 *	$Id: PackageConfig.h,v 1.15 2007/08/16 14:39:10 i27249 Exp $
 *
 * **********************************************************/


// Package type: mpkg

#ifndef PACKAGECONFIG_H_
#define PACKAGECONFIG_H_

#include <string>
#include <vector>
#include <cassert>

#include "xmlParser.h"
#include "dataunits.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>
using namespace std;

#define XPATH_CTX_ERR (mDebug("Failed to create XPath context"))
#define XPATH_EVAL_ERR (mDebug("XPath eval error"))
#define EMPTY ""
#define EMPTYVECTOR ( new std::vector<std::string>() )

#define GET_PKG_NAME  ((const xmlChar *)"//package/name")
#define GET_PKG_VERSION ((const xmlChar *)"//package/version")
#define GET_PKG_ARCH ((const xmlChar *)"//package/arch")
#define GET_PKG_BUILD ((const xmlChar *)"//package/build")
#define GET_PKG_SHORT_DESCRIPTION ((const xmlChar *)"//package/short_description")
#define GET_PKG_DESCRIPTION ((const xmlChar *)"//package/description")
#define GET_PKG_MAINT_NAME ((const xmlChar *)"//package/maintainer/name")
#define GET_PKG_MAINT_EMAIL ((const xmlChar *)"//package/maintainer/email")
#define GET_PKG_LOCATION ((const xmlChar *)"//package/location")
#define GET_PKG_FILENAME ((const xmlChar *)"//package/filename")
#define GET_PKG_MD5 ((const xmlChar *)"//package/md5")
#define GET_PKG_CHANGELOG ((const xmlChar *)"/package/changelog")
#define GET_PKG_COMP_SIZE ((const xmlChar *)"//package/compressed_size")
#define GET_PKG_INST_SIZE ((const xmlChar *)"//package/installed_size")
#define GET_PKG_TAGS ((const xmlChar *)"//package/tags/tag")
#define GET_PKG_FILE_LIST ((const xmlChar *)"//package/files/file")
#define GET_PKG_SUG_COND ((const xmlChar *) "//suggests/suggest/condition")
#define GET_PKG_SUG_NAME ((const xmlChar *) "//suggests/suggest/name")
#define GET_PKG_SUG_VERSION ((const xmlChar *) "//suggests/suggest/version")
#define GET_PKG_CONFIG_FILE_LIST ((const xmlChar *)"//package/configfiles/conffile")
#define GET_PKG_DEP_NAME ((const xmlChar *)"//dependencies/dep/name")
#define GET_PKG_DEP_COND ((const xmlChar *)"//dependencies/dep/condition")
#define GET_PKG_DEP_VERSION ((const xmlChar *)"//dependencies/dep/version")

class PackageConfig
{
public:
	PackageConfig(string _f);
	PackageConfig(xmlNodePtr __rootXmlNodePtr);
	~PackageConfig();

	string getName(void);
	string getVersion(void);
	string getArch(void);
	string getBuild(void);
	string getAuthorName(void);
	string getAuthorEmail(void);
	string getPackageFileName(void);

	string getSuggestName(int suggest_num);
	string getSuggestCondition(int suggest_num);
	string getSuggestVersion(int suggest_num);


	string getDescription(string lang="");
	string getDescriptionI(int num=0);
	string getShortDescription(string lang="");
	string getShortDescriptionI(int num=0);

	string getDependencyName(int dep_num);
	string getDependencyCondition(int dep_num);
	string getDependencyVersion(int dep_num);
	string getTag(int tag_num);

	string getChangelog(void);

	string getFile(int file_num);
	string getConfigFile(int file_num);

	vector <string> getDepNames(void);
	vector <string> getDepConditions(void);
	vector <string> getDepVersions(void);
	vector <string> getSuggestNames(void);
	vector <string> getSuggestConditions(void);
	vector <string> getSuggestVersions(void);
	vector <string> getTags(void);
	vector <string> getFilelist(void);
	vector <string> getConfigFilelist(void);

	xmlNodePtr getXMLNode(void);
	xmlDocPtr getXMLDoc(void);

	// Repository-related functions
	string getMd5(void);
	string getCompressedSize(void);
	string getInstalledSize(void);
	string getFilename(void);
	string getLocation(void);
	std::string getXMLNodeEx();

	bool parseOk;

    bool hasErrors();

private:
	string fileName;
	XMLNode _node;
	XMLNode tmp;
    int errors;
    xmlDocPtr doc;
    xmlNodePtr curNode;

    xmlXPathObjectPtr getNodeSet(const xmlChar * exp);
};

#endif /*PACKAGECONFIG_H_*/
