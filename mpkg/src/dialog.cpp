/****************************************************************
 * Basic C++ bingings to dialog utility
 * $Id: dialog.cpp,v 1.5 2007/06/15 12:40:52 i27249 Exp $
 *
 * Developed as part of MOPSLinux package system, but can be used
 * separately
 */
#include "dialog.h"

TagPair::TagPair()
{
	checkState=false;
}
TagPair::~TagPair()
{
}

TagPair::TagPair(string tagV, string valueV, bool checkStateV)
{
	tag=tagV;
	value=valueV;
       	checkState=checkStateV;
}
void TagPair::clear()
{
	tag.clear();
	value.clear();
	checkState=false;
}

string Dialog::getReturnValue(string tmp_file)
{
	FILE *returnValues = fopen(tmp_file.c_str(), "r");
	if (!returnValues)
	{
		perror("cannot open temp file");
		abort();
	}
	char membuff[2000];
	string ret;
	memset(&membuff, 0, sizeof(membuff));
	if (fscanf(returnValues, "%s", &membuff)!=EOF)
	{
		ret = (string) membuff;
	}
	fclose(returnValues);
	mDebug("returned [" + ret + "]");
	return ret;
}

vector<string> Dialog::getReturnValues(string tmp_file, bool quoted)
{
	FILE *returnValues = fopen(tmp_file.c_str(), "r");
	if (!returnValues)
	{
		perror("cannot open temp file");
		abort();
	}
	char membuff[2000];
	vector<string> ret;
	string tmp;
	memset(&membuff, 0, sizeof(membuff));
	while (fscanf(returnValues, "%s", &membuff)!=EOF)
	{
		tmp = (string) membuff;
		if (quoted) tmp = tmp.substr(1, tmp.size()-2);
		ret.push_back(tmp);
	}

	fclose(returnValues);
	return ret;
}

bool Dialog::execYesNo(string header, unsigned int height, unsigned int width)
{
	string tmp_file = get_tmp_file();
	string exec_str = "dialog --yesno \"" + header + "\" " + IntToStr(height) + " " + IntToStr(width);
	int r = system(exec_str.c_str());
	unlink(tmp_file.c_str());
	if (r==0) return true;
	else return false;
}
void Dialog::execInfoBox(string text, unsigned int height, unsigned int width)
{
	string exec_str = "dialog --infobox \"" + text + "\" " + IntToStr(height) + " " + IntToStr(width);
	system(exec_str.c_str());
}

void Dialog::execMsgBox(string text, unsigned int height, unsigned int width)
{
	string exec_str = "dialog --msgbox \"" + text + "\" " + IntToStr(height) + " " + IntToStr(width);
	system(exec_str.c_str());
}
string Dialog::execMenu(string header, unsigned int height, unsigned int width, unsigned int menu_height, vector<TagPair> menuItems)
{
	if (menuItems.empty())
	{
		mError("Empty item list");
		return "";
	}
	string tmp_file = get_tmp_file();
	string exec_str = "dialog --menu \"" + header + "\" " + IntToStr(height) + " " + IntToStr(width) + " " + IntToStr(menu_height);
	for (unsigned int i=0; i<menuItems.size(); i++)
	{
		exec_str += " \"" + menuItems[i].tag + "\" \"" + menuItems[i].value+"\"";
	}
	exec_str += " 2>"+tmp_file;
	system(exec_str.c_str());
	string ret = getReturnValue(tmp_file);
	unlink(tmp_file.c_str());
	return ret;
}

string Dialog::execInputBox(string header, string default_text, unsigned int height, unsigned int width)
{
	string tmp_file = get_tmp_file();
	string exec_str = "dialog --inputbox \"" + header + "\" " + IntToStr(height) + " " + IntToStr(width) + " \"" + default_text + "\"" + " 2>"+tmp_file;
	system(exec_str.c_str());
	string ret = getReturnValue(tmp_file);
	unlink(tmp_file.c_str());
	return ret;
}

int Dialog::execMenu(string header, vector<string> menuItems)
{
	if (menuItems.empty())
	{
		mError("Empty item list");
		return -1;
	}
	vector<TagPair> mItems;
	for (unsigned int i=0; i<menuItems.size(); i++)
	{
		mItems.push_back(TagPair(IntToStr(i), menuItems[i]));
	}
	string ret = execMenu(header, 0,0,0, mItems);
	if (ret.empty()) return -1;
	return atoi(ret.c_str());
}
void Dialog::execGauge(string text, unsigned int height, unsigned int width, int value)
{
	string exec_str = "echo " + IntToStr(value) + " | dialog --gauge \"" + text + "\" " + IntToStr(height) + " " + IntToStr(width);
	
	//string exec_str = "dialog --gauge \"" + text + "\" " + IntToStr(height) + " " + IntToStr(width) + " &";
	system(exec_str.c_str());
	/*for (unsigned int i=0; i<100; i++)
	{
		fprintf(stdout, "%s\n", IntToStr(i).c_str());
		usleep(100);
	}*/
}
bool Dialog::execCheckList(string header, unsigned int height, unsigned int width, unsigned int menu_height, vector<TagPair>* menuItems)
{
	if (menuItems->empty())
	{
		mError("Empty item list");
		return false;
	}
	string tmp_file = get_tmp_file();
	string exec_str = "dialog --checklist \"" + header + "\" " + IntToStr(height) + " " + IntToStr(width) + " " + IntToStr(menu_height);
	for (unsigned int i=0; i<menuItems->size(); i++)
	{
		exec_str += " \"" + menuItems->at(i).tag + "\" \"" + menuItems->at(i).value+"\" ";
		if (menuItems->at(i).checkState) exec_str += "on";
		else exec_str += "off";
	}
	exec_str += " 2>"+tmp_file;
	int d_ret = system(exec_str.c_str());
	vector<string> ret = getReturnValues(tmp_file);
	unlink(tmp_file.c_str());
	if (d_ret!=0)
		return false;
	for (unsigned int i=0; i<menuItems->size(); i++)
	{
		menuItems->at(i).checkState=false;
	}
	for (unsigned int i=0; i<ret.size(); i++)
	{
		for (unsigned int t=0; t<menuItems->size(); t++)
		{
			if (ret[i]==menuItems->at(t).tag)
			{
				menuItems->at(t).checkState=true;
			}
		}
	}
	return true;
}

Dialog::Dialog(){}
Dialog::~Dialog(){}
