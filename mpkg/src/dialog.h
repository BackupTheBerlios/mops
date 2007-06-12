/****************************************************************
 * Basic C++ bingings to dialog utility
 * $Id: dialog.h,v 1.3 2007/06/12 07:49:47 i27249 Exp $
 *
 * Developed as part of MOPSLinux package system, but can be used
 * separately
 */
#ifndef DIALOGCPP_H_
#define DIALOGCPP_H_

/*#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>*/
#include "libmpkg.h"
//using namespace std;
class TagPair
{
	public:
	TagPair(string tagV, string valueV, bool checkState=false);
	TagPair();
	~TagPair();
	string tag;
	string value;
	bool checkState;
	void clear();
};

class Dialog
{
	public:
	Dialog();
	~Dialog();
	string getReturnValue(string tmp_file);
	bool execYesNo(string header, unsigned int height=0, unsigned int width=0);
	void execInfoBox(string text, unsigned int height=0, unsigned int width=0);
	vector<string> getReturnValues(string tmp_file, bool quoted=true);
	string execMenu(string header, unsigned int height, unsigned int width, unsigned int menu_height, vector<TagPair> menuItems);
	void execMsgBox(string text, unsigned int height=0, unsigned int width=0);
	int execMenu(string header, vector<string> menuItems);
	void execGauge(string text, unsigned int height, unsigned int width, int value);
	bool execCheckList(string header, unsigned int height, unsigned int width, unsigned int menu_height, vector<TagPair>* menuItems);
};

#endif
