/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.4 2007/02/18 03:10:34 i27249 Exp $
 * ***************************************************************/

#include <QTextCodec>
#include <QtGui>
#include "mainwindow.h"
#include <QDir>
//#include <mpkg/libmpkg.h>
#include <QFileDialog>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>
#include "aboutbox.h"
#include "preferencesbox.h"
#include "loading.h"
#include <stdio.h>
MainWindow::MainWindow(QMainWindow *parent)
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	ui.setupUi(this);

	mDb = new mpkg;
	loadBox = new LoadingBox;
	if (!mDb->init_ok)
	{
		QMessageBox::critical(this, tr("MOPSLinux package manager"),
				tr("Database initialization error!\nCheck your settings"),
				QMessageBox::Ok, QMessageBox::Ok);
		delete mDb;
		qApp->quit();
	}
	loadData();
}

void MainWindow::showPreferences()
{
	PreferencesBox *prefbox = new PreferencesBox;
	prefbox->loadData(mDb);
	prefbox->openInterface();
}

void MainWindow::showAbout()
{
	printf("abox show!\n");
	AboutBox *abox = new AboutBox;
	abox->show();

}
void MainWindow::quitApp(){}
void MainWindow::showCoreSettings(){}
void MainWindow::commitChanges(){}
void MainWindow::resetChanges(){}
void MainWindow::resetQueue(){}
void MainWindow::saveQueue(){}
void MainWindow::showAddRemoveRepositories(){}
void MainWindow::showCustomFilter(){}
void MainWindow::setInstalledFilter(bool showThis){}
void MainWindow::setAvailableFilter(bool showThis){}
void MainWindow::setBrokenFilter(bool showThis){}
void MainWindow::setUnavailableFilter(bool showThis){}
void MainWindow::setRemovedFilter(bool showThis){}
void MainWindow::showHelpTopics(){}
void MainWindow::showFaq(){}
void MainWindow::loadData()
{


	SQLRecord sqlSearch;
	PACKAGE_LIST packagelist;
	if (mDb->get_packagelist(sqlSearch, &packagelist, true)!=0)
	{
		QMessageBox::critical(this, tr("MOPSLinux package manager"),
				tr("Error querying package list.\nPossibly database is damaged"),
				QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	ui.packageTable->setSortingEnabled(false);
	unsigned int count = packagelist.size();
	loadBox->show();
	ui.progressBar->hide();
	loadBox->ui.progressBar->setValue(0);
	loadBox->ui.progressBar->setMaximum(count);

	for (unsigned int i=0; i<packagelist.size(); i++)
	{
		ui.packageTable->insertRow(i);
		ui.packageTable->setItem(i,0, new QTableWidgetItem(packagelist.get_package(i)->get_vstatus().c_str()));
		ui.packageTable->setItem(i,1, new QTableWidgetItem(packagelist.get_package(i)->get_name().c_str()));
		ui.packageTable->setItem(i,2, new QTableWidgetItem(packagelist.get_package(i)->get_version().c_str()));
		ui.packageTable->setItem(i,3, new QTableWidgetItem(packagelist.get_package(i)->get_arch().c_str()));
		ui.packageTable->setItem(i,4, new QTableWidgetItem(packagelist.get_package(i)->get_build().c_str()));
		ui.packageTable->setItem(i,5, new QTableWidgetItem("!"));
		ui.packageTable->setItem(i,6, new QTableWidgetItem(packagelist.get_package(i)->get_short_description().c_str()));
		loadBox->ui.progressBar->setValue(i);
	}
	ui.packageTable->setSortingEnabled(true);
	
	loadBox->hide();
	this->show();

}




       	
