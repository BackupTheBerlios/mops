/*
* XML parser of package config
* $Id: PackageConfig.cpp,v 1.2 2006/12/17 19:34:57 i27249 Exp $
*/

#include "PackageConfig.h"
#include "debug.h"
using namespace std;

PackageConfig::PackageConfig(string _f)
{
	this->fileName = _f;
	_node = XMLNode::openFileHelper(fileName.c_str(), "PACKAGE");
	debug("XML file opened");
}

PackageConfig::~PackageConfig()
{
}

string PackageConfig::getName()
{
	if (_node.nChildNode("name")!=0)
	{
		string a = (string )_node.getChildNode("name").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getVersion()
{
	if (_node.nChildNode("version")!=0)
	{
		string a = (string )_node.getChildNode("version").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getArch()
{
	if (_node.nChildNode("arch")!=0)
	{
		string a = (string )_node.getChildNode("arch").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getBuild()
{
	if (_node.nChildNode("build")!=0)
	{
		string a = (string )_node.getChildNode("build").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getAuthorName()
{
	if (_node.nChildNode("maintainer")!=0 && _node.getChildNode("maintainer").nChildNode("name")!=0)
	{
		string a = (string )_node.getChildNode("maintainer").getChildNode("name").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getAuthorEmail()
{
	if (_node.nChildNode("maintainer")!=0 && _node.getChildNode("maintainer").nChildNode("email")!=0)
	{
		string a =  (string )_node.getChildNode("maintainer").getChildNode("email").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getChangelog()
{
	if (_node.nChildNode("changelog")!=0)
	{
		string a = (string )_node.getChildNode("changelog").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getDescription()
{
	if (_node.nChildNode("description")!=0)
	{
		string a = (string )_node.getChildNode("description").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getShortDescription()
{
	if (_node.nChildNode("short_description")!=0)
	{
		string a = (string )_node.getChildNode("short_description").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getDependencyName(int dep_num)
{
	if (_node.nChildNode("dependencies")!=0 && _node.getChildNode("dependencies").nChildNode("dep")>dep_num && \
			_node.getChildNode("dependencies").getChildNode("dep", dep_num).nChildNode("name")!=0)
	{
		string a = (string )_node.getChildNode("dependencies").getChildNode("dep", dep_num).getChildNode("name").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getDependencyCondition(int dep_num)
{
	if (_node.nChildNode("dependencies")!=0 && _node.getChildNode("dependencies").nChildNode("dep")>dep_num && \
			_node.getChildNode("dependencies").getChildNode("dep", dep_num).nChildNode("condition")!=0)
	{
		string a = (string )_node.getChildNode("dependencies").getChildNode("dep", dep_num).getChildNode("condition").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getDependencyVersion(int dep_num)
{
	if (_node.nChildNode("dependencies")!=0 && _node.getChildNode("dependencies").nChildNode("dep")>dep_num && \
			_node.getChildNode("dependencies").getChildNode("dep", dep_num).nChildNode("version")!=0)
	{
		string a = (string )_node.getChildNode("dependencies").getChildNode("dep", dep_num).getChildNode("version").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getSuggestName(int suggest_num)
{
	if (_node.nChildNode("suggests")!=0 && _node.getChildNode("suggests").nChildNode("suggest")>suggest_num && \
			_node.getChildNode("suggests").getChildNode("suggest", suggest_num).nChildNode("name")!=0)
	{
		string a = (string )_node.getChildNode("suggests").getChildNode("suggest", suggest_num).getChildNode("name").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getSuggestCondition(int suggest_num)
{
	if (_node.nChildNode("suggests")!=0 && _node.getChildNode("suggests").nChildNode("suggest")>suggest_num && \
			_node.getChildNode("suggests").getChildNode("suggest", suggest_num).nChildNode("condition")!=0)
	{
		string a = (string )_node.getChildNode("suggests").getChildNode("suggest", suggest_num).getChildNode("condition").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getSuggestVersion(int suggest_num)
{
	if (_node.nChildNode("suggests")!=0 && _node.getChildNode("suggests").nChildNode("suggest")>suggest_num && \
			_node.getChildNode("suggests").getChildNode("suggest", suggest_num).nChildNode("version")!=0)
	{
		string a = (string )_node.getChildNode("suggests").getChildNode("suggest", suggest_num).getChildNode("version").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getTag(int tag_num)
{
	if (_node.nChildNode("tags")!=0 && _node.getChildNode("tags").nChildNode("tag")>tag_num)
	{
		string a = (string )_node.getChildNode("tags").getChildNode("tag", tag_num).getText();
		return a;
	}
	else return "";
}

vector<string> PackageConfig::getDepNames()
{
	vector<string> a_name;
	if (_node.nChildNode("dependencies")!=0)
	{
		a_name.resize(_node.getChildNode("dependencies").nChildNode());
		for (unsigned int i=0;i<a_name.size();i++)
		{
			a_name[i]=getDependencyName(i);
		}
	}
	return a_name;

}
vector<string> PackageConfig::getDepConditions()
{
	vector<string> a_cond;
	if(_node.nChildNode("dependencies")!=0)
	{
		a_cond.resize(_node.getChildNode("dependencies").nChildNode());
		for (unsigned int i=0;i<a_cond.size();i++)
		{
			a_cond[i]=getDependencyCondition(i);
		}
	}
	return a_cond;
}

vector<string> PackageConfig::getDepVersions()
{
	vector<string> a_ver;
	if(_node.nChildNode("dependencies")!=0)
	{
		a_ver.resize(_node.getChildNode("dependencies").nChildNode());
		for (unsigned int i=0;i<a_ver.size();i++)
		{
			a_ver[i]=getDependencyVersion(i);
		}
	}
	return a_ver;
}

vector<string> PackageConfig::getSuggestNames()
{
	vector<string> a_name;
	if(_node.nChildNode("suggests")!=0)
	{
		a_name.resize(_node.getChildNode("suggests").nChildNode());
		for (unsigned int i=0;i<a_name.size();i++)
		{
			a_name[i]=getSuggestName(i);
		}
	}
	return a_name;
}
vector<string> PackageConfig::getSuggestConditions()
{
	vector<string> a_cond;
	if(_node.nChildNode("suggests")!=0)
	{
		a_cond.resize(_node.getChildNode("suggests").nChildNode());
		for (unsigned int i=0;i<a_cond.size();i++)
		{
			a_cond[i]=getSuggestCondition(i);
		}
	}
	return a_cond;
}

vector<string> PackageConfig::getSuggestVersions()
{
	vector<string> a_ver;
	if(_node.nChildNode("suggests")!=0)
	{
		a_ver.resize(_node.getChildNode("suggests").nChildNode());
		for (unsigned int i=0;i<a_ver.size();i++)
		{
			a_ver[i]=getSuggestVersion(i);
		}
	}
	return a_ver;
}

vector<string> PackageConfig::getTags()
{
	vector<string> a;
	if(_node.nChildNode("tags")!=0)
	{
		a.resize(_node.getChildNode("tags").nChildNode());
		for (unsigned int i=0;i<a.size();i++)
		{
			a[i]=getTag(i);
		}
	}
	return a;
}

string PackageConfig::getFile(int file_num)
{
	if (_node.nChildNode("filelist")!=0 && _node.getChildNode("filelist").nChildNode("file")>file_num)
	{
		string a = (string )_node.getChildNode("filelist").getChildNode("file", file_num).getText();
		return a;
	}
	else return "";
}

vector<string> PackageConfig::getFilelist()
{
	vector<string> a;
	if(_node.nChildNode("filelist")!=0)
	{
		a.resize(_node.getChildNode("filelist").nChildNode());
		for (unsigned int i=0;i<a.size();i++)
		{
			a[i]=getFile(i);
		}
	}
	return a;
}


