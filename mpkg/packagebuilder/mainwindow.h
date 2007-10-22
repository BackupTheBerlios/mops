/*****************************************************
 * MOPSLinux packaging system
 * Package builder - header
 * $Id: mainwindow.h,v 1.13 2007/10/22 23:12:27 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_package_builder.h"
#include <mpkg/libmpkg.h>
#include "package.h"
typedef enum {
       	TYPE_NONE=0,
	TYPE_TGZ,
	TYPE_XML
} TargetType;
using namespace std;
typedef struct keys
{
	string name;
	string value;
};


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
		void changeHeader(const QString &);
		void saveAndExit();
		void changeHeader();
		void swapLanguage();
		void storeCurrentDescription();
		void addDepsFromFiles();
		void quitApp();

		void switchConfigureField(int state);
		void switchCompilationField(int state);
		void switchInstallField(int state);
		void switchSourceDirectoryField(int state);
		void switchBuildSystem(int index);
		void switchCpuArchField(int state);
		void switchCpuTuneField(int state);
		void switchOptimizationField(int state);
		void switchGccOptionsField(int state);
		void displayKeys();
		void displayPatches();
		void addKey();
		void addPatch();
		void deletePatch();
		void deleteKey();
	public:
		Ui::Form ui;
		bool xmlExists;
	private:
		TargetType _type;
		string _arg;
		std::vector<QString> short_description;
		std::vector<QString> description;
		QString xmlFilename;
		QString pkgRoot;
		std::string _tmpdir;
		bool modified;
		vector<string> patchList;
		vector<keys> keyList;
};
#endif
