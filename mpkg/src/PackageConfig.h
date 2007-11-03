/************************************************************
 *
 *	MOPSLinux packaging system
 *	XML parsing helper: reads XML, creates XML for
 *	packages and whole repository
 *
 *	$Id: PackageConfig.h,v 1.22 2007/11/03 01:08:15 i27249 Exp $
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

typedef struct depTree { bool name, version, condition, build_only; } ;

#define XPATH_CTX_ERR (mDebug("Failed to create XPath context"))
#define XPATH_EVAL_ERR (mDebug("XPath eval error"))
#define EMPTY ""
#define EMPTYVECTOR ( new std::vector<std::string>() )


// Binary package XML paths
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
#define GET_PKG_BETARELEASE ((const xmlChar *)"//package/betarelease")
#define GET_PKG_MD5 ((const xmlChar *)"//package/md5")
#define GET_PKG_CHANGELOG ((const xmlChar *)"//package/changelog")
#define GET_PKG_COMP_SIZE ((const xmlChar *)"//package/compressed_size")
#define GET_PKG_INST_SIZE ((const xmlChar *)"//package/installed_size")
#define GET_PKG_TAGS ((const xmlChar *)"//package/tags/tag")
#define GET_PKG_TYPE ((const xmlChar *)"//package/type")
#define GET_PKG_FILE_LIST ((const xmlChar *)"//package/files/file")
#define GET_PKG_SUG ((const xmlChar *) "//suggests/suggest")
#define GET_PKG_SUG_COND ((const xmlChar *) "//suggests/suggest/condition")
#define GET_PKG_SUG_NAME ((const xmlChar *) "//suggests/suggest/name")
#define GET_PKG_SUG_VERSION ((const xmlChar *) "//suggests/suggest/version")
#define GET_PKG_CONFIG_FILE_LIST ((const xmlChar *)"//package/configfiles/conffile")
#define GET_PKG_TEMP_FILE_LIST ((const xmlChar *)"//package/tempfiles/tempfile")
#define GET_PKG_DEP ((const xmlChar *)"//package/dependencies/dep")
#define GET_PKG_DEP_NAME ((const xmlChar *)"//dependencies/dep/name")
#define GET_PKG_DEP_COND ((const xmlChar *)"//dependencies/dep/condition")
#define GET_PKG_DEP_VERSION ((const xmlChar *)"//dependencies/dep/version")
#define GET_PKG_DEP_BUILDONLY ((const xmlChar *)"//dependencies/dep/build_only")

#define GET_PKG_ENUM_DEP_NAME ((const xmlChar *)"//dependencies/dep[name]")
#define GET_PKG_ENUM_DEP_VERSION ((const xmlChar *)"//dependencies/dep[version]")
#define GET_PKG_ENUM_DEP_COND ((const xmlChar *)"//dependencies/dep[condition]")
#define GET_PKG_ENUM_DEP_BUILDONLY ((const xmlChar *)"//dependencies/dep[build_only]")

// MPKG-SRC extensions
#define GET_PKG_MBUILD_URL ((const xmlChar *)"//mbuild/url")
#define GET_PKG_MBUILD_PATCH_LIST ((const xmlChar *)"//mbuild/patches/patch")
#define GET_PKG_MBUILD_SOURCEROOT ((const xmlChar *)"//mbuild/sources_root_directory")
#define GET_PKG_MBUILD_BUILDSYSTEM ((const xmlChar *)"//mbuild/build_system")
#define GET_PKG_MBUILD_MAX_NUMJOBS ((const xmlChar *)"//mbuild/max_numjobs")
#define GET_PKG_MBUILD_OPTIMIZATION_ALLOW_CHANGE ((const xmlChar *)"//mbuild/optimization/allow_change")
#define GET_PKG_MBUILD_OPTIMIZATION_MARCH ((const xmlChar *)"//mbuild/optimization/march")
#define GET_PKG_MBUILD_OPTIMIZATION_MTUNE ((const xmlChar *)"//mbuild/optimization/mtune")
#define GET_PKG_MBUILD_OPTIMIZATION_LEVEL ((const xmlChar *)"//mbuild/optimization/olevel")
#define GET_PKG_MBUILD_OPTIMIZATION_CUSTOM_GCC_OPTIONS ((const xmlChar *)"//mbuild/optimization/custom_gcc_options")
#define GET_PKG_MBUILD_CONFIGURATION_KEY_NAME ((const xmlChar *)"//mbuild/configuration/key/name")
#define GET_PKG_MBUILD_CONFIGURATION_KEY_VALUE ((const xmlChar *)"//mbuild/configuration/key/value")

#define GET_PKG_MBUILD_CMD_CONFIGURE ((const xmlChar *)"//mbuild/custom_commands/configure")
#define GET_PKG_MBUILD_CMD_MAKE ((const xmlChar *)"//mbuild/custom_commands/make")
#define GET_PKG_MBUILD_CMD_MAKEINSTALL ((const xmlChar *)"//mbuild/custom_commands/make_install")

#define GET_PKG_MBUILD_ENVOPTIONS ((const xmlChar *)"//mbuild/env_options")

class PackageConfig
{
public:
	PackageConfig(string _f);
	PackageConfig(xmlChar * membuf, int bufsize);

	PackageConfig(xmlNodePtr __rootXmlNodePtr);
	~PackageConfig();
	void buildDepDef(void);
	void buildSugDef(void);
	string getName(void);
	string getVersion(void);
	string getBetarelease(void);
	string getArch(void);
	string getBuild(void);
	string getAuthorName(void);
	string getAuthorEmail(void);
	string getPackageFileName(void);
	string getPackageType(void);

	string getSuggestName(int suggest_num);
	string getSuggestCondition(int suggest_num);
	string getSuggestVersion(int suggest_num);


	string getDescription(string lang="");
	string getDescriptionI(int num=0);
	string getShortDescription(string lang="");
	string getShortDescriptionI(int num=0);

//	string getDependencyName(int dep_num);
//	string getDependencyCondition(int dep_num);
//	string getDependencyVersion(int dep_num);
	//string getTag(int tag_num);

	string getChangelog(void);

	//string getFile(int file_num);
	//string getConfigFile(int file_num);
	vector <string> getDepNames(void);
	vector <string> getDepConditions(void);
	vector <string> getDepVersions(void);
	vector <bool> getDepBuildOnlyFlags(void);
	vector <string> getSuggestNames(void);
	vector <string> getSuggestConditions(void);
	vector <string> getSuggestVersions(void);
	vector <string> getTags(void);
	vector <string> getFilelist(void);
	vector <string> getConfigFilelist(void);
	vector <string> getTempFilelist(void);
	
	string getBuildUrl(void);
	string getBuildSourceRoot(void);
	string getBuildSystem(void);
	string getBuildMaxNumjobs(void);
	bool getBuildOptimizationCustomizable(void);
	string getBuildOptimizationMarch(void);
	string getBuildOptimizationMtune(void);
	string getBuildOptimizationLevel(void);
	string getBuildOptimizationCustomGccOptions(void);
	string getBuildConfigureEnvOptions(void);
	string getBuildCmdConfigure(void);
	string getBuildCmdMake(void);
	string getBuildCmdMakeInstall(void);
	vector<string> getBuildKeyNames(void);
	vector<string> getBuildKeyValues(void);
	vector<string> getBuildPatchList(void);

	xmlNodePtr getXMLNode(void);
	xmlDocPtr getXMLDoc(void);

	// Repository-related functions
	string getMd5(void);
	string getCompressedSize(void);
	string getInstalledSize(void);
	string getFilename(void);
	string getLocation(void);
	xmlChar * getXMLNodeXPtr(int * bufsize);
	//std::string getXMLNodeEx();

	bool parseOk;

	bool hasErrors();

private:
	vector<depTree> dependencyTreeDef, suggestTreeDef;
	string fileName;
	XMLNode _node;
	XMLNode tmp;
    int errors;
    int depCount, suggestCount;
    xmlDocPtr doc;
    xmlNodePtr curNode;

    xmlXPathObjectPtr getNodeSet(const xmlChar * exp);
};

#endif /*PACKAGECONFIG_H_*/
