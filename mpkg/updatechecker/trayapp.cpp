/*************************************************************
 * MOPSLinux package management system
 * Updates monitor - main header file
 * $Id: trayapp.cpp,v 1.6 2007/05/16 06:43:20 i27249 Exp $
 ************************************************************/

#include "trayapp.h"

TrayApp::TrayApp()
{
	core = new mpkg(false);
	mThread = new monitorThread;
	mThread->start();

	appMenu = new QMenu;
	appMenu->setTitle("Updates monitor");

	forceUpdateAction = appMenu->addAction(QIcon("/opt/kde/share/icons/crystalsvg/48x48/actions/reload.png"),\
			"Check now");
	appMenu->addSeparator();
	mergeUpdatesAction = appMenu->addAction(QIcon("/opt/kde/share/icons/crystalsvg/48x48/actions/vcs_update.png"),\
		       "Import changes into database");
	launchManagerAction = appMenu->addAction(QIcon("/usr/share/mpkg/icons/icons/crystalsvg/128x128/apps/package_applications.png"),\
		       "Launch package manager");
	appMenu->addSeparator();
	quitAction = appMenu->addAction(QIcon("/opt/kde/share/icons/crystalsvg/48x48/actions/exit.png"),"Quit");
	setContextMenu(appMenu);


	QObject::connect(this, SIGNAL(execThread()), mThread, SLOT(enable()));
	QObject::connect(this, SIGNAL(shutdownThread()), mThread, SLOT(shutdown()));
	QObject::connect(mThread, SIGNAL(updatesDetected(bool)), this, SLOT(showUpdateMessage(bool)));
	QObject::connect(mThread, SIGNAL(showStateMessage(QString)), this, SLOT(showStateMessage(QString)));
	QObject::connect(this, SIGNAL(forceCheck()), mThread, SLOT(forceCheck()));
	QObject::connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApp()));
	QObject::connect(launchManagerAction, SIGNAL(triggered()), mThread, SLOT(launchManager()));
	QObject::connect(mergeUpdatesAction, SIGNAL(triggered()), mThread, SLOT(mergeUpdates()));
	QObject::connect(forceUpdateAction, SIGNAL(triggered()), mThread, SLOT(forceCheck()));
	emit execThread();
	emit forceCheck();
	setToolTip("Updates monitor");
	setIcon(QIcon("/usr/share/mpkg/icons/available.png"));
	show();
}
void TrayApp::quitApp()
{
	hide();
	emit shutdownThread();
	while (mThread->isRunning())
	{
		printf("TrayApp: waiting threads to exit\n");
		sleep(1);
	}
	delete core;
	delete mThread;
	qApp->quit();
}

TrayApp::~TrayApp()
{
}

void TrayApp::showUpdateMessage(bool hasUpdates)
{
	if (hasUpdates) {
		setIcon(QIcon("/usr/share/mpkg/icons/update.png"));
		showMessage("System update monitor", "New updates available");
	}
	else 
	{
		setIcon(QIcon("/usr/share/mpkg/icons/installed.png"));
		//setIcon(QIcon("/opt/kde/share/icons/crystalsvg/22x22/actions/domtreeviewer.png"));
	}

}

void TrayApp::showStateMessage(QString text)
{
	showMessage("System update monitor", text);
}
