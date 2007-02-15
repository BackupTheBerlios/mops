/****************************************************************
 *     MOPSLinux packaging system
 *     Package builder - main file
 *     $Id: main.cpp,v 1.7 2007/02/15 14:28:22 i27249 Exp $
 ***************************************************************/

#include <QApplication>
 #include <QPushButton>
 #include <QListWidget>
 #include <QWidget>
#include "mainwindow.h"
 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);

     Form mw;

	mw.show();
	QObject::connect(mw.ui.LoadButton, SIGNAL(clicked()), &mw, SLOT(loadData()));
	QObject::connect(mw.ui.TagAddButton, SIGNAL(clicked()), &mw, SLOT(addTag()));
	QObject::connect(mw.ui.TagDeleteButton, SIGNAL(clicked()), &mw, SLOT(deleteTag()));
	QObject::connect(mw.ui.DepAddButton, SIGNAL(clicked()), &mw, SLOT(addDependency()));
	QObject::connect(mw.ui.BuildButton, SIGNAL(clicked()), &mw, SLOT(saveData()));
	QObject::connect(mw.ui.DepDeleteButton, SIGNAL(clicked()), &mw, SLOT(deleteDependency()));
	QObject::connect(mw.ui.NameEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.VersionEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.BuildEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.ArchComboBox, SIGNAL(currentIndexChanged(const QString &)), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.quitButton, SIGNAL(clicked()), &mw, SLOT(quitApp()));
	QObject::connect(mw.ui.DescriptionLanguageComboBox, SIGNAL(currentIndexChanged(const QString &)), &mw, SLOT(swapLanguage()));
	QObject::connect(mw.ui.ShortDescriptionEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.DescriptionEdit, SIGNAL(textChanged()), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.ChangelogEdit, SIGNAL(textChanged()), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.TagEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader(const QString &)));
	QObject::connect(mw.ui.DepAddButton, SIGNAL(clicked()), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.DepDeleteButton, SIGNAL(clicked()), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.TagAddButton, SIGNAL(clicked()), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.TagDeleteButton, SIGNAL(clicked()), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.MaintainerNameEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader()));
	QObject::connect(mw.ui.MaintainerMailEdit, SIGNAL(textChanged(const QString &)), &mw, SLOT(changeHeader()));










     return app.exec();
 } 
