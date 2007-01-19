/******************************************************
 * Data converter for legacy Slackware packages
 * $Id: converter.cpp,v 1.1 2007/01/19 06:11:04 i27249 Exp $
 * ***************************************************/

#include "converter.h"

int slack_convert(string filename, string xml_output)
{
	printf(".");
	PACKAGE package;
	package.set_filename(filename);
	// Resolving name, version, arch and build
	string tmp;
	int pos;
	// NAME
	int name_start;
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
//				printf("package name: %s\n", package.get_name().c_str());
				pos=i+2;
				break;
			}
		}
		tmp+=filename[i];
	}
	tmp.clear();
	//VERSION
	printf(".");
	for (unsigned int i=pos-1; i< filename.length(); i++)
	{
		if (filename[i]=='-')
		{
			package.set_version(tmp);
//			printf("package version: %s\n", package.get_version().c_str());
			pos=i+2;
			break;
		}
		tmp+=filename[i];
	}
	tmp.clear();
	//ARCH
	printf(".");
	for (unsigned int i=pos-1; i< filename.length(); i++)
	{
		if (filename[i]=='-')
		{
			package.set_arch(tmp);
//			printf("package arch: %s\n", package.get_arch().c_str());
			pos=i+2;
			break;
		}
		tmp+=filename[i];
	}
	tmp.clear();
	printf(".");
	//BUILD
	for (unsigned int i=pos-1; i<filename.length()-4; i++)
	{
		tmp+=filename[i];
	}
	package.set_build(tmp);
//	printf("package build: %s\n", package.get_build().c_str());

	tmp.clear();
#ifdef DESCRIPTION_PROCESS	
	//DESCRIPTION
	string desc_file=get_tmp_file();
	string desc="tar zxf "+filename+" install/slack-desc --to-stdout > "+desc_file;
	system(desc.c_str());
	string description=ReadFile(desc_file);
	// Step 1. Skip comments
	unsigned int dpos=0;
	string comment;
	bool str1=true;
	if (!description.empty())
	{
		for (unsigned int i=0; i<package.get_name().length()+1; i++)
		{
			dpos=i;
			tmp+=description[i];
		}
//		printf("After 1 cycle:\n%s\n", tmp.c_str());

		while(dpos<description.length())
		{
			while(tmp!=package.get_name()+":")
			{
				for (unsigned int i=dpos; description[i]!='\n' && i<description.length(); i++)
				{
					dpos=i;
					// Skipping to line end
				}
				tmp.clear();
				for (unsigned int i=dpos+1; i<package.get_name().length()+1; i++)
				{
					dpos=i;
					tmp+=description[i];
				}
			}
			tmp.clear();
			for (unsigned int i=dpos+1; description[i]!='\n' && i<description.length(); i++)
			{
				comment+=description[i];
			}
			if (str1)
			{
				package.set_short_description(comment);
				comment.clear();
			}
		}
		package.set_description(comment);
	}
#endif
	printf(".");
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
	//printf("output written to %s\n", xml_output.c_str());
	printf(".");
	pkg.writeToFile(xml_output.c_str());
	printf(".");
	return 0;
}

int convert_package(string filename, string output_dir)
{
//	char tmp[1000];
	printf("converting package %s...", filename.c_str());
	int name_start;
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
//	printf("tmp_dir=%s\n", tmp_dir.c_str());
	string xml_output=tmp_dir+"/install/data.xml";
	string reasm="rm "+tmp_dir+" && mkdir "+tmp_dir+" "+/*"2>/dev/null"+*/" && cp "+filename+" "+tmp_dir+"/ && cd "+tmp_dir+" &&  tar zxf "+real_filename+" > /dev/null";
//	printf("cmd: %s\n",reasm.c_str());
//	scanf("%s", &tmp);
	system(reasm.c_str());	
	slack_convert(filename, xml_output);
	reasm="cd "+tmp_dir+" && rm "+real_filename+" && makepkg -l n -c n "+real_filename+" 2&>/dev/null &&  mv *.tgz "+output_dir+" && rm -rf "+tmp_dir;
	system(reasm.c_str());
	printf("done\n");
	return 0;
}



