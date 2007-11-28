/*
* XML parser of package config
* $Id: PackageConfig.cpp,v 1.42 2007/11/28 02:24:25 i27249 Exp $
*/
#include "file_routines.h"
#include "PackageConfig.h"
#include "debug.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>

using namespace std;

/**
 * 
 * @param _f path to file
 */
PackageConfig::PackageConfig(string _f)
{
	depCount=-1; suggestCount=-1;
    // new interface using libxml2
    this->errors = 0;

    
    //doc = xmlParseFile(_f.c_str());
	doc = xmlReadFile(_f.c_str(), "UTF-8", 0);
	
    if (doc == NULL) {
        mDebug("XML Load failed");
        this->errors++;
		this->parseOk = false;
        xmlFreeDoc(doc);
		doc=NULL;
		return;
    }

    curNode = xmlDocGetRootElement(doc);

    if (curNode == NULL) {
        mDebug("Failed to get root node");
        this->errors++;
		this->parseOk = false;
        xmlFreeDoc(doc);
		doc=NULL;
		return;
    } else {
		mDebug("CENSORED 00-5");
	}

    // checking for valid root node
    if (xmlStrcmp(curNode->name, (const xmlChar *) "package") ) {
        mDebug("Invalid root node definition");
        this->errors++;
        this->parseOk = false;
		xmlFreeDoc(doc);
		doc=NULL;
		return;

    } else {
		mDebug("CENSORED 00-6");
	}

	if (this->errors == 0) {
		this->parseOk = true;
		buildDepDef();
		buildSugDef();
	}

	if (this->doc == NULL) {
		mDebug("CENSORED 00-3");
	} else {
		mDebug("CENSORED 00-4");
	}

}

PackageConfig::PackageConfig(xmlChar * membuf, int bufsize)
{
	depCount = -1;
	suggestCount=-1;
    // new interface using libxml2
    this->errors = 0;

    
    //doc = xmlParseFile(_f.c_str());
	//doc = xmlReadFile(_f.c_str(), "UTF-8", 0);
	doc = xmlParseMemory((const char *) membuf, bufsize);
	xmlFree(membuf);
	
    if (doc == NULL) {
        mDebug("XML Load failed");
        this->errors++;
		this->parseOk = false;
        xmlFreeDoc(doc);
		doc=NULL;
		return;
    }

    curNode = xmlDocGetRootElement(doc);

    if (curNode == NULL) {
        mDebug("Failed to get root node");
        this->errors++;
		this->parseOk = false;
        xmlFreeDoc(doc);
		doc=NULL;
		return;
    } else {
		mDebug("CENSORED 00-5");
	}

    // checking for valid root node
    if (xmlStrcmp(curNode->name, (const xmlChar *) "package") ) {
        mDebug("Invalid root node definition");
        this->errors++;
        this->parseOk = false;
		xmlFreeDoc(doc);
		doc=NULL;
		return;

    } else {
		mDebug("CENSORED 00-6");
	}

	if (this->errors == 0) {
		this->parseOk = true;
		buildDepDef();
		buildSugDef();
	}

	if (this->doc == NULL) {
		mDebug("CENSORED 00-3");
	} else {
		mDebug("CENSORED 00-4");
	}
    

}

/**
 * check if we have any errors during parsing
 * 
 * @return bool state
 */
bool PackageConfig::hasErrors() {
    if (this->errors == 0) {
        return false;
    } else {
        return true;
    }
}

/**
 * WTF???
 * 
 * @param rootnode
 */
PackageConfig::PackageConfig(xmlNodePtr __rootXmlNodePtr)
{
	depCount = -1;
	suggestCount = -1;
	mDebug("INITIALIZATION");
	parseOk = true;
	//curNode = __rootXmlNodePtr;
	//curNode = xmlNewNode;
	*curNode=*__rootXmlNodePtr;
	this->doc=NULL;
	mDebug("Constructing XML elements");
	if (this->doc == NULL) {
		this->doc = xmlNewDoc((const xmlChar *)"1.0");
		mDebug("Created an empty XML doc");
		if (this->doc == NULL) {
			mDebug("in constructor doc == NULL");
			parseOk=false;
		}
		if (xmlDocSetRootElement(this->doc, this->curNode))
		{
			mDebug("INIT OK");
			if (this->doc==NULL) mDebug("OMGWTF!");
		}
		else {
			mDebug("INIT FAILED");
		//	parseOk=false;
		}
	}
	buildDepDef();
	buildSugDef();
}

