/***********************************************************
 * Standard C String helpful functions
 * $Id: string_operations.cpp,v 1.4 2007/02/19 05:14:10 i27249 Exp $
 * ********************************************************/

#include "string_operations.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
char * strMerge(const char *part1, const char *part2)
{
#ifdef DEBUG
	printf("part 1: [%s]\n", part1);
	printf("part 2: [%s]\n", part2);
#endif
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
#ifdef DEBUG
	printf("ret: [%s]\n", ret);
#endif
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
/*
int main()
{
	char *c1="void";
	char *c2="matrix";
	char *c3=strMerge(c1, c2);
	printf("c3 = [%s]\n", c3);
	free(c3);
	return 0;
}
*/
