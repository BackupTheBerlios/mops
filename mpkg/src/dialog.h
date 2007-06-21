/****************************************************************
 * Basic C++ bingings to dialog utility
 * $Id: dialog.h,v 1.6 2007/06/21 18:35:48 i27249 Exp $
 *
 * Developed as part of MOPSLinux package system, but can be used
 * separately
 */
#ifndef DIALOGCPP_H_
#define DIALOGCPP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include "file_routines.h"
using namespace std;
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
	void execAddableList(string header, vector<string> *menuItems, string tagLimiter="");
	string resolveComment(string tag);
	bool execYesNo(string header, unsigned int height=0, unsigned int width=0);
	void execInfoBox(string text, unsigned int height=0, unsigned int width=0);
	vector<string> getReturnValues(string tmp_file, bool quoted=true);
	string execInputBox(string header, string default_text, unsigned int height=0, unsigned int width=0);

	string execMenu(string header, unsigned int height, unsigned int width, unsigned int menu_height, vector<TagPair> menuItems);
	void execMsgBox(string text, unsigned int height=0, unsigned int width=0);
	int execMenu(string header, vector<string> menuItems);
	void execGauge(string text, unsigned int height, unsigned int width, int value);
	bool execCheckList(string header, unsigned int height, unsigned int width, unsigned int menu_height, vector<TagPair>* menuItems);
};

#endif
