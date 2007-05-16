/************************************
 * MOPSLinux package management system
 * Updates monitor - main header file
 * $Id: trayapp.cpp,v 1.2 2007/05/16 00:07:29 i27249 Exp $
 */
#include "trayapp.h"
TrayApp::TrayApp()
{
	core = new mpkg;
	mThread = new monitorThread;
	mThread->start();
	QObject::connect(this, SIGNAL(execThread()), mThread, SLOT(enable()));
	QObject::connect(this, SIGNAL(shutdownThread()), mThread, SLOT(shutdown()));
	QObject::connect(mThread, SIGNAL(updatesDetected()), this, SLOT(showUpdateMessage()));
	emit execThread();
	setIcon(QIcon("/usr/share/mpkg/icons/installed.png"));
	setToolTip("System update monitor");
	//QMenu menu = new QMenu;
	show();

}
TrayApp::~TrayApp()
{
	hide();
	emit shutdownThread();
	while (mThread->isRunning()) sleep(1);
	delete core;
	delete mThread;
}

void TrayApp::showUpdateMessage()
{
	showMessage("System update monitor", "New updates available");
}
