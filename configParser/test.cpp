/**
 * $Id: test.cpp,v 1.1 2006/12/15 09:40:40 adiakin Exp $
 */

#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>

#include "PackageConfig.h"

#define CONF "conf.xml"

using namespace std;

int main(int argc, char* argv[])
{
	printf("ok\n");
	string name("");
	string version("");
	string author_name("");
	string author_email("");
	string sdesc("");
	string desc("");
	
	try {
	
		
		//PackageConfig *p = new PackageConfig("conf.xml");
		PackageConfig p("./conf.xml");

		name = p.getName();

		version = p.getVersion();

		sdesc = p.getShortDescription();

		desc = p.getDescription();

		author_email = p.getAuthorEmail();

		author_name = p.getAuthorName();
		cout << "name: " << name 
		<< "\n ver: " << version
		<< "\n sdesc: " << sdesc
		<< "\n aemail: " << author_email
		<< "\n ename: " << author_name
		<< "\n desc:\n" << desc << endl;
	} catch ( std::exception e) {
		printf("%s\n", e.what() );
	}
	
	
	return 0;	
}
