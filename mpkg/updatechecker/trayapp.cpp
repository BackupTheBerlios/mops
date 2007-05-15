/************************************
 * MOPSLinux package management system
 * Updates monitor - main header file
 * $Id: trayapp.cpp,v 1.1 2007/05/15 22:28:58 i27249 Exp $
 */
#include "trayapp.h"
TrayApp::TrayApp()
{
	core = new mpkg;
}
TrayApp::~TrayApp()
{
	delete core;
}
