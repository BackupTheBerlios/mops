/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.10 2007/02/21 16:01:28 i27249 Exp $
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
#include "db.h"
#include <stdio.h>
MainWindow::MainWindow(QMainWindow *parent)
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	ui.setupUi(this);

	dbBox = new DatabaseBox;
	mDb = dbBox->mDb;
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

void MainWindow::resetQueue()
{
	mDb->clean_queue();
	loadData();
}

void MainWindow::showPackageInfo()
{
	long id = ui.packageTable->item(ui.packageTable->currentRow(), PT_ID)->text().toLong();
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

    if (ui.packageTable->columnCount() < 9)
    	ui.packageTable->setColumnCount(9);
    QTableWidgetItem *__colItem0 = new QTableWidgetItem();
    __colItem0->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_INSTALLCHECK, __colItem0);

    QTableWidgetItem *__colItem = new QTableWidgetItem();
    __colItem->setText(QApplication::translate("MainWindow", "Status", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_STATUS, __colItem);

    QTableWidgetItem *__colItem1 = new QTableWidgetItem();
    __colItem1->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_NAME, __colItem1);

    QTableWidgetItem *__colItem2 = new QTableWidgetItem();
    __colItem2->setText(QApplication::translate("MainWindow", "Version", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_VERSION, __colItem2);

    QTableWidgetItem *__colItem3 = new QTableWidgetItem();
    __colItem3->setText(QApplication::translate("MainWindow", "Arch", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_ARCH, __colItem3);

    QTableWidgetItem *__colItem4 = new QTableWidgetItem();
    __colItem4->setText(QApplication::translate("MainWindow", "Build", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_BUILD, __colItem4);

    QTableWidgetItem *__colItem5 = new QTableWidgetItem();
    __colItem5->setText(QApplication::translate("MainWindow", "Avail.", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_MAXAVAILABLE, __colItem5);

    QTableWidgetItem *__colItem6 = new QTableWidgetItem();
    __colItem6->setText(QApplication::translate("MainWindow", "Info", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_INFO, __colItem6);

    QTableWidgetItem *__colItem7 = new QTableWidgetItem();
    __colItem7->setText(QApplication::translate("MainWindow", "ID", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_ID, __colItem7);

    QTableWidgetItem *__colItem8 = new QTableWidgetItem();
    __colItem8->setText(QApplication::translate("MainWindow", "Other data", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(9, __colItem8);
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
	initPackageTable();
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
void MainWindow::commitChanges()
{
	for (unsigned int i = 0; i< newStatus.size(); i++)
	{
		if (packagelist.get_package(i)->get_status()!=newStatus[i])
		{
			switch(newStatus[i])
			{
				case PKGSTATUS_INSTALL:
					install_queue.push_back(packagelist.get_package(i)->get_name());
					break;
				case PKGSTATUS_REMOVE:
					remove_queue.push_back(packagelist.get_package(i)->get_name());
					break;
			}
		}
	}
	printf("install_queue size = %d\n", install_queue.size());
	mDb->uninstall(remove_queue);
	mDb->install(install_queue);
	ui.statusbar->showMessage("Committing changes...");
	mDb->commit();
	mDb = dbBox->recreateDb();
	install_queue.clear();
	remove_queue.clear();
	loadData();
	ui.statusbar->showMessage("All operations completed");
}
void MainWindow::resetChanges(){}
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

void MainWindow::markChanges(int x, Qt::CheckState state)
{
	printf("markChanges! x = %d, newStatus size = %d\n", x, newStatus.size());
		long i = ui.packageTable->item(x, PT_ID)->text().toLong();
		if (i >= newStatus.size())
		{
			printf("i is out of range: i=%d, max = %d\n", i, packagelist.size());
			return;
		}
		switch(packagelist.get_package(i)->get_status())
		{
			case PKGSTATUS_AVAILABLE:
				if (state == Qt::Checked)
				{
					newStatus[i]=PKGSTATUS_INSTALL;
					ui.statusbar->showMessage("Package added to install queue");
				}
				else
				{
					newStatus[i]=PKGSTATUS_AVAILABLE;
					ui.statusbar->showMessage("Package removed from install queue");
				}
				printf("status set\n");
				break;
			case PKGSTATUS_REMOVED_AVAILABLE:
				if (state == Qt::Checked)
				{
					newStatus[i]=PKGSTATUS_INSTALL;
					ui.statusbar->showMessage("Package added to install queue");
				}
				else
				{
					newStatus[i]=PKGSTATUS_REMOVED_AVAILABLE;
					ui.statusbar->showMessage("Package removed from install queue");
				}
				printf("status set\n");
				break;

			case PKGSTATUS_INSTALLED:
				if (state != Qt::Checked)
				{
					newStatus[i]=PKGSTATUS_PURGE;
					ui.statusbar->showMessage("Package added to remove queue");
				}
				else
				{
					newStatus[i]=PKGSTATUS_INSTALLED;
					ui.statusbar->showMessage("Package removed from remove queue");
				}
				printf("ins set\n");
				break;
		}
}
void MainWindow::insertPackageIntoTable(unsigned int package_num)
{
	unsigned int i = ui.packageTable->rowCount();
	ui.packageTable->insertRow(i);
	CheckBox *stat = new CheckBox(this);
	stat->row = package_num;
	if (packagelist.get_package(package_num)->get_vstatus().find("INSTALLED") != std::string::npos && \
			packagelist.get_package(package_num)->get_vstatus().find("INSTALL") != std::string::npos) stat->setCheckState(Qt::Checked);
	ui.packageTable->setCellWidget(i,PT_INSTALLCHECK, stat);
	QObject::connect(stat, SIGNAL(stateChanged(int)), stat, SLOT(markChanges()));
	ui.packageTable->setItem(i,PT_STATUS, new QTableWidgetItem(packagelist.get_package(package_num)->get_vstatus().c_str()));
	ui.packageTable->setItem(i,PT_NAME, new QTableWidgetItem(packagelist.get_package(package_num)->get_name().c_str()));
	ui.packageTable->setItem(i,PT_VERSION, new QTableWidgetItem(packagelist.get_package(package_num)->get_version().c_str()));
	ui.packageTable->setItem(i,PT_ARCH, new QTableWidgetItem(packagelist.get_package(package_num)->get_arch().c_str()));
	ui.packageTable->setItem(i,PT_BUILD, new QTableWidgetItem(packagelist.get_package(package_num)->get_build().c_str()));
	ui.packageTable->setItem(i,PT_MAXAVAILABLE, new QTableWidgetItem("!"));
	ui.packageTable->setItem(i,PT_INFO, new QTableWidgetItem(packagelist.get_package(package_num)->get_short_description().c_str()));
	ui.packageTable->setItem(i,PT_ID, new QTableWidgetItem(IntToStr(package_num).c_str()));
}

void MainWindow::searchPackagesByTag(QString tag)
{
	ui.packageTable->clearContents();
	ui.packageTable->setRowCount(0);

	for (unsigned int i=0; i<packagelist.size(); i++)
	{
		for (unsigned int t=0; t<packagelist.get_package(i)->get_tags()->size(); t++)
		{
			if (packagelist.get_package(i)->get_tags()->get_tag(t)->get_name() == tag.toStdString())
			{
				insertPackageIntoTable(i);
			}
		}
		setBarValue(ui.progressBar, i);
	}
}

void MainWindow::loadData(bool internal)
{


	SQLRecord sqlSearch;
	packagelist.clear();
	//stateChanged.clear();
	newStatus.clear();
	
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
	ui.packageTable->setRowCount(0);
	for (unsigned int i=0; i<packagelist.size(); i++)
	{
	//	stateChanged.push_back(false);
		newStatus.push_back(packagelist.get_package(i)->get_status());
		insertPackageIntoTable(i);
		if (internal) setBarValue(ui.progressBar, i);
		else setBarValue(loadBox->ui.progressBar, i);
	}
	
	if (internal) ui.progressBar->hide();
	else loadBox->hide();
	this->show();
	fitTable();
	//printf("stCh.size = %d, plist.size = %d\n", stateChanged.size(), packagelist.size());
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
	//ui.packageTable->item(i,0)->setCheckState(Qt::Checked);
/*	if (ui.packageTable->item(i,0)->text() == "AVAILABLE" || ui.packageTable->item(i,0)->text()=="REMOVED_AVAILABLE")
	{
i		ui.packageTable->item(ui.packageTable->currentRow(),0)->setText("INSTALL");
	}*/
}

void CheckBox::markChanges()
{
	printf("row = %d\n",row);
	mw->markChanges(row, checkState());
}

CheckBox::CheckBox(MainWindow *parent)
{
	mw = parent;
}