PackageConfig::~PackageConfig()
{
	mDebug("Cleanup...");
    if (this->doc != NULL) {
        xmlFreeDoc(doc);
	doc=NULL;
	doc=NULL;
    }
    xmlCleanupMemory();
    mDebug("Cleanup complete");
}

/**
 * evaluate XPath expression and return result nodes
 * 
 * @param exp XPath expression
 * 
 * @return xmlXPathObjectPtr node set
 */
xmlXPathObjectPtr PackageConfig::getNodeSet(const xmlChar * exp) 
{
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
	
	mDebug("Creating context");
    	context = xmlXPathNewContext(doc);
    	if (context == NULL) 
	{
		mDebug("Failed to create context");
        	this->errors++;
        	XPATH_CTX_ERR;
        	return NULL;
    	}
    	else mDebug("Context created");

    	mDebug("Retrieving xpath, expression: " + (string) (const char *) exp);
    	result = xmlXPathEvalExpression(exp, context);
    	if (result == NULL) 
	{
        	(mDebug("XPath eval error"));
        	this->errors++;
        	return NULL;
    	} 
	else mDebug("XPath OK"); 

	if(xmlXPathNodeSetIsEmpty(result->nodesetval))
	{
        	xmlXPathFreeObject(result);
		xmlXPathFreeContext(context);
		mDebug("No result");
        	return NULL;
    	}
       	else 
	{
		xmlXPathFreeContext(context);
		mDebug("Returning result");
		return result;
	}
}

/**
 * get /package/name from xml
 * 
 * @return string
 */
string PackageConfig::getName()
{
	if (!pName.empty()) return pName;
	mDebug("Start");
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    mDebug("getNodeSet...");
    res = getNodeSet(GET_PKG_NAME);
    mDebug("getNodeSet complete");
    if (res) {
        mDebug("Retrieved nodeset");
        nodeset = res->nodesetval;

        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
		std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
		mDebug("NAME = '" + strim(__r) + "'");
		pName = strim(__r);
		return pName;
    } else {
        return EMPTY;
    }
}

/**
 * get /package/version from xml
 * 
 * @return string
 */
string PackageConfig::getVersion()
{
	if (!pVersion.empty()) return pVersion;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	res = getNodeSet(GET_PKG_VERSION);
	if (res) {
        
		nodeset = res->nodesetval;
		xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
		const char * _result = (const char * )key;
		std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
		mDebug("VERSION = '" +strim( __r) + "'");
		pVersion = strim(__r);
		return pVersion;
	} else {
		return EMPTY;
	}
}
string PackageConfig::getBetarelease()
{
	if (!pBetarelease.empty()) return pBetarelease;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	res = getNodeSet(GET_PKG_BETARELEASE);
	if (res) {

		nodeset = res->nodesetval;
		xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
		const char * _result = (const char * )key;
		std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
		mDebug("VERSION = '" +strim( __r) + "'");
		pBetarelease = strim(__r);
		return pBetarelease;
	} else {
		return EMPTY;
    }
}

string PackageConfig::getBuildConfigureEnvOptions()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_ENVOPTIONS);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD ENVOPTIONS = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}

}

string PackageConfig::getBuildUrl()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_URL);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD URL = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}
string PackageConfig::getBuildSourceRoot()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_SOURCEROOT);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD SOURCEROOT = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}
string PackageConfig::getBuildSystem()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_BUILDSYSTEM);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD BUILDSYSTEM = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}
string PackageConfig::getBuildMaxNumjobs()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_MAX_NUMJOBS);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD MAX_NUMJOBS = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}
bool PackageConfig::getBuildOptimizationCustomizable()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_OPTIMIZATION_ALLOW_CHANGE);
    	if (res) {
        
        	nodeset = res->nodesetval;

		xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD CAN_CUSTOMIZE = '" +strim( __r) + "'");
	        if (strim(__r)=="true") return true;
		else return false;
	    } else {
        	return false;
    	}
}
bool PackageConfig::getBuildNoSubfolder()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_NO_SUBFOLDER);
    	if (res) {
        
        	nodeset = res->nodesetval;

		xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD NO_SUBFOLDER = '" +strim( __r) + "'");
	        if (strim(__r)=="true") return true;
		else return false;
	    } else {
        	return false;
    	}
}

bool PackageConfig::getBuildUseCflags()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_USE_CFLAGS);
    	if (res) {
        
        	nodeset = res->nodesetval;

		xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD USE_CFLAGS = '" +strim( __r) + "'");
	        if (strim(__r)=="false") return false;
		else return true;
	    } else {
        	return true;
    	}
}

