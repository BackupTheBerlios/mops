/****************************************************************
 *     MOPSLinux packaging system
 *     Package builder - main file
 *     $Id: main.cpp,v 1.16 2007/11/22 15:32:56 i27249 Exp $
 ***************************************************************/

#include <QApplication>
 #include <QPushButton>
 #include <QListWidget>
 #include <QWidget>
#include "mainwindow.h"
#include <QtGui>
#include <string>
using namespace std;
int main(int argc, char *argv[])
{
	// Now we can pass as argument:
	// 	package archive filename
	// 	package index data.xml
	if (getuid()!=0) {
		string args;
		for (unsigned int i=0; i<argc; i++)
		{
			args+=(string) argv[i] + " ";
		}
		return system("kdesu " + args);
		
	}
	QApplication app(argc, argv);
	QTranslator translator;
	translator.load("/usr/share/mpkg/packagebuilder_ru");
	app.installTranslator(&translator);

	string arg;
	if (argc == 2) arg=argv[1];
	
     	Form *mw=new Form(0,arg);


	//mw->show();
	QObject::connect(mw->ui.addConfFileButton, SIGNAL(clicked()), mw, SLOT(addConfigFile()));
	QObject::connect(mw->ui.delConfFileButton, SIGNAL(clicked()), mw, SLOT(deleteConfigFile()));
	QObject::connect(mw->ui.conffileSearchButton, SIGNAL(clicked()), mw, SLOT(searchConfigFile()));
	QObject::connect(mw->ui.LoadButton, SIGNAL(clicked()), mw, SLOT(loadData()));
	QObject::connect(mw->ui.TagAddButton, SIGNAL(clicked()), mw, SLOT(addTag()));
	QObject::connect(mw->ui.TagDeleteButton, SIGNAL(clicked()), mw, SLOT(deleteTag()));
	QObject::connect(mw->ui.DepAddButton, SIGNAL(clicked()), mw, SLOT(addDependency()));
	QObject::connect(mw->ui.BuildButton, SIGNAL(clicked()), mw, SLOT(saveFile()));
	QObject::connect(mw->ui.DepDeleteButton, SIGNAL(clicked()), mw, SLOT(deleteDependency()));
	QObject::connect(mw->ui.NameEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw->ui.VersionEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw->ui.BuildEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw->ui.ArchComboBox, SIGNAL(currentIndexChanged(const QString &)), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.quitButton, SIGNAL(clicked()), mw, SLOT(quitApp()));
	//QObject::connect(mw->ui.DescriptionLanguageComboBox, SIGNAL(currentIndexChanged(const QString &)), mw, SLOT(swapLanguage()));
	QObject::connect(mw->ui.ShortDescriptionEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw->ui.DescriptionEdit, SIGNAL(textChanged()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.ChangelogEdit, SIGNAL(textChanged()), mw, SLOT(changeHeader()));
//	QObject::connect(mw->ui.TagEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw->ui.DepAddButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.DepDeleteButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.TagAddButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.TagDeleteButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.MaintainerNameEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.MaintainerMailEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader()));










     return app.exec();
 } 
