/***********************************************************
 * Standard C String helpful functions
 * $Id: string_operations.cpp,v 1.17 2007/11/06 20:25:18 i27249 Exp $
 * ********************************************************/

#include "string_operations.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
void PrepareSql(string *str)
{
	if (str->empty())
	{
		*str="0";
		return;
	}
	if (str->find("\'")==std::string::npos) return;
	unsigned int last_pos=0;
	unsigned int offset = 0;
	while (offset<str->length())
	{
		last_pos=str->substr(offset).find_first_of("\'");
		if (last_pos!=std::string::npos) str->insert(last_pos+offset, "\'");
		else return;
		offset = offset + last_pos + 2;
	}
	
}

string getAbsolutePath(string directory)
{
	string cwd = get_current_dir_name();
	cwd+="/";
	// Костыль-mode
	if (directory.find("/")==0) return directory; // Already absolute
	return cwd + directory;
}

string getExtension(string filename)
{
	filename = getFilename(filename);
	if (filename.find(".")==std::string::npos || filename.find_last_of(".")==filename.length()-1) return ""; // No extension
	return filename.substr(filename.find_last_of(".")+1);
}

string getFilename(string fullpath)
{
	if (fullpath.find("/")==std::string::npos) return fullpath;
	if (fullpath.find_last_of("/")==fullpath.length()-1) return "";
	return fullpath.substr(fullpath.find_last_of("/")+1);
}

string getDirectory(string fullpath)
{
	if (fullpath.find("/")==std::string::npos) return "";
	return fullpath.substr(0, fullpath.find_last_of("/"));

}

// Helpful function ))
string IntToStr(long long num)
{
  	char *s = (char *) malloc(2000);
  	string ss;
  	if (s)
  	{
		sprintf(s,"%Ld",num);
	  	ss=s;
	  	free(s);
  	}
  	else 
  	{
		perror("Error while allocating memory");
	  	abort();
  	}
  	return ss;
}


char * strMerge(const char *part1, const char *part2)
{

	int p1=strlen(part1);
	int p2=strlen(part2);
	// Beginning dump
	char *ret=(char *) malloc(p1+p2+1);
	for (int i=0; i<p1; i++)
	{
		ret[i]=part1[i];
	}
	for (int i=p1; i<=p1+p2; i++)
	{
		ret[i]=part2[i-p1];
	}
	return ret;
}

string cutSpaces(string str)
{
	int start = str.find_first_not_of(" \n\t");
	int end = str.find_last_not_of(" \n\t");
	string ret =  str.substr(start, end-start+1);
	return ret;
}

bool mstring::operator += (string str2)
{
	string tmp;
	tmp=str+str2;
	str=tmp;
	tmp.clear();
	return true;
}

bool mstring::operator += (const char *str2)
{
	string tmp = str + (string) str2;
	str=tmp;
	tmp.clear();
	return true;
}

bool mstring::operator += (char str2)
{
	string tmp = str + str2;
	str=tmp;
	tmp.clear();
	return true;
}

bool mstring::operator == (string str2)
{
	if (str==str2) return true;
	return false;
}

bool mstring::operator == (const char *str2)
{
	if (str==(string) str2) return true;
	return false;
}

bool mstring::operator = (string str2)
{
	str=str2;
	return true;
}

bool mstring::operator = (const char *str2)
{
	str = (string) str2;
	return true;
}

bool mstring::operator != (string str2)
{
	if (str!=str2) return true;
	return false;
}

bool mstring::operator != (const char *str2)
{
	if (str!=(string) str2) return true;
	return false;
}

char mstring::operator [] (int i)
{
	return str[i];
}

void mstring::clear()
{
	str.clear();
}

bool mstring::empty()
{
	return str.empty();
}

const char * mstring::c_str()
{
	return str.c_str();
}

string mstring::s_str()
{
	return str;
}

unsigned int mstring::length()
{
	return str.length();
}

mstring::mstring(){}
mstring::~mstring(){}
unsigned int fl2ul(float input)
{
	unsigned int preout = (unsigned int) input;
	if (!(input - preout < .5))
	{
		preout++;
	}
	return preout;
}

string humanizeSize(string size)
{
	return humanizeSize(strtod(size.c_str(), NULL));
}
string humanizeSize(double size)
{
	long double new_size;
	string ret;
	string tmp;

	if (size >= 1024 && size < 1048576)
	{
		new_size = size/1024;
		tmp=IntToStr((int) (new_size*1000));
		if (tmp.length()>=4) tmp=tmp.substr(1,2);
		ret = IntToStr((int) new_size) + "."+ tmp + " Kb";
		return ret;
	}
	if (size >= (1024*1024) && size < (1024*1024*1024))
	{
		new_size = size/(1024*1024);
		tmp=IntToStr((int) (new_size*1000));
		if (tmp.length()>=4) tmp=tmp.substr(1,2);
		ret = IntToStr((int) new_size) + "." +tmp + " Mb";
		return ret;
	}
	if (size >= (1024*1024*1024))
	{
		new_size = size/(1024*1024*1024);
		tmp=IntToStr((int) (new_size*1000));
		if (tmp.length()>=4) tmp=tmp.substr(1,2);
		ret = IntToStr((int) new_size) + "." + tmp + " Gb";

		return ret;
	}
	return "0";
}

string adjustStringWide(string input, unsigned int char_width, string prefix)
{
	if (char_width < 50) char_width = 50;
	vector<string> spaces;
	vector<string> chunks;
	int lspace=0;
	for (int i=0; i < (int) input.size(); i++)
	{
		if (input[i]==' ') lspace = i;
		if (i>=(int) char_width)
		{
			if (lspace==0) lspace = i;
			chunks.push_back(input.substr(0,lspace));
			input=input.substr(lspace);
			i=-1;
			lspace = 0;
		}
		if (i==(int) input.size()-1) chunks.push_back(input);

	}
	
	string ret;
	for (unsigned int i=0; i<chunks.size(); i++)
	{
		if (prefix.empty())
		{
			ret+=chunks[i];
		       	ret += "<br>";
		}
		else
		{	
			ret+=prefix+": " + chunks[i];
			ret+="\n";
		}

	}
	return ret;

}
