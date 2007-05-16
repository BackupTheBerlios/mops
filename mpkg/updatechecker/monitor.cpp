#include "monitor.h"

monitorThread::monitorThread()
{
	TIMER_RES=60;
	action=MST_DISABLE;
}

void monitorThread::run()
{
	forever
	{
		switch(action)
		{
			case MST_ENABLE:
				checkUpdates();
			case MST_DISABLE:
				sleep(TIMER_RES);
				break;
			case MST_SHUTDOWN:
				return;
		}
	}
}

void monitorThread::enable()
{
	action=MST_ENABLE;
}

void monitorThread::disable()
{
	action=MST_DISABLE;
}

void monitorThread::shutdown()
{
	action=MST_SHUTDOWN;
}

void monitorThread::setUpdateInterval(unsigned int sec)
{
	TIMER_RES=sec;
}

void monitorThread::checkUpdates()
{
	vector<string>filename;
	string cmd = "gunzip ";
	bool proceed=false;
	for (unsigned int i=0; i<REPOSITORY_LIST.size(); i++)
	{
		proceed=false;
		filename.push_back(get_tmp_file());
		if (CommonGetFile(REPOSITORY_LIST.at(i)+"packages.xml.gz", filename.at(i)+".gz")==DOWNLOAD_OK)
		{
			cmd = "gunzip "+ filename.at(i)+".gz";
			if (system(cmd.c_str()) && \
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
		emit updatesDetected();
	}
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