string PackageConfig::getBuildOptimizationMarch()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_OPTIMIZATION_MARCH);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD MARCH = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}

string PackageConfig::getBuildOptimizationMtune()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_OPTIMIZATION_MTUNE);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD MTUNE = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}

string PackageConfig::getBuildOptimizationLevel()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_OPTIMIZATION_LEVEL);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD LEVEL = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}

string PackageConfig::getBuildOptimizationCustomGccOptions()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_OPTIMIZATION_CUSTOM_GCC_OPTIONS);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD CUSTOM GCC OPTIONS = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}

string PackageConfig::getBuildCmdConfigure()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_CMD_CONFIGURE);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD CMD CONFIGURE = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}
string PackageConfig::getPackageType()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_TYPE);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("PKG TYPE = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}

string PackageConfig::getBuildCmdMake()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_CMD_MAKE);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD CMD MAKE = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}

string PackageConfig::getBuildCmdMakeInstall()
{
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
    	res = getNodeSet(GET_PKG_MBUILD_CMD_MAKEINSTALL);
    	if (res) {
        
        	nodeset = res->nodesetval;
	        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        	const char * _result = (const char * )key;
	        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        	mDebug("MBUILD CMD MAKE_INSTALL = '" +strim( __r) + "'");
	        return strim(__r);
	    } else {
        	return EMPTY;
    	}
}
vector<string> PackageConfig::getBuildPatchList()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_MBUILD_PATCH_LIST);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found patch name '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	} 
	return a;
}


vector<string> PackageConfig::getBuildKeyNames()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_MBUILD_CONFIGURATION_KEY_NAME);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found key name '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	} 
	return a;
}

vector<string> PackageConfig::getBuildKeyValues()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_MBUILD_CONFIGURATION_KEY_VALUE);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found key value '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	} 
	return a;
}



/**
 * return /package/arch
 * 
 * @return string
 */
string PackageConfig::getArch()
{
	xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_ARCH);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("ARCH = '" + strim(__r) + "'");

        return strim(__r);
    } else {
        return EMPTY;
    }
}

/**
 * return /package/build
 * 
 * @return string
 */
string PackageConfig::getBuild()
{
	if (!pBuild.empty()) return pBuild;
	xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_BUILD);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("BUILD= '" + strim(__r) + "'");
        //return (std::string)_result;
	pBuild = strim(__r);
	return pBuild;
	//	return "1";
    } else {
        return EMPTY;
    }
}

/**
 * return /package/maintainer/name
 * @return string
 */
