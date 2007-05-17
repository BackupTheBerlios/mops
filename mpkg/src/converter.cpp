/******************************************************
 * Data converter for legacy Slackware packages
 * $Id: converter.cpp,v 1.8 2007/05/17 15:12:36 i27249 Exp $
 * ***************************************************/

#include "converter.h"

int slack_convert(string filename, string xml_output)
{
	PACKAGE package;
	package.set_filename(filename);
	// Resolving name, version, arch and build
	string tmp;
	string tmp_xml = get_tmp_file();
	extractFromTgz(filename, "install/data.xml", tmp_xml);
	if (FileNotEmpty(tmp_xml))
	{
		WriteFile(xml_output, ReadFile(tmp_xml));
		return 0;
	}
	int pos=0;
	// NAME
	int name_start=0;
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
				package.set_name(tmp);
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
			package.set_version(tmp);
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
			package.set_arch(tmp);
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
	package.set_build(tmp);

	tmp.clear();
#define DESCRIPTION_PROCESS
#ifdef DESCRIPTION_PROCESS	
	//DESCRIPTION
	string desc_file=get_tmp_file();
	string desc="tar zxf "+filename+" install/slack-desc --to-stdout > "+desc_file;
	if (system(desc.c_str())==0)
	{
		string description=ReadFile(desc_file);
		// Step 1. Skip comments
		unsigned int dpos=0;
		//unsigned int strLen=0;
		string comment;
		string head;
		string short_description;
		//bool str1=true;
		if (!description.empty())
		{
			// Cleaning out comments
			for (unsigned int i=0; i<description.length(); i++)
			{
				if (description[i]=='#')
				{
					while (description[i]!='\n') i++;
				}
				if (i<description.length()) tmp+=description[i];
			}
			description=tmp;
			tmp.clear();
			string pHead=package.get_name()+":";
			int spos=description.find(pHead,0);
			// Removing package: headers
			for (unsigned int i=spos; i<description.length(); i++)
			{
				//head+=description[i];
				if (i==0 || description[i-1]=='\n')
				{
					i=i+package.get_name().length()+1;
					//if (description[i-1]=='\n') i=i+package.get_name().length()+2;
					//head.clear();
				}
				if (i<description.length()) tmp+=description[i];
			}
			description=tmp;
			tmp.clear();
	
			// Splitting short and long descriptions
			for (unsigned int i=0; i<description.length() && description[i]!='\n'; i++)
			{
				tmp+=description[i];
				dpos=i+1;
			}
			short_description=tmp;
			tmp.clear();
			for (unsigned int i=dpos; i<description.length(); i++)
			{
				if (i==dpos && description[i]=='\n')
				{
					while (description[i]=='\n' || description[i]==' ') i++;
					if (i>=description.length()) break;
				}
				if (i<description.length()) tmp+=description[i];
			}
			description=tmp;
			tmp.clear();
			package.set_short_description(short_description);
			package.set_description(description);
		}
	}
#endif
	XMLNode pkg=XMLNode::createXMLTopNode("package");
	pkg.addChild("name");
	pkg.getChildNode("name").addText(package.get_name().c_str());
	pkg.addChild("version");
	pkg.getChildNode("version").addText(package.get_version().c_str());
	pkg.addChild("arch");
	pkg.getChildNode("arch").addText(package.get_arch().c_str());
	pkg.addChild("build");
	pkg.getChildNode("build").addText(package.get_build().c_str());
#ifdef DESCRIPTION_PROCESS
	pkg.addChild("short_description");
	pkg.getChildNode("short_description").addText(package.get_short_description().c_str());
	pkg.addChild("description");
	pkg.getChildNode("description").addText(package.get_description().c_str());
#endif
	pkg.addChild("tags");
	pkg.getChildNode("tags").addChild("tag");
	pkg.getChildNode("tags").getChildNode("tag").addText("slackware");
	pkg.addChild("maintainer");
	pkg.getChildNode("maintainer").addChild("name");
	pkg.getChildNode("maintainer").getChildNode("name").addText("Slackware to MPKG autoconverter by AiX");
	pkg.getChildNode("maintainer").addChild("email");
	pkg.getChildNode("maintainer").getChildNode("email").addText("i27249@gmail.com");
	pkg.writeToFile(xml_output.c_str());
	return 0;
}

int convert_package(string filename, string output_dir)
{
//	char tmp[1000];
	say("converting package %s\n", filename.c_str());
	int name_start=0;
	for (int i=filename.length()-1; filename[i]!='/' && i>=0; i--)
	{
		name_start=i;
	}
	string real_filename;
	for (unsigned int i=name_start; i<filename.length(); i++)
	{
		real_filename+=filename[i];
	}

	string tmp_dir=get_tmp_file();
	string xml_output=tmp_dir+"/install/data.xml";
	string reasm="rm "+tmp_dir+" && mkdir "+tmp_dir+" "+/*"2>/dev/null"+*/" && mkdir "+tmp_dir+"/install 2>/dev/null && cp "+filename+" "+tmp_dir+"/ && cd "+tmp_dir+" &&  tar zxf "+real_filename+" > /dev/null";
	system(reasm.c_str());	
	slack_convert(filename, xml_output);
	reasm="cd "+tmp_dir+" && rm "+real_filename+" && makepkg -l n -c n "+real_filename+" 2&>/dev/null &&  mv *.tgz "+output_dir+" && rm -rf "+tmp_dir;
	system(reasm.c_str());
	return 0;
}


int tag_package(string filename, string tag)
{
	string run = "mkdir -p "+filename.substr(0,filename.length()-4)+" && tar zxf " + filename + " -C " + filename.substr(0,filename.length()-4);
	if (system(run.c_str())!=0) return -1; // Extracting
	mDebug("extracting complete");
	
	string xml_path = filename.substr(0,filename.length()-4) + "/install/data.xml";
	if (!FileExists(xml_path)) return -2;
	XMLResults xmlErrCode;
	XMLNode _node = XMLNode::parseFile(xml_path.c_str(), "package", &xmlErrCode);
	if (xmlErrCode.error != eXMLErrorNone)
	{
		mError("parse error");
		return -1;
	}
	mDebug("File opened");
	if (_node.nChildNode("tags")==0)
	{
		_node.addChild("tags");
	}
	unsigned int tag_id = _node.getChildNode("tags").nChildNode("tag");
	_node.getChildNode("tags").addChild("tag");
	_node.getChildNode("tags").getChildNode("tag",tag_id).addText(tag.c_str());
	_node.writeToFile(xml_path.c_str());

	run = "cd "+filename.substr(0,filename.length()-4) + \
	      " && makepkg -l y -c n " \
	      + _node.getChildNode("name").getText() + "-"\
	      + _node.getChildNode("version").getText() + "-"\
	      + _node.getChildNode("arch").getText() + "-"\
	      + _node.getChildNode("build").getText() + ".tgz"\
	      + " && mv "\
	      + _node.getChildNode("name").getText() + "-"\
	      + _node.getChildNode("version").getText() + "-"\
	      + _node.getChildNode("arch").getText() + "-"\
	      + _node.getChildNode("build").getText() + ".tgz .. && cd ..";
	if (system(run.c_str())!=0) return -3; // Packing back;
	run = "rm -rf " + filename.substr(0,filename.length()-4);
	system(run.c_str());
	return 0;
}

