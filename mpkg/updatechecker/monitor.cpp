/******************************************
 * MOPSLinux package system
 * Update monitor - processing thread
 * $Id: monitor.cpp,v 1.6 2007/05/21 19:25:45 i27249 Exp $
 */

#include "monitor.h"
unsigned int TIMER_RES=200;
monitorThread::monitorThread()
{	setPriority(QThread::LowestPriority);
	setUpdateInterval(60);
	action=MST_DISABLE;
	launchAction=LAC_NONE;
}

void monitorThread::forceCheck()
{
	_forceCheck=true;
	enable();
}

void monitorThread::run()
{
	forever
	{
		switch(action)
		{
			case MST_ENABLE:
				if (idleTime<updateThreshold && !_forceCheck)
				{
					idleTime++;
					switch(launchAction)
					{
						case LAC_MANAGER:
							_launchManager();
							launchAction=LAC_NONE;
							break;
						case LAC_MERGE:
							_mergeUpdates();
							launchAction=LAC_NONE;
							break;
						default:
							break;
					}
				}
				else
				{
					_forceCheck=false;
					printf("Checking.........\n");
					idleTime=0;
					checkUpdates();
				}
			case MST_DISABLE:
				msleep(TIMER_RES);
				break;
			case MST_SHUTDOWN:
				return;
		}
	}
}

void monitorThread::enable()
{
	action=MST_ENABLE;
	start();
}

void monitorThread::disable()
{
	action=MST_DISABLE;
}

void monitorThread::shutdown()
{
	action=MST_SHUTDOWN;
	exit();
}

void monitorThread::setUpdateInterval(unsigned int sec)
{
	updateThreshold=sec*5;
}

void monitorThread::launchManager()
{
	launchAction=LAC_MANAGER;
}
void monitorThread::_launchManager()
{
	system("manager");
}

void monitorThread::mergeUpdates()
{
	launchAction=LAC_MERGE;
}

void monitorThread::_mergeUpdates()
{
	system("mpkg update");
	emit showStateMessage(tr("Database updated"));
}


void monitorThread::checkUpdates()
{
	hDatabase.setControlPoint();
	loadGlobalConfig();
	vector<string>filename;
	string cmd = "gunzip -f ";
	bool proceed=false;
	for (unsigned int i=0; i<REPOSITORY_LIST.size(); i++)
	{
		printf("Checking %s\n",REPOSITORY_LIST.at(i).c_str());
		proceed=false;
		filename.push_back(get_tmp_file());
		if (CommonGetFile(REPOSITORY_LIST.at(i)+"packages.xml.gz", filename.at(i)+".gz")==DOWNLOAD_OK)
		{
			cmd = "gunzip -f "+ filename.at(i)+".gz";
			if (FileNotEmpty(filename.at(i)+".gz") && system(cmd.c_str()) && \
					ReadFile(filename.at(i)).find("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<repository")!=std::string::npos)
			{
				proceed=true;
			}
		}
		if (!proceed && CommonGetFile(REPOSITORY_LIST.at(i) + "PACKAGES.TXT", filename.at(i))==DOWNLOAD_OK && \
				FileNotEmpty(filename.at(i)))
		{
			proceed = true;
		}
		if (proceed) hDatabase.setRepositoryData(REPOSITORY_LIST.at(i), get_file_md5(filename.at(i)));
	}

	if (hDatabase.changed())
	{
		hDatabase.setControlPoint();
		emit updatesDetected(true);
	}
	else emit updatesDetected(false);
	printf("Check complete\n");
	delete_tmp_files();
}

HashDatabase::HashDatabase()
{
	loadHashes();
	changesExist=false;
}

HashDatabase::~HashDatabase(){}

void HashDatabase::loadHashes()
{
	XMLResults XMLE;
	XMLNode _data = XMLNode::parseFile("/etc/mpkg-repository-hashes.xml","hashes", &XMLE);
	repList.clear();
	if (XMLE.error==eXMLErrorNone)
	{
		for (int i=0; i<_data.nChildNode("rep"); i++)
		{
			if (_data.getChildNode("rep", i).nChildNode("url")!=0 && _data.getChildNode("rep", i).nChildNode("md5")!=0)
			{
				repList.push_back((string) _data.getChildNode("rep", i).getChildNode("url").getText());
				data[(string) _data.getChildNode("rep", i).getChildNode("url").getText()] = \
					(string) _data.getChildNode("rep", i).getChildNode("md5").getText();
			}
		}
	}
}

void HashDatabase::saveHashes()
{
	XMLNode _data = XMLNode::createXMLTopNode("hashes");
	for (unsigned int i=0; i<repList.size(); i++)
	{
		_data.addChild("rep");
		_data.getChildNode("rep", i).addChild("url");
		_data.getChildNode("rep", i).addChild("md5");
		_data.getChildNode("rep", i).getChildNode("url").addText(repList.at(i).c_str());
		_data.getChildNode("rep", i).getChildNode("md5").addText(data[repList.at(i)].c_str());
	}
	_data.writeToFile("/etc/mpkg-repository-hashes.xml");
}

void HashDatabase::setControlPoint()
{
	saveHashes();
	changesExist=false;
}

void HashDatabase::setRepositoryData(string url, string md5)
{
	// Searching in repList
	bool found=false;
	for (unsigned int i=0; i<repList.size(); i++)
	{
		if (repList.at(i)==url) found=true;
	}
	if (!found)
	{
		repList.push_back(url);
		changesExist=true;
	}
	if (data[url]!=md5)
	{
		changesExist=true;
		data[url]=md5;
	}
}

bool HashDatabase::changed()
{
	return changesExist;
}