string PackageConfig::getAuthorName()
{
	xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_MAINT_NAME);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("MNAME = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

/**
 * return /package/maintainer/email
 * 
 * @return string
 */
string PackageConfig::getAuthorEmail()
{
	xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_MAINT_EMAIL);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("MEMAIL = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

/**
 * return /package/changelog
 * 
 * @return string
 */
string PackageConfig::getChangelog()
{
	mDebug("Goning to fetch changelog");
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_CHANGELOG);
    if (res != NULL) {
        nodeset = res->nodesetval;

		xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
		
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("CHANGELOG = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

/**
 * return description
 * really return only 'en' description by
 * /package/description[@lang='en']
 * 
 * @param lang description language
 * 
 * @return string
 */
string PackageConfig::getDescription(string lang)
{
    if (!lang.empty()) say("warning: languaged descriptions disabled\n");
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_DESCRIPTION);
    if (res) {
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("DESCR = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

/**
 * return short description
 * really return only 'en' description by
 * /package/short_description[@lang='en']
 * 
 * @param lang description language
 * 
 * @return string
 */
string PackageConfig::getShortDescription(string lang)
{
	if (!lang.empty()) say("warning: languaged descriptions disabled\n");

	xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_SHORT_DESCRIPTION);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("SDESC = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

void PackageConfig::buildSugDef()
{
	suggestTreeDef.clear();
	xmlXPathObjectPtr res;
	string path;
	suggestCount=0;
	for (unsigned int i=1; ; i++) {
		//printf("Try %d\n", i);
		path = "//package/suggests/suggest[" + IntToStr(i)+"]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) {
			suggestCount++;
		//	printf("suggestCount = %d\n", suggestCount);
		}
		else break;
	}
//	printf("Received %d suggestions\n", suggestCount);
	suggestTreeDef.resize(suggestCount);
	for (int i=1; i<=suggestCount; i++) {
		path = "//suggests/suggest[" + IntToStr(i) + "]/name[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) {
			suggestTreeDef[i-1].name=true;
//			printf("found name in %d suggest\n", i);
		}
		else {
//			printf("NOT FOUND name in %d suggest\n", i);
			suggestTreeDef[i-1].name=false;
		}
	}
	for (int i=1; i<=suggestCount; i++) {
		path = "//suggests/suggest[" + IntToStr(i) + "]/condition[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) suggestTreeDef[i-1].condition=true;
		else suggestTreeDef[i-1].condition=false;
	}
	for (int i=1; i<=suggestCount; i++) {
		path = "//suggests/suggest[" + IntToStr(i) + "]/version[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) suggestTreeDef[i-1].version=true;
		else suggestTreeDef[i-1].version=false;
	}
	for (int i=1; i<=suggestCount; i++) {
		path = "//suggests/suggest[" + IntToStr(i) + "]/build_only[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) suggestTreeDef[i-1].build_only=true;
		else suggestTreeDef[i-1].build_only=false;
	}
	for (unsigned int i=0; i<suggestTreeDef.size(); i++)
	{
		//printf("[%d] %d %d %d %d\n", i, suggestTreeDef[i].name, suggestTreeDef[i].condition, suggestTreeDef[i].version, suggestTreeDef[i].build_only);
	}


}



void PackageConfig::buildDepDef()
{
	dependencyTreeDef.clear();
	xmlXPathObjectPtr res;
	string path;
	depCount=0;
	for (unsigned int i=1; ; i++) {
		//printf("Try %d\n", i);
		path = "//package/dependencies/dep[" + IntToStr(i)+"]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) {
			depCount++;
		//	printf("depCount = %d\n", depCount);
		}
		else break;
	}
//	printf("Received %d deps\n", depCount);
	dependencyTreeDef.resize(depCount);
	for (int i=1; i<=depCount; i++) {
		path = "//dependencies/dep[" + IntToStr(i) + "]/name[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) {
			dependencyTreeDef[i-1].name=true;
//			printf("found name if %d dependency\n", i);
		}
		else {
//			printf("NOT FOUND name in %d dependency\n", i);
			dependencyTreeDef[i-1].name=false;
		}
	}
	for (int i=1; i<=depCount; i++) {
		path = "//dependencies/dep[" + IntToStr(i) + "]/condition[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) dependencyTreeDef[i-1].condition=true;
		else dependencyTreeDef[i-1].condition=false;
	}
	for (int i=1; i<=depCount; i++) {
		path = "//dependencies/dep[" + IntToStr(i) + "]/version[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) dependencyTreeDef[i-1].version=true;
		else dependencyTreeDef[i-1].version=false;
	}
	for (int i=1; i<=depCount; i++) {
		path = "//dependencies/dep[" + IntToStr(i) + "]/build_only[1]";
		res = getNodeSet((const xmlChar *) path.c_str());
		if (res) dependencyTreeDef[i-1].build_only=true;
		else dependencyTreeDef[i-1].build_only=false;
	}
	for (unsigned int i=0; i<dependencyTreeDef.size(); i++)
	{
		//printf("[%d] %d %d %d %d\n", i, dependencyTreeDef[i].name, dependencyTreeDef[i].condition, dependencyTreeDef[i].version, dependencyTreeDef[i].build_only);
	}


}
vector<bool> PackageConfig::getDepBuildOnlyFlags()
{
	//printf("Getting buildonly flags, deoCount = %d\n",depCount);
	vector<bool> a;
	xmlNodeSetPtr nodeset;
    	xmlXPathObjectPtr res;
	xmlNodePtr nPtr;
    	res = getNodeSet(GET_PKG_DEP_BUILDONLY);
    	if (res) {
        
        	nodeset = res->nodesetval;
		for (int i=0; i<depCount; i++) {
			//printf("Try %d\n", i);
			if (!dependencyTreeDef[i].build_only) {
				//printf("zaglushka!\n");
				a.push_back(false); 
				continue;
			}
			//else printf("Hm... item exist!\n");

			nPtr = nodeset->nodeTab[0]->xmlChildrenNode;
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode, 1);

	        	const char * _result = (const char * )key;
		        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        		mDebug("DEP BUILD_ONLY = '" +strim( __r) + "'");
		        if (strim(__r)=="true") a.push_back(true);
			else a.push_back(false);
	    	}
        	return a;
    	}
	else {
		for (int i=0; i<depCount; i++)
		{
			a.push_back(false);
		}
	}
	return a;
}

vector<string> PackageConfig::getDepNames()
{
	//printf("getting names. depcount = %d\n", depCount);
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_DEP_NAME);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < depCount; i++) {
			if (!dependencyTreeDef[i].name) {
				a.push_back(""); 
				continue;
			}
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found dep name '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	}
	else {
		for (int i=0; i<depCount; i++)
		{
			a.push_back("");
		}
	}
	
	return a;
}
vector<string> PackageConfig::getDepConditions()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_DEP_COND);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < depCount; i++) {
			if (!dependencyTreeDef[i].condition) {
				a.push_back(""); 
				continue;
			}

			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : "OMGWTF!!!!";
			mDebug("Found dep cond '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	}
	else {
		for (int i=0; i<depCount; i++)
		{
			a.push_back("");
		}
	}

	return a;
}

vector<string> PackageConfig::getDepVersions()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_DEP_VERSION);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < depCount; i++) {
			if (!dependencyTreeDef[i].version) {
				a.push_back(""); 
				continue;
			}

			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found dep ver '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	} 
	else {
		for (int i=0; i<depCount; i++)
		{
			a.push_back("");
		}
	}

	return a;;
}

