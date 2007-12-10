/**********************************************************
 * Standard C String helpful functions - header file
 * $Id: string_operations.h,v 1.16 2007/12/10 03:12:59 i27249 Exp $
 * ********************************************************/
#ifndef _STRING_OPERATIONS_H_
#define _STRING_OPERATIONS_H_
#include <string>
#include <vector>
#include <stdio.h>
using namespace std;
string IntToStr(long long num);
string getAbsolutePath(string directory);
void PrepareSql(string* str);
char * strMerge(const char *part1, const char *part2); // Merges part1 and part2, and returns the result. Note: you should free() the result at the end.
string cutSpaces(string str);
string humanizeSize(string size);
string humanizeSize(double size);
unsigned int fl2ul(float input);
string adjustStringWide(string input, unsigned int char_width, string prefix="");

string getExtension(string filename);
string getFilename(string fullpath);
string getDirectory(string fullpath);
class StringMap {
	public:
		StringMap();
		~StringMap();
		string getValue(string keyName);
		int keyNum(string keyName);
		void setValue(string keyName, string keyValue);
		bool getValue(string keyName, string *keyValue); // More complex function, returns false if key not found

	private:
		vector<string> value;
		vector<string> key;
};
class mstring
{
	public:
		bool operator += (string str2);
		bool operator += (const char *str2);
		bool operator += (char str2);
		bool operator == (string str2);
		bool operator == (const char *str2);
		bool operator != (string str2);
		bool operator != (const char *str2);
		bool operator + (string str2);
		bool operator + (const char *str2);
		bool operator = (const mstring str2);
		bool operator = (string str2);
		bool operator = (const char *str2);


		char operator [] (int i);
		void clear();
		bool empty();
		const char * c_str();
		unsigned int length();
		string s_str();
		mstring();
		~mstring();

	private:
		string str;
};

#endif

