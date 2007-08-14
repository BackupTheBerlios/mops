/****************************************************************
 *     MOPSLinux packaging system
 *     Package builder - main file
 *     $Id: main.cpp,v 1.11 2007/08/14 14:29:54 i27249 Exp $
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

	QApplication app(argc, argv);
	QTranslator translator;
	translator.load("/usr/share/mpkg/packagebuilder_ru");
	app.installTranslator(&translator);

     	Form *mw;
	if (argc == 2)
	{
		// Mean that we have such argument
		string arg = argv[1];
		if (arg.find("data.xml")==arg.length()-strlen("data.xml"))
		{
			mw == new Form(0,TYPE_XML, arg);
			printf("data.xml found\n");
			// We have data.xml as argument
		}
		if (arg.find(".tgz")==arg.length()-strlen(".tgz"))
		{
			mw = new Form(0,TYPE_TGZ, arg);
			printf(".tgz found\n");
			// We have an archive as argument
		}
	}
	else mw = new Form;


	//mw->show();
	QObject::connect(mw->ui.addConfFileButton, SIGNAL(clicked()), mw, SLOT(addConfigFile()));
	QObject::connect(mw->ui.delConfFileButton, SIGNAL(clicked()), mw, SLOT(deleteConfigFile()));
	QObject::connect(mw->ui.conffileSearchButton, SIGNAL(clicked()), mw, SLOT(searchConfigFile()));
	QObject::connect(mw->ui.LoadButton, SIGNAL(clicked()), mw, SLOT(loadData()));
	QObject::connect(mw->ui.TagAddButton, SIGNAL(clicked()), mw, SLOT(addTag()));
	QObject::connect(mw->ui.TagDeleteButton, SIGNAL(clicked()), mw, SLOT(deleteTag()));
	QObject::connect(mw->ui.DepAddButton, SIGNAL(clicked()), mw, SLOT(addDependency()));
	QObject::connect(mw->ui.BuildButton, SIGNAL(clicked()), mw, SLOT(saveData()));
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
	QObject::connect(mw->ui.TagEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw->ui.DepAddButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.DepDeleteButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.TagAddButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.TagDeleteButton, SIGNAL(clicked()), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.MaintainerNameEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader()));
	QObject::connect(mw->ui.MaintainerMailEdit, SIGNAL(textChanged(const QString &)), mw, SLOT(changeHeader()));










     return app.exec();
 } 
