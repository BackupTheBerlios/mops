/**********************************************************
 * Standard C String helpful functions - header file
 * $Id: string_operations.h,v 1.6 2007/04/22 12:03:27 i27249 Exp $
 * ********************************************************/
#ifndef _STRING_OPERATIONS_H_
#define _STRING_OPERATIONS_H_
//#include <string>
#include "dataunits.h"
//using namespace std;
char * strMerge(const char *part1, const char *part2); // Merges part1 and part2, and returns the result. Note: you should free() the result at the end.
string cutSpaces(string str);
string humanizeSize(unsigned int size);
string humanizeSize(string size);
unsigned int fl2ul(float input);

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

