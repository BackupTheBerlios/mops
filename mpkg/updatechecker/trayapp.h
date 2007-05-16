/************************************
 * MOPSLinux package management system
 * Updates monitor - main header file
 * $Id: trayapp.h,v 1.2 2007/05/16 00:07:29 i27249 Exp $
 */

#ifndef TRAYAPP_H_
#define TRAYAPP_H_
#include <QSystemTrayIcon>
#include "monitor.h"
class TrayApp: public QSystemTrayIcon
{
	Q_OBJECT
	public:
		TrayApp();
		~TrayApp();
	public slots:
		void showUpdateMessage();
	signals:
		void execThread();
		void shutdownThread();
		void pauseThread();
	private:
		mpkg *core;
		monitorThread *mThread;
};

#endif //TRAYAPP_H_
