/********************************************************
 * A try to reimplement slow STL string to make it faster
 * $Id: faststl.h,v 1.3 2007/05/13 21:09:23 i27249 Exp $
 * ******************************************************/


#ifndef FASTSTL_H_
#define FASTSTL_H_
#include <stdio.h>
#include <string>
using namespace std;
class FastString
{
	private:
		char *data;
	public:
		FastString();
		~FastString();
		char operator [] (unsigned int i);
		bool operator = (char c);
		bool operator = (const char *c);
		bool operator += (char c);
		bool operator += (const char *c);
		bool operator += (FastString str);
		bool operator == (const char *c);
		bool operator == (FastString str);
		FastString operator + (FastString str);
		FastString operator + (const char *c);
		unsigned int length();
		unsigned int size();
		FastString substr(unsigned int start, int end=-1);
		int find_first_of(FastString str);
		int find(FastString str);
		int find_last_of(FastString str);
		//void push_back(FastString str);
		//void push_back(char *c);
		//void push_back(char c);
		void clear();
		bool empty();
		char * c_str();
};





#endif // FASTSTL_H_
