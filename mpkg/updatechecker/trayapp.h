/************************************
 * MOPSLinux package management system
 * Updates monitor - main header file
 * $Id: trayapp.h,v 1.1 2007/05/15 22:28:58 i27249 Exp $
 */

#ifndef TRAYAPP_H_
#define TRAYAPP_H_
#include <QSystemTrayIcon>
#include <mpkg/libmpkg.h>
class TrayApp: public QSystemTrayIcon
{
	Q_OBJECT
	public:
		TrayApp();
		~TrayApp();
	private:
		mpkg *core;
};

#endif //TRAYAPP_H_
