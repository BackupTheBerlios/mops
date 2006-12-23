/************************************************************
 *
 *	MOPSLinux packaging system
 *	XML parsing helper: reads XML, creates XML for
 *	packages and whole repository
 *
 *	$Id: PackageConfig.h,v 1.3 2006/12/23 11:42:06 i27249 Exp $
 *
 * **********************************************************/




#ifndef PACKAGECONFIG_H_
#define PACKAGECONFIG_H_

#include <string>
#include <vector>
#include <cassert>

#include "xmlParser.h"
using namespace std;

class PackageConfig
{
public:
	PackageConfig(string _f);
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

	string getDescription(void);
	string getShortDescription(void);

	string getDependencyName(int dep_num);
	string getDependencyCondition(int dep_num);
	string getDependencyVersion(int dep_num);
	string getTag(int tag_num);

	string getChangelog(void);

	string getFile(int file_num);

	vector <string> getDepNames(void);
	vector <string> getDepConditions(void);
	vector <string> getDepVersions(void);
	vector <string> getSuggestNames(void);
	vector <string> getSuggestConditions(void);
	vector <string> getSuggestVersions(void);
	vector <string> getTags(void);
	vector <string> getFilelist(void);

	XMLNode getXMLNode(void);

	
private:
	string fileName;
	XMLNode _node;
	XMLNode tmp;
};

#endif /*PACKAGECONFIG_H_*/
