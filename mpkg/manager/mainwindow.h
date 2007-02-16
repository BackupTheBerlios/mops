/*****************************************************
 * MOPSLinux packaging system
 * Package manager UI - header
 * $Id: mainwindow.h,v 1.1 2007/02/16 01:36:36 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_pkgmanager.h"

class MainWindow: public QMainWindow
{
	Q_OBJECT
	public:
		MainWindow (QMainWindow *parent = 0);
	//public slots:
	public:
		Ui::MainWindow ui;
//	private:
};
#endif
