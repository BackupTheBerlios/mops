/*
* XML parser of package config
* $Id: PackageConfig.cpp,v 1.8 2007/02/15 14:28:22 i27249 Exp $
*/

#include "PackageConfig.h"
#include "debug.h"
using namespace std;

//TODO: Add check for text!!!!!!!!!
PackageConfig::PackageConfig(string _f)
{
	this->fileName = _f;
	_node = XMLNode::openFileHelper(fileName.c_str(), "PACKAGE");
	debug("XML file opened");
}

PackageConfig::PackageConfig(XMLNode rootnode)
{
	_node = rootnode;
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

string PackageConfig::getDescription(string lang)
{
	if (_node.nChildNode("description")!=0)
	{
		if (lang.empty())
		{
			string a = (string )_node.getChildNode("description").getText();
			return a;
		}
		else {
			string a = (string)_node.getChildNodeWithAttribute("description", "lang", lang.c_str()).getText();
			return a;
		}
	}
	else return "";
}

string PackageConfig::getDescriptionI(int num)
{
	if (_node.nChildNode("description")>num)
	{
		string a = (string )_node.getChildNode("description", num).getText();
		return a;
	}
	else return "";
}


DESCRIPTION_LIST PackageConfig::getDescriptions()
{
	DESCRIPTION desc;
	DESCRIPTION_LIST descriptions;
	
	for (int i=0; i<_node.nChildNode("description"); i++)
	{
		if (_node.nChildNode("description")>i)
		{
			desc.set_text((string )_node.getChildNode("description", i).getText());
			if (_node.getChildNode("description",i).nAttribute()!=0)
			{
				desc.set_language((string)_node.getChildNode("description", i).getAttributeValue(0));
				desc.set_shorttext((string )_node.getChildNodeWithAttribute("short_description", "lang", desc.get_language().c_str()).getText());
			}
		}
		descriptions.add(desc);
	}
	return descriptions;
}
		
	

string PackageConfig::getShortDescription(string lang)
{
	if (_node.nChildNode("short_description")!=0)
	{
		if (lang.empty())
		{
			string a = (string )_node.getChildNode("short_description").getText();
			return a;
		}
		else {
			string a = (string)_node.getChildNodeWithAttribute("short_description", "lang", lang.c_str()).getText();
			return a;
		}
	}
	else return "";
}

string PackageConfig::getShortDescriptionI(int num)
{
	if (_node.nChildNode("short_description")>num)
	{
		string a = (string )_node.getChildNode("short_description", num).getText();
		return a;
	}
	else return "";
}

vector<string> PackageConfig::getShortDescriptions()
{
	vector<string> descriptions;
	for (int i=0; i<_node.nChildNode("short_description"); i++)
	{
		descriptions.push_back(getShortDescriptionI(i));
	}
	return descriptions;
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
	else return "any";
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
	else return "any";
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
		a_name.resize(_node.getChildNode("dependencies").nChildNode("dep"));
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
		a_cond.resize(_node.getChildNode("dependencies").nChildNode("dep"));
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
		a_ver.resize(_node.getChildNode("dependencies").nChildNode("dep"));
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
		a_name.resize(_node.getChildNode("suggests").nChildNode("suggest"));
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
		a_cond.resize(_node.getChildNode("suggests").nChildNode("suggest"));
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
		a_ver.resize(_node.getChildNode("suggests").nChildNode("suggest"));
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
		a.resize(_node.getChildNode("tags").nChildNode("tag"));
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

string PackageConfig::getConfigFile(int file_num)
{
	if (_node.nChildNode("configfiles")!=0 && _node.getChildNode("configfiles").nChildNode("conffile")>file_num)
	{
		string a = (string )_node.getChildNode("configfiles").getChildNode("conffile", file_num).getText();
		return a;
	}
	else return "";
}

vector<string> PackageConfig::getFilelist()
{
	vector<string> a;
	if(_node.nChildNode("filelist")!=0)
	{
		a.resize(_node.getChildNode("filelist").nChildNode("file"));
		for (unsigned int i=0;i<a.size();i++)
		{
			a[i]=getFile(i);
		}
	}
	return a;
}

vector<string> PackageConfig::getConfigFilelist()
{
	vector<string> a;
	if(_node.nChildNode("configfiles")!=0)
	{
		a.resize(_node.getChildNode("configfiles").nChildNode("conffile"));
		for (unsigned int i=0;i<a.size();i++)
		{
			a[i]=getConfigFile(i);
		}
	}
	return a;
}

XMLNode PackageConfig::getXMLNode()
{
	return _node;
}

string PackageConfig::getMd5()
{
	if (_node.nChildNode("md5")!=0)
	{
		string a = (string) _node.getChildNode("md5").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getCompressedSize()
{
	if (_node.nChildNode("compressed_size")!=0)
	{
		string a = (string) _node.getChildNode("compressed_size").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getInstalledSize()
{
	if (_node.nChildNode("installed_size")!=0)
	{
		string a = (string) _node.getChildNode("installed_size").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getFilename()
{
	if (_node.nChildNode("filename")!=0)
	{
		string a = (string) _node.getChildNode("filename").getText();
		return a;
	}
	else return "";
}

string PackageConfig::getLocation()
{
	if (_node.nChildNode("location")!=0)
	{
		string a = (string) _node.getChildNode("location").getText();
		return a;
	}
	else return "";
}
