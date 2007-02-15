/*****************************************************
 * MOPSLinux packaging system
 * Package builder - header
 * $Id: mainwindow.h,v 1.4 2007/02/15 13:19:19 i27249 Exp $
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
		void changeHeader();
		void swapLanguage();
		void storeCurrentDescription();
		void quitApp();
	public:
		Ui::Form ui;
	private:
		std::vector<QString> short_description;
		std::vector<QString> description;
		QString xmlFilename;
		bool modified;
};
#endif
