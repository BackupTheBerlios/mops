/******************************************************
 * MOPSLinux packaging system - global configuration
 * $Id: cdconfig.cpp,v 1.2 2007/05/17 15:12:36 i27249 Exp $
 *
 * ***************************************************/

#include "cdconfig.h"
#include "xmlParser.h"
#define HTTP_LIB;
string DL_CDROM_DEVICE;
string DL_CDROM_MOUNTPOINT;
int loadGlobalCdConfig(string config_file)
{
	bool conf_init=false;
	XMLResults xmlErrCode;
	if (access(config_file.c_str(), R_OK)==0)
	{
		XMLNode config=XMLNode::parseFile(config_file.c_str(), "mpkgconfig", &xmlErrCode);
		
		if (xmlErrCode.error != eXMLErrorNone)
		{
			mError("config parse error!");
			conf_init=true;
		}
		if (!conf_init)
		{
			if (config.nChildNode("cdrom_device")!=0)
				cdrom_device=(string) config.getChildNode("cdrom_device").getText();
			if (config.nChildNode("cdrom_mountpoint")!=0)
				cdrom_mountpoint = (string) config.getChildNode("cdrom_mountpoint").getText();
		}

	}
	else
	{
		conf_init=true;
		mError("Configuration file /etc/mpkg.xml not found, using defaults and creating config");
	}
	if (cdrom_device.empty())
	{
		mError("empty cd-rom, using default");
		DL_CDROM_DEVICE="/dev/cdrom";
	}
	else
	{
		mDebug("using cd-rom drive " + cdrom_device);
		DL_CDROM_DEVICE=cdrom_device;
	}

	if (cdrom_mountpoint.empty())
	{
		DL_CDROM_MOUNTPOINT="/mnt/cdrom";
	}

	else
	{
		DL_CDROM_MOUNTPOINT=cdrom_mountpoint;
	}
	return 0;
}

XMLNode mpkgconfig::getXMLConfig(string conf_file)
{
	mDebug("getXMLConfig");
	XMLNode config;
	XMLResults xmlErrCode;
	bool conf_init = false;
	if (access(conf_file.c_str(), R_OK)==0)
	{
		config=XMLNode::parseFile(conf_file.c_str(), "mpkgconfig", &xmlErrCode);
		if (xmlErrCode.error != eXMLErrorNone)
		{
			mError("config parse error!");
			conf_init=true;
		}

	}
	else conf_init = true;
	if (conf_init) config=XMLNode::createXMLTopNode("mpkgconfig");

	if (config.nChildNode("cdrom_device")==0)
	{
		config.addChild("cdrom_device");
		config.getChildNode("cdrom_device").addText(get_cdromdevice().c_str());
	}


	if (config.nChildNode("cdrom_mountpoint")==0)
	{
		config.addChild("cdrom_mountpoint");
		config.getChildNode("cdrom_mountpoint").addText(get_cdrommountpoint().c_str());
	}
	return config;
}

int mpkgconfig::initConfig()
{
	XMLNode tmp=getXMLConfig();
	return setXMLConfig(tmp);
}

int mpkgconfig::setXMLConfig(XMLNode xmlConfig, string conf_file)
{

write_config:
	if (xmlConfig.writeToFile(conf_file.c_str())!=eXMLErrorNone) 
	{
		mError("error writing config file");
		errRet = waitResponce(MPKG_SUBSYS_XMLCONFIG_WRITE_ERROR);
		if (errRet == MPKG_RETURN_RETRY)
		{
			abort();
		}
	}
	loadGlobalCdConfig();
	return 0;
}
string mpkgconfig::get_cdromdevice()
{
#ifndef HTTP_LIB
	return CDROM_DEVICE;
#else
	return DL_CDROM_DEVICE;
#endif
}

string mpkgconfig::get_cdrommountpoint()
{
#ifndef HTTP_LIB
	return CDROM_MOUNTPOINT;
#else
	return DL_CDROM_MOUNTPOINT;
#endif
}
int mpkgconfig::set_cdromdevice(string cdromDevice)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("cdrom_device").deleteNodeContent(1);
	tmp.addChild("cdrom_device");
	mDebug("setting cd device to " + cdromDevice);
	tmp.getChildNode("cdrom_device").addText(cdromDevice.c_str());
}

int mpkgconfig::set_cdrommountpoint(string cdromMountPoint)
{
	XMLNode tmp;
	tmp=getXMLConfig();
	tmp.getChildNode("cdrom_mountpoint").deleteNodeContent(1);
	tmp.addChild("cdrom_mountpoint");
	tmp.getChildNode("cdrom_mountpoint").addText(cdromMountPoint.c_str());
}


