/********************************************************
 * A try to reimplement slow STL string to make it faster
 * $Id: faststl.cpp,v 1.5 2007/05/23 14:16:03 i27249 Exp $
 * ******************************************************/


#include "faststl.h"

FastString::FastString()
{
	data = (char *) malloc(1);
}

FastString::~FastString()
{
}

FastString::FastString ( const char *c)
{
	//mDebug("str = " + c);
	data = (char *) malloc(sizeof(c));
	strcpy(data,c);
}

char FastString::operator [] (unsigned int i)
{
	if (this->length()>i) return data[i];
	else
	{
		//mError("out of range (length = " + IntToStr(length()) + ", requested " + IntToStr(i) +")");
		return 0;
	}
}

bool FastString::operator = (const char *c)
{
	//free(&data);
	realloc(&data, sizeof(c));
	//realloc(&data, sizeof(c));
	strcpy(data, c);
	return true;
}

bool FastString::operator = (char c)
{
	if (data!=NULL) free(&data);
	realloc(&data,sizeof(c));
	strcpy(data, &c);
	return true;
}

bool FastString::operator += (const char *c)
{
	realloc(&data, sizeof(data)+sizeof(c)-1);
	strcat(data,c);
	return true;
}

bool FastString::operator += (FastString str)
{
	realloc(&data, sizeof(data)+sizeof(str.c_str())-1);
	strcat(data,str.c_str());
	return true;
}

bool FastString::operator == (FastString str)
{
	if (strcmp(data, str.c_str())==0) return true;
	else return false;
}

bool FastString::operator == (const char *c)
{
	if (strcmp(data, c)==0) return true;
	else return false;
}

FastString FastString::operator + (FastString str)
{
	realloc(&data, sizeof(data)+sizeof(str.c_str())-1);
	strcat(data,str.c_str());
	return *this;
}

FastString FastString::operator + (const char *c)
{
	realloc(&data, sizeof(data)+sizeof(c)-1);
	strcat(data,c);
	return *this;
}

unsigned int FastString::length()
{
	return strlen(data);
}

unsigned int FastString::size()
{
	return strlen(data);
}

FastString FastString::substr(unsigned int start, int length)
{
	if (length<0) length = strlen(data) - start;
	char tmp[length];
	//tmp = (char *) malloc(length+1);
	for (unsigned int i=start; i<=start+length; i++)
	{
		tmp[i-start]=data[i];
	}
	FastString _tmp;
        _tmp	= tmp;
        free(&tmp);
	return _tmp;
}

int FastString::find_first_of(FastString str)
{
	unsigned int z = strlen(data);
	unsigned int y = str.length();
	for (unsigned int i=0; i<z; i++)
	{
		for (unsigned int t=0; t<y; t++)
		{
			if (data[i]==str[t]) return i;
		}
	}
	return std::string::npos;
}

int FastString::find(FastString str)
{
	unsigned int z = strlen(data);
	unsigned int y = str.length();
	if (str.length()>z) return std::string::npos;
	bool found = true;
	for (unsigned int i=0; i<z-y; i++)
	{
		found=true;
		for (unsigned int t=0; t<str.length(); t++)
		{
			if (data[i+t]!=str[t]) found=false;
			break;
		}
		if (found) return i;
	}
	return std::string::npos;
}

int FastString::find_last_of(FastString str)
{
	unsigned int z = strlen(data);
	unsigned int y = str.length();
	for (int i=z-1; i>=0; i++)
	{
		for (unsigned int t=0; t<y; t++)
		{
			if (data[i]==str[t]) return i;
		}
	}
	return std::string::npos;
}




//		void push_back(FastString str);
//		void push_back(char *c);
//		void push_back(char c);
void FastString::clear()
{
	free(data);
}
bool FastString::empty()
{
	if (strlen(data)==0) return true;
	else return false;
}
const char * FastString::c_str()
{
	return data;
}

