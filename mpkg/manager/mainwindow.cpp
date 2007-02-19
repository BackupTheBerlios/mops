/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.8 2007/02/19 10:09:32 i27249 Exp $
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
	tableMenu = new QMenu;
	installPackageAction = tableMenu->addAction(tr("Install"));
	removePackageAction = tableMenu->addAction(tr("Remove"));
	purgePackageAction = tableMenu->addAction(tr("Purge"));
	upgradePackageAction = tableMenu->addAction(tr("Upgrade"));
	QObject::connect(installPackageAction, SIGNAL(triggered()), this, SLOT(markToInstall()));
	QObject::connect(ui.packageTable, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(execMenu()));
	if (!mDb->init_ok)
	{
		QMessageBox::critical(this, tr("MOPSLinux package manager"),
				tr("Database initialization error!\nCheck your settings"),
				QMessageBox::Ok, QMessageBox::Ok);
		delete mDb;
		qApp->quit();
	}
	loadData(false);
	initPackageTable();
}

void MainWindow::showPackageInfo()
{
	long id = ui.packageTable->item(ui.packageTable->currentRow(), 7)->text().toLong();
	PACKAGE *pkg = packagelist.get_package(id);
	string info = "<html><h1>"+pkg->get_name()+" "+pkg->get_version()+"</h1><p><b>Architecture:</b> "+pkg->get_arch()+"<br><b>Build:</b> "+pkg->get_build();
	info += "<br><b>Description: </b><br>"+pkg->get_description()+"</p></html>";
	
	ui.visualInfoTextEdit->setHtml(info.c_str());
}

void MainWindow::execMenu()
{
	
	tableMenu->exec(QCursor::pos());
}

void MainWindow::initPackageTable()
{
	//QCheckBox *chh = new QCheckBox;
	//ui.packageTable->setCellWidget(0,0,chh);
	//ui.packageTable->resizeColumnsToContents();
	//ui.packageTable->resizeRowsToContents();
}

void MainWindow::fitTable()
{
	ui.packageTable->resizeColumnsToContents();
	ui.packageTable->resizeRowsToContents();
}

void MainWindow::showPreferences()
{
	PreferencesBox *prefbox = new PreferencesBox(mDb);
	prefbox->loadData();
	prefbox->openInterface();
}

void MainWindow::showAbout()
{
	printf("abox show!\n");
	AboutBox *abox = new AboutBox;
	abox->show();

}

void MainWindow::clearForm()
{
	ui.packageTable->clear();
	ui.packageTable->setRowCount(0);
}

void MainWindow::updateData()
{
	clearForm();
	mDb->update_repository_data();
	loadData(true);
}
	
void MainWindow::quitApp()
{
	delete mDb;
	qApp->quit();

}
void MainWindow::showCoreSettings()
{
}
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

void MainWindow::loadData(bool internal)
{


	SQLRecord sqlSearch;
	packagelist.clear();
	
	if (mDb->get_packagelist(sqlSearch, &packagelist, true)!=0)
	{
		QMessageBox::critical(this, tr("MOPSLinux package manager"),
				tr("Error querying package list.\nProbably database is damaged"),
				QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	ui.packageTable->setSortingEnabled(false);
	unsigned int count = packagelist.size();
	if (internal)
	{
		initProgressBar(ui.progressBar);
		ui.progressBar->show();
	}
	else {
		initProgressBar(loadBox->ui.progressBar, count);
		ui.progressBar->hide();
		loadBox->show();
	}
	ui.packageTable->clearContents();
	ui.packageTable->setRowCount(1);
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
		ui.packageTable->setItem(i,7, new QTableWidgetItem(IntToStr(i).c_str()));
		if (internal) setBarValue(ui.progressBar, i);
		else setBarValue(loadBox->ui.progressBar, i);
	}
	ui.packageTable->setSortingEnabled(true);
	
	if (internal) ui.progressBar->hide();
	else loadBox->hide();
	this->show();
	fitTable();

}

void MainWindow::initProgressBar(QProgressBar *Bar, int stepCount)
{
	Bar->setValue(0);
	Bar->setMaximum(stepCount);
}

void MainWindow::setBarValue(QProgressBar *Bar, int stepValue)
{
	Bar->setValue(stepValue);
}

void MainWindow::markToInstall()
{
	int i = ui.packageTable->currentRow();
	if (ui.packageTable->item(i,0)->text() == "AVAILABLE" || ui.packageTable->item(i,0)->text()=="REMOVED_AVAILABLE")
	{
		// Build Dep tree
		
		//Interface changes
		ui.packageTable->item(ui.packageTable->currentRow(),0)->setText("INSTALL");
	}
}
