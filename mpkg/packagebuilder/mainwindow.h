/*****************************************************
 * MOPSLinux packaging system
 * Package builder - header
 * $Id: mainwindow.h,v 1.8 2007/08/24 06:20:52 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_package_builder.h"
#include <string>
typedef enum {
       	TYPE_NONE=0,
	TYPE_TGZ,
	TYPE_XML
} TargetType;
using namespace std;
class Form: public QWidget
{
	Q_OBJECT
	public:
		Form (QWidget *parent = 0, TargetType type=TYPE_NONE, string arg="");
	public slots:
		void loadData();
		void saveData();
		void addConfigFile();
		void deleteConfigFile();
		void searchConfigFile();
		void addTag();
		void addDependency();
		void deleteTag();
		void deleteDependency();
		void changeHeader(const QString & text);
		void saveAndExit();
		void changeHeader();
		void swapLanguage();
		void storeCurrentDescription();
		void addDepsFromFiles();
		void quitApp();
	public:
		Ui::Form ui;
	private:
		TargetType _type;
		string _arg;
		std::vector<QString> short_description;
		std::vector<QString> description;
		QString xmlFilename;
		QString pkgRoot;
		std::string _tmpdir;
		bool modified;
};
#endif
