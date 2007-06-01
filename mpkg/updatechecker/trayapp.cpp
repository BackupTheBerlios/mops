/*************************************************************
 * MOPSLinux package management system
 * Updates monitor - main header file
 * $Id: trayapp.cpp,v 1.10 2007/06/01 03:53:16 i27249 Exp $
 ************************************************************/

#include "trayapp.h"

TrayApp::TrayApp()
{
	//core = new mpkg(false);
	mThread = new monitorThread;
	mThread->start();

	appMenu = new QMenu;
	appMenu->setTitle(tr("Updates monitor"));

	forceUpdateAction = appMenu->addAction(QIcon("/opt/kde/share/icons/crystalsvg/48x48/actions/reload.png"),\
			tr("Check now"));
	appMenu->addSeparator();
	mergeUpdatesAction = appMenu->addAction(QIcon("/opt/kde/share/icons/crystalsvg/48x48/actions/vcs_update.png"),\
		       tr("Import changes into database"));
	launchManagerAction = appMenu->addAction(QIcon("/usr/share/mpkg/icons/icons/crystalsvg/128x128/apps/package_applications.png"),\
		       tr("Launch package manager"));
	appMenu->addSeparator();
	quitAction = appMenu->addAction(QIcon("/opt/kde/share/icons/crystalsvg/48x48/actions/exit.png"),tr("Quit"));
	setContextMenu(appMenu);


	QObject::connect(this, SIGNAL(execThread()), mThread, SLOT(enable()));
	QObject::connect(this, SIGNAL(shutdownThread()), mThread, SLOT(shutdown()));
	QObject::connect(mThread, SIGNAL(updatesDetected(bool)), this, SLOT(showUpdateMessage(bool)));
	QObject::connect(mThread, SIGNAL(showStateMessage(QString)), this, SLOT(showStateMessage(QString)));
	QObject::connect(this, SIGNAL(forceCheckSig()), mThread, SLOT(forceCheck()));
	QObject::connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApp()));
	QObject::connect(launchManagerAction, SIGNAL(triggered()), this, SLOT(launchManager()));
	QObject::connect(mergeUpdatesAction, SIGNAL(triggered()), this, SLOT(mergeUpdates()));
	QObject::connect(forceUpdateAction, SIGNAL(triggered()), this, SLOT(forceCheck()));
	QObject::connect(this, SIGNAL(launchManagerSig()), mThread, SLOT(launchManager()));
	QObject::connect(this, SIGNAL(mergeUpdatesSig()), mThread, SLOT(mergeUpdates()));
	QObject::connect(this, SIGNAL(forceCheckSig()), mThread, SLOT(forceCheck()));

	emit execThread();
	emit forceCheckSig();
	setToolTip(tr("Updates monitor"));
	setIcon(QIcon("/usr/share/mpkg/icons/available.png"));
	show();
}

void TrayApp::forceCheck()
{
	emit forceCheckSig();
}

void TrayApp::launchManager()
{
	emit launchManagerSig();
}

void TrayApp::mergeUpdates()
{
	emit mergeUpdatesSig();
}

void TrayApp::quitApp()
{
	hide();
	emit shutdownThread();
	mThread->wait();
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
		showMessage(tr("System update monitor"), tr("New updates available"));
	}
	else 
	{
		setIcon(QIcon("/usr/share/mpkg/icons/installed.png"));
	}

}

void TrayApp::showStateMessage(QString text)
{
	showMessage(tr("System update monitor"), text);
}
