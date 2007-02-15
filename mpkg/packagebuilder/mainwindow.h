/*****************************************************
 * MOPSLinux packaging system
 * Package builder - header
 * $Id: mainwindow.h,v 1.2 2007/02/15 08:38:20 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_package_builder.h"

class Form: public QWidget
{
	Q_OBJECT
	public:
		Form (QWidget *parent = 0);
	public slots:
		void loadData();
		void saveData();
		void addTag();
		void addDependency();
		void deleteTag();
	public:
		Ui::Form ui;
};
#endif