vector<string> PackageConfig::getSuggestNames()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_SUG_NAME);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < suggestCount; i++) {
			if (!suggestTreeDef[i].name) {
				a.push_back("");
				continue;
			}
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found suggest '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	}
	else {
		a.resize(suggestCount);
		return a;
	}
}
vector<string> PackageConfig::getSuggestConditions()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_SUG_COND);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < suggestCount; i++) {
			if (!suggestTreeDef[i].condition) {
				a.push_back("");
				continue;
			}

			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found suggest '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	}
	else {
		a.resize(suggestCount);
		return a;
	}
}

vector<string> PackageConfig::getSuggestVersions()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_SUG_VERSION);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < suggestCount; i++) {
			if (!suggestTreeDef[i].version) {
				a.push_back("");
				continue;
			}

			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found suggest '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	}
	else {
		a.resize(suggestCount);
	}
	return a;
}

/**
 * return tags /package/tags/tag
 * 
 * @return vector<string>
 */
vector<string> PackageConfig::getTags()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
	int i;

    res = getNodeSet(GET_PKG_TAGS);
    if (res) {
        
        nodeset = res->nodesetval;
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found tag '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
        return a;
    } 

	return a;
}

vector<string> PackageConfig::getFilelist()
{
	vector<string> a;

	return a;
}

vector<string> PackageConfig::getConfigFilelist()
{
	// SEEMS NOT TO WORK!!!!!!!!!!!!!!!!!
	vector<string> a;

	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_CONFIG_FILE_LIST);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found conf file '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	}


	return a;
}
vector<string> PackageConfig::getTempFilelist()
{
	vector<string> a;

	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_TEMP_FILE_LIST);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found temp file '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	}
	return a;
}

/**
 * NAHUI!!!!
 * 
 * @return XMLNode
 */
xmlNodePtr PackageConfig::getXMLNode()
{
	if (this->curNode == NULL) {
		mDebug("Root node == NULL");
	} else {
		mDebug("Root node != NULL");
		const xmlChar * __name = curNode->name;
		mDebug("CCCCCCCC __name = " +(string) (const char *)__name);
	}
	return this->curNode;
}

xmlChar * PackageConfig::getXMLNodeXPtr(int *bufsize)
{
	xmlChar *membuf;
	xmlDocDumpMemory(this->doc, &membuf, bufsize);
	return membuf;
}
	
/*
std::string PackageConfig::getXMLNodeEx() {
	printf("FUCK!\n");
	mDebug("Dumping the doc");
	FILE* __dump = fopen(TEMP_XML_DOC, "w");
	if (xmlDocDump(__dump, this->doc) == -1) {
		mDebug("Dump failed");
		fclose(__dump);
		abort();
	} else {
		fclose(__dump);
		mDebug("Dump OK");
		return TEMP_XML_DOC;
	}
}
*/
xmlDocPtr PackageConfig::getXMLDoc() 
{
	mDebug("Returning xmlDocPtr");
	if (this->doc == NULL) {
		mDebug("CENSORED 00-1");
	} else {
		mDebug("CENSORED 00-2");
	}
	return this->doc;
}

/**
 * return /package/md5
 * 
 * @return string
 */
string PackageConfig::getMd5()
{
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_MD5);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("MD5 = '" + strim(__r)+ "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

string PackageConfig::getCompressedSize()
{
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_COMP_SIZE);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("CSIZE = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

string PackageConfig::getInstalledSize()
{
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_INST_SIZE);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("ISIZE = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

string PackageConfig::getFilename()
{
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_FILENAME);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("FILENAME = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}

string PackageConfig::getLocation()
{
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_LOCATION);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("LOCATION = '" + strim(__r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
}
