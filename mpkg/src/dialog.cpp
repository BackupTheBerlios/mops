/****************************************************************
 * Basic C++ bingings to dialog utility
 * $Id: dialog.cpp,v 1.7 2007/07/11 13:54:28 i27249 Exp $
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

string Dialog::resolveComment(string tag)
{
	vector<TagPair> comments;

	comments.push_back(TagPair("ftp", "Онлайн-репозиторий (FTP)"));
	comments.push_back(TagPair("http", "Онлайн-репозиторий (HTTP)"));
	comments.push_back(TagPair("cdrom", "Пакеты на CD/DVD-диске"));
	comments.push_back(TagPair("file", "Пакеты на смонтированной файловой системе"));
	for (unsigned int i=0; i<comments.size(); i++)
	{
		if (comments[i].tag==tag)
			return comments[i].value;
	}
	return (string) "<Неизвестный тип репозитория либо некорректный URL>";
}


void Dialog::execAddableList(string header, vector<string> *menuItems, string tagLimiter)
{
	// Legend:
	// tagLimiter, for exapmle, may be equal to "://" - this means that the value of "ftp://something.com" will be splitted in next way:
	// 	ftp will be a value (or assotiated comment using internal database), and something.com will be a tag.
	string tmp_file, exec_str, value;
	vector<TagPair> menuList;
	vector<string> tmpList;
	int ret;
	unsigned int pos;
	tmp_file = get_tmp_file();

begin:
	exec_str = "dialog --ok-label \"Удалить\" --cancel-label \"Продолжить\" --extra-button --extra-label \"Добавить\" --menu \"" + header + "\" " + \
			   IntToStr(0) + " " + IntToStr(0) + " " + IntToStr(0);

	menuList.clear();
	if (!tagLimiter.empty())
	{
		for (unsigned int i=0; i<menuItems->size(); i++)
		{
			pos = menuItems->at(i).find(tagLimiter);
			if (pos!=std::string::npos)
			{
				menuList.push_back(TagPair(menuItems->at(i), resolveComment(menuItems->at(i).substr(0, pos))));
			}
			else
			{
				menuList.push_back(TagPair(menuItems->at(i), "Некорректный URL"));
			}
		}
	}
	for (unsigned int i=0; i<menuList.size(); i++)
	{
		exec_str += " \"" + menuList[i].tag + "\" \"" + menuList[i].value + "\" ";
	}
	exec_str += " 2>"+tmp_file;
	ret = system(exec_str.c_str());
	printf("returned %i\n", ret);
	switch(ret)
	{
		case 256: // OK button
			printf("Ok\n");
			return;
			break;
		case 768: // Add button
			value = execInputBox("Введите URL репозитория:", "");
			if (!value.empty())
			{
				menuItems->push_back(value);
			}
			goto begin;
			break;
		case 0:	// Delete button
			mError("delete button");
			value = getReturnValue(tmp_file);
			if (!value.empty())
			{
				if (menuList.size()==1)
				{
					execMsgBox("Список не может быть пустым. Сначала добавьте еще что-нибудь");
					goto begin;
				}
				for (unsigned int i=0; i<menuList.size(); i++)
				{
					if (menuList[i].tag == value)
					{
						tmpList.clear();
						for (unsigned int t=0; t<menuItems->size(); t++)
						{
							if (menuItems->at(t)!=value)
							{
								tmpList.push_back(menuItems->at(t));
							}
						}
						*menuItems = tmpList;
						tmpList.clear();
						mDebug("Deleted " + value);
						goto begin;
					}
				}
				mError("out of cycle");
				goto begin;
			}
			else
			{
				mDebug("empty value");
			}
			goto begin;
			break;
		default: // Cancel, ESC, and other errors
			mError(exec_str);
			mDebug("Returned " +  IntToStr(ret));
			sleep(5);
			
			return;		
/*			if (execYesNo("Действительно прервать?")) abort();
			else goto begin;*/
	}
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
