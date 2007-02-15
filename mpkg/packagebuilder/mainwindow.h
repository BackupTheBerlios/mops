/*****************************************************
 * MOPSLinux packaging system
 * Package builder - header
 * $Id: mainwindow.h,v 1.3 2007/02/15 09:48:40 i27249 Exp $
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
		void deleteDependency();
		void changeHeader(const QString & text);
	public:
		Ui::Form ui;
};
#endif
