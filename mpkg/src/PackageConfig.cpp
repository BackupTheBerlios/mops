/*
* XML parser of package config
* $Id: PackageConfig.cpp,v 1.29 2007/08/16 14:39:10 i27249 Exp $
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
    // new interface using libxml2
    this->errors = 0;

    
    //doc = xmlParseFile(_f.c_str());
	doc = xmlReadFile(_f.c_str(), "UTF-8", NULL);
	
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
		mDebug("PIZDEC 00-5");
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
		mDebug("PIZDEC 00-6");
	}

	if (this->errors == 0) {
		this->parseOk = true;
	}

	if (this->doc == NULL) {
		mDebug("PIZDEC 00-3");
	} else {
		mDebug("PIZDEC 00-4");
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
		return strim(__r);
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
	xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr res;
    res = getNodeSet(GET_PKG_VERSION);
    if (res) {
        
        nodeset = res->nodesetval;
        xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[0]->xmlChildrenNode,1);
        const char * _result = (const char * )key;
        std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
        mDebug("VERSION = '" +strim( __r) + "'");
        return strim(__r);
    } else {
        return EMPTY;
    }
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
	return strim(__r);
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


vector<string> PackageConfig::getDepNames()
{
	vector<string> a;
	xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr res;
	int i;

	res = getNodeSet(GET_PKG_DEP_NAME);
	if (res) {

		nodeset = res->nodesetval;
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found dep name '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
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
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : "PIZZDECCC!!!!";
			mDebug("Found dep cond '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
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
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found dep ver '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
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
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found suggest '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	} 
	return a;;
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
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found suggest '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
	} 
	return a;
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
		for (i = 0; i < nodeset->nodeNr; i++) {
			xmlChar * key = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode,1);
			const char * _result = (const char * )key;
			std::string __r = (_result != NULL) ? ((std::string)_result) : EMPTY;
			mDebug("Found suggest '" + strim(__r) + "'");
			a.push_back(strim(__r));
		}
		return a;
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
std::string PackageConfig::getXMLNodeEx() {
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

xmlDocPtr PackageConfig::getXMLDoc() 
{
	mDebug("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA Returning xmlDocPtr");
	if (this->doc == NULL) {
		mDebug("PIZDEC 00-1");
	} else {
		mDebug("PIZDEC 00-2");
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
