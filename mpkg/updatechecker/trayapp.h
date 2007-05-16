/************************************
 * MOPSLinux package management system
 * Updates monitor - main header file
 * $Id: trayapp.h,v 1.4 2007/05/16 01:38:32 i27249 Exp $
 */

#ifndef TRAYAPP_H_
#define TRAYAPP_H_
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include "monitor.h"
class TrayApp: public QSystemTrayIcon
{
	Q_OBJECT
	public:
		TrayApp();
		~TrayApp();
	public slots:
		void showUpdateMessage(bool hasUpdates);
		void showStateMessage(QString text);
		void quitApp();
	signals:
		void forceCheck();
		void execThread();
		void shutdownThread();
		void pauseThread();
	private:
		QMenu *appMenu;
		QAction *quitAction;
		QAction *mergeUpdatesAction;
		QAction *launchManagerAction;
		QAction *forceUpdateAction;
		mpkg *core;
		monitorThread *mThread;
};

#endif //TRAYAPP_H_
