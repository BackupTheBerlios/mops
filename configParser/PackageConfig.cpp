/**
 * $Id: PackageConfig.cpp,v 1.1 2006/12/15 09:40:40 adiakin Exp $
 */

#include "PackageConfig.h"
using namespace std;

PackageConfig::PackageConfig(string _f)
{
	this->fileName = _f;
	_node = XMLNode::openFileHelper(fileName.c_str(), "CONFIG");
	printf("ok!\n");
}

PackageConfig::~PackageConfig()
{
}

string PackageConfig::getName()
{
	printf("ok!!\n");
	string a = (string )_node.getChildNode("name").getText(); 		
	
	printf("ok!!!\n");
	return a;
}

string PackageConfig::getVersion()
{
	string a = (string )_node.getChildNode("version").getText();
	return a;
}

string PackageConfig::getAuthorName()
{
	string a = (string )_node.getChildNode("packager").getChildNode("name").getText();
	return a;
	
}

string PackageConfig::getAuthorEmail()
{
	string a =  (string )_node.getChildNode("packager").getChildNode("email").getText();
	return a;
}

string PackageConfig::getDescription()
{
	string a = (string )_node.getChildNode("description").getText();
	return a;
}

string PackageConfig::getShortDescription()
{
	string a = (string )_node.getChildNode("short_description").getText();
	return a;
}


