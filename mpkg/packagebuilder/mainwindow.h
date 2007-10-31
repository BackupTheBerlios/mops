/*****************************************************
 * MOPSLinux packaging system
 * Package builder - header
 * $Id: mainwindow.h,v 1.19 2007/10/31 01:52:38 i27249 Exp $
 * ***************************************************/

#ifndef MV_H
#define MV_H
#include "ui_package_builder.h"
#include <mpkg/libmpkg.h>
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
		Form (QWidget *parent = 0, string arg="");
	public slots:
		void showAbout();
		void loadData();
		void loadFile(QString filename);
		void saveFile();
		void addConfigFile();
		void deleteConfigFile();
		void searchConfigFile();
		void searchTempFile();
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
		void switchEnvField(int state);
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

		void embedSources();
		void analyzeSources();

		void editBuildScriptWithGvim();
		void loadBuildScriptFromFile();
	public:
		Ui::Form ui;
	private:
		string tempScriptEditFile;
		string _arg;
		std::vector<QString> short_description;
		std::vector<QString> description;
		bool modified;
		vector<string> patchList;
		vector<keys> keyList;

		BinaryPackage binaryPackage;
		SourcePackage sourcePackage;
		int dataType;
};
#endif
