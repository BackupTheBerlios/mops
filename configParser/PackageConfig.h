/**
 * $Id: PackageConfig.h,v 1.1 2006/12/15 09:40:40 adiakin Exp $
 */


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
	string getAuthorName(void);
	string getAuthorEmail(void);
	string getPackageFileName(void);
	string getDescription(void);
	string getShortDescription(void);
	
	//vector<string> getDeps();
	
private:
	string fileName;
	XMLNode _node;
	XMLNode tmp;
};

#endif /*PACKAGECONFIG_H_*/
