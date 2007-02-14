/*
    Comparsion conditions constants and conversion functions
    $Id: conditions.h,v 1.4 2007/02/14 14:33:58 i27249 Exp $
*/

#ifndef CONDITIONS_H_
#define CONDITIONS_H_

#define VER_MORE 1	// >
#define VER_LESS 2	// <
#define VER_EQUAL 3	// ==
#define VER_NOTEQUAL 4	// !=
#define VER_XMORE 5	// >=
#define VER_XLESS 6	// <=
#define VER_ANY 7	// any

#define COND_MORE 	"more"		// >	1
#define COND_LESS 	"less"		// <	2
#define COND_EQUAL 	"equal"		// ==	3
#define COND_NOTEQUAL 	"notequal"	// !=	4
#define COND_XMORE 	"atleast"	// >=	5
#define COND_XLESS 	"notmore"	// <=	6
#define COND_ANY	"any"		// any	7

#define HCOND_MORE 	">"
#define HCOND_LESS 	"<"
#define HCOND_EQUAL 	"=="
#define HCOND_NOTEQUAL 	"!="
#define HCOND_XMORE 	">="
#define HCOND_XLESS 	"<="
#define HCOND_ANY	"any"


#include <string>
using namespace std;
int condition2int(string condition);
string hcondition2xml(string condition);
#endif //CONDITIONS_H_
