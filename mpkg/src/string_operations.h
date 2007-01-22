/**********************************************************
 * Standard C String helpful functions - header file
 * $Id: string_operations.h,v 1.2 2007/01/22 00:38:47 i27249 Exp $
 * ********************************************************/
#ifndef _STRING_OPERATIONS_H_
#define _STRING_OPERATIONS_H_
#include <string>
using namespace std;
char * strMerge(const char *part1, const char *part2); // Merges part1 and part2, and returns the result. Note: you should free() the result at the end.

class mstring
{
	public:
		bool operator += (string str2);
		bool operator += (const char *str2);
		bool operator == (string str2);
		bool operator == (const char *str2);
		bool operator != (string str2);
		bool operator != (const char *str2);
		bool operator + (string str2);
		bool operator + (const char *str2);

		char operator [] (int i);
		void clear();
		bool empty();
		const char * c_str();
		string s_str();
		mstring();
		~mstring();

	private:
		string str;
};

#endif

