/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.26 2007/03/18 04:37:52 i27249 Exp $
 * ***************************************************************/

#include <QTextCodec>
#include <QtGui>
#include "mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>
#include "aboutbox.h"
#include "preferencesbox.h"
#include "loading.h"
#include "db.h"
#include <stdio.h>
#include "tablelabel.h"
#include <unistd.h>

void MainWindow::errorLoadingDatabase()
{
	QMessageBox::critical(this, tr("MOPSLinux package manager"),
				tr("Database initialization error!\nCheck your settings"),
				QMessageBox::Ok, QMessageBox::Ok);

}

void MainWindow::sqlQueryBegin()
{
	ui.statusbar->showMessage("Loading database...");
}

void MainWindow::sqlQueryEnd()
{
	ui.statusbar->showMessage("Data loaded, rendering visuals");
}

void MainWindow::loadingStarted()
{
	ui.statusbar->showMessage("LoadingStarted");
}

void MainWindow::loadingFinished()
{
	ui.statusbar->showMessage("LoadingFinished");
}

void MainWindow::enableProgressBar()
{
	ui.progressBar->show();
}

void MainWindow::disableProgressBar()
{
	ui.progressBar->hide();
}

void MainWindow::setProgressBarValue(unsigned int value)
{
	ui.progressBar->setValue(value);
}


void MainWindow::clearTable()
{
	ui.packageTable->clearContents();
	ui.packageTable->setRowCount(0);
}

void MainWindow::setTableSize(unsigned int size)
{
	ui.packageTable->setRowCount(size);
}

void MainWindow::setTableItem(unsigned int row, bool checkState, string cellItemText)
{
	CheckBox *stat = new CheckBox(this);
	if (checkState) stat->setCheckState(Qt::Checked);
	ui.packageTable->setCellWidget(row,PT_INSTALLCHECK, stat);

	TableLabel *pkgName = new TableLabel(ui.packageTable);
	pkgName->setTextFormat(Qt::RichText);
	pkgName->setText(cellItemText.c_str());
	pkgName->row = row;
	ui.packageTable->setCellWidget(row, PT_NAME, pkgName);
	stat->row = row;
	QObject::connect(stat, SIGNAL(stateChanged(int)), stat, SLOT(markChanges()));
	ui.packageTable->setItem(row,PT_ID, new QTableWidgetItem(IntToStr(row).c_str()));
	ui.packageTable->setRowHeight(row-1, 45);

}


void MainWindow::setTableItemVisible(unsigned int row, bool visible)
{
	ui.packageTable->setRowHidden(row, visible);
}







MainWindow::MainWindow(QMainWindow *parent)
{
	if (getuid()!=0)
	{
		QMessageBox::critical(this, tr("MOPSLinux package manager"),
                   tr("You need to be root to run package manager"),
                   QMessageBox::Ok,
                   QMessageBox::Ok);
		exit(0);

	}

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	ui.setupUi(this);

	clearForm();
	disableProgressBar();
	//dbBox = new DatabaseBox;
	//mDb = dbBox->mDb;
	//loadBox = new LoadingBox;
	tableMenu = new QMenu;
	installPackageAction = tableMenu->addAction(tr("Install"));
	removePackageAction = tableMenu->addAction(tr("Remove"));
	purgePackageAction = tableMenu->addAction(tr("Purge"));
	upgradePackageAction = tableMenu->addAction(tr("Upgrade"));
	qRegisterMetaType<string>("string");
	qRegisterMetaType<PACKAGE_LIST>("PACKAGE_LIST");
	QObject::connect(installPackageAction, SIGNAL(triggered()), this, SLOT(markToInstall()));
	QObject::connect(ui.packageTable, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(execMenu()));
	
	this->show();
	thread = new coreThread;
	packagelist = new PACKAGE_LIST;
	
	// Thread connections
	QObject::connect(thread, SIGNAL(errorLoadingDatabase()), this, SLOT(errorLoadingDatabase()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(sqlQueryBegin()), this, SLOT(sqlQueryBegin()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(sqlQueryEnd()), this, SLOT(sqlQueryEnd()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(loadingStarted()), this, SLOT(loadingStarted()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(enableProgressBar()), this, SLOT(enableProgressBar()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(disableProgressBar()), this, SLOT(disableProgressBar()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setProgressBarValue(unsigned int)), this, SLOT(setProgressBarValue(unsigned int)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(fitTable()), this, SLOT(fitTable()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(clearTable()), this, SLOT(clearTable()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setTableSize(unsigned int)), this, SLOT(setTableSize(unsigned int)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setTableItem(unsigned int, bool, string)), this, SLOT(setTableItem(unsigned int, bool, string)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setTableItemVisible(unsigned int, bool)), this, SLOT(setTableItemVisible(unsigned int, bool)), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(loadPackageDatabase()), thread, SLOT(loadPackageDatabase()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(startThread()), thread, SLOT(start()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(syncData()), thread, SLOT(getPackageList()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(initProgressBar(unsigned int)), this, SLOT(initProgressBar(unsigned int))), Qt::QueuedConnection;
	QObject::connect(thread, SIGNAL(sendPackageList(PACKAGE_LIST)), this, SLOT(receivePackageList(PACKAGE_LIST)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(loadData()), this, SLOT(loadData()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(updateDatabase()), thread, SLOT(updatePackageDatabase()), Qt::QueuedConnection);
	emit startThread();
	//emit loadPackageDatabase();
}

MainWindow::~MainWindow()
{
	thread->exit();
	delete thread;
}

void MainWindow::receivePackageList(PACKAGE_LIST pkgList)
{
	*packagelist=pkgList;
}

void MainWindow::quickPackageSearch()
{
	for (unsigned int i=0; i<ui.packageTable->rowCount(); i++)
	{
		if (packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_name().find(ui.quickPackageSearchEdit->text().toStdString())==std::string::npos)
		{
			ui.packageTable->setRowHidden(i, true);
		}
		else
		{
			ui.packageTable->setRowHidden(i, false);
		}
	}
}

void MainWindow::showAllPackages()
{
	for (unsigned int i=0; i<ui.packageTable->rowCount(); i++)
	{
		ui.packageTable->setRowHidden(i,false);
	}
}
void MainWindow::resetQueue()
{
	//mDb->clean_queue();
	loadData();
}

void MainWindow::showPackageInfo()
{
	long id = ui.packageTable->item(ui.packageTable->currentRow(), PT_ID)->text().toLong();
	printf("ID = %d\n", id);
	printf("List size = %d\n", thread->getPackageList()->size());
	PACKAGE *pkg = packagelist->get_package(id);
	string info = "<html><h1>"+pkg->get_name()+" "+pkg->get_version()+"</h1><p><b>Architecture:</b> "+pkg->get_arch()+"<br><b>Build:</b> "+pkg->get_build();
	info += "<br><b>Description: </b><br>"+pkg->get_description()+"</p></html>";
	
	ui.overviewEdit->setHtml(info.c_str());
}

void MainWindow::execMenu()
{
	
	tableMenu->exec(QCursor::pos());
}

void MainWindow::initPackageTable()
{

    if (ui.packageTable->columnCount() < 4)
    	ui.packageTable->setColumnCount(4);
    QTableWidgetItem *__colItem0 = new QTableWidgetItem();
    __colItem0->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_INSTALLCHECK, __colItem0);
    ui.packageTable->setColumnWidth(PT_INSTALLCHECK, 15);

    QTableWidgetItem *__colItem = new QTableWidgetItem();
    __colItem->setText(QApplication::translate("MainWindow", "Status", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_STATUS, __colItem);
    ui.packageTable->setColumnHidden(PT_STATUS, true);

    QTableWidgetItem *__colItem1 = new QTableWidgetItem();
    __colItem1->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_NAME, __colItem1);
    ui.packageTable->setColumnWidth(PT_NAME, 600);

    QTableWidgetItem *__colItem7 = new QTableWidgetItem();
    __colItem7->setText(QApplication::translate("MainWindow", "ID", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_ID, __colItem7);
    ui.packageTable->setColumnHidden(PT_ID, true);

}

void MainWindow::fitTable()
{

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
	emit updateDatabase();
}
	
void MainWindow::quitApp()
{
	qApp->quit();

}
void MainWindow::showCoreSettings()
{
}
void MainWindow::commitChanges()
{
	for (unsigned int i = 0; i< newStatus.size(); i++)
	{
		if (packagelist->get_package(i)->get_status()!=newStatus[i])
		{
			switch(newStatus[i])
			{
				case PKGSTATUS_INSTALL:
					install_queue.push_back(packagelist->get_package(i)->get_name());
					break;
				case PKGSTATUS_REMOVE:
					remove_queue.push_back(packagelist->get_package(i)->get_name());
					break;
				case PKGSTATUS_PURGE:
					purge_queue.push_back(packagelist->get_package(i)->get_name());
					break;
				default:
					printf("Unknown status %d\n", newStatus[i]);
			}
		}
	}
	printf("install_queue size = %d\n", install_queue.size());
	printf("remove_queue size = %d\n", remove_queue.size());
	mDb->uninstall(remove_queue);
	mDb->install(install_queue);
	mDb->purge(purge_queue);
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
void MainWindow::showAddRemoveRepositories(){
	PreferencesBox *prefBox = new PreferencesBox(mDb);
	//thread.tellAreYouRunning();
	prefBox->openRepositories();
}
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
			printf("i is out of range: i=%d, max = %d\n", i, packagelist->size());
			return;
		}
		PACKAGE *_p = packagelist->get_package(i);
		switch(packagelist->get_package(i)->get_status())
		{
			case PKGSTATUS_AVAILABLE:
				if (state == Qt::Checked)
				{
					newStatus[i]=PKGSTATUS_INSTALL;
					ui.statusbar->showMessage("Package added to install queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/install.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					ui.packageTable->setCellWidget(x, PT_NAME, _z);

				}
				else
				{
					newStatus[i]=PKGSTATUS_AVAILABLE;
					ui.statusbar->showMessage("Package removed from install queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/available.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					//ui.packageTable->cellWidget(x, PT_NAME)->setText(pName.c_str());
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					ui.packageTable->setCellWidget(x, PT_NAME, _z);


				}
				printf("status set\n");
				break;
			case PKGSTATUS_REMOVED_AVAILABLE:
				if (state == Qt::Checked)
				{
					newStatus[i]=PKGSTATUS_INSTALL;
					ui.statusbar->showMessage("Package added to install queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/install.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					ui.packageTable->setCellWidget(x, PT_NAME, _z);


				}
				else
				{
					string pName = "<table><tbody><tr><td><img src = \"icons/available.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					newStatus[i]=PKGSTATUS_REMOVED_AVAILABLE;
					ui.statusbar->showMessage("Package removed from install queue");
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					ui.packageTable->setCellWidget(x, PT_NAME, _z);

				}
				printf("status set\n");
				break;

			case PKGSTATUS_INSTALLED:
				if (state != Qt::Checked)
				{
					newStatus[i]=PKGSTATUS_PURGE;
					ui.statusbar->showMessage("Package added to remove queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/purge.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					ui.packageTable->setCellWidget(x, PT_NAME, _z);


				}
				else
				{
					newStatus[i]=PKGSTATUS_INSTALLED;
					ui.statusbar->showMessage("Package removed from remove queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/installed.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";

					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					ui.packageTable->setCellWidget(x, PT_NAME, _z);

				}
				printf("ins set\n");
				break;
		}
}
void MainWindow::insertPackageIntoTable(unsigned int package_num)
{/*
	unsigned int i = package_num;
	//unsigned int i = ui.packageTable->rowCount();
	//ui.packageTable->insertRow(i);
	CheckBox *stat = new CheckBox(this);
	//QIcon *icon = new QIcon ("/home/ftp/img.png");
	string package_icon;
	if (packagelist.get_package(package_num)->get_vstatus().find("INSTALLED") != std::string::npos && \
			packagelist.get_package(package_num)->get_vstatus().find("INSTALL") != std::string::npos)
	{
		stat->setCheckState(Qt::Checked);
	}

	ui.packageTable->setCellWidget(i,PT_INSTALLCHECK, stat);

	TableLabel *pkgName = new TableLabel(ui.packageTable);
	pkgName->setTextFormat(Qt::RichText);
	PACKAGE *_p = packagelist.get_package(package_num);
	switch(_p->get_status())
	{
		case PKGSTATUS_INSTALLED:
			package_icon = "installed.png";
			break;
		case PKGSTATUS_INSTALL:
			package_icon = "install.png";
			break;
		case PKGSTATUS_REMOVE:
			package_icon = "remove.png";
			break;
		case PKGSTATUS_PURGE:
			package_icon = "purge.png";
			break;
		case PKGSTATUS_REMOVED_AVAILABLE:
			package_icon = "removed_available.png";
			break;
		case PKGSTATUS_AVAILABLE:
			package_icon = "available.png";
			break;
		default:
			package_icon = "unknown.png";
	}
	string pName = "<table><tbody><tr><td><img src = \"icons/"+package_icon+"\"></img></td><td><b>"+_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
	pkgName->setText(pName.c_str());
	pkgName->row = i;
	ui.packageTable->setCellWidget(i, PT_NAME, pkgName);
	stat->row = package_num;
	QObject::connect(stat, SIGNAL(stateChanged(int)), stat, SLOT(markChanges()));
	//ui.packageTable->setItem(i,PT_STATUS, new QTableWidgetItem(packagelist.get_package(package_num)->get_vstatus().c_str()));
	ui.packageTable->setItem(i,PT_ID, new QTableWidgetItem(IntToStr(package_num).c_str()));
	ui.packageTable->setRowHeight(i-1, 45);
*/
}

void MainWindow::searchPackagesByTag(QString tag)
{
	ui.packageTable->clearContents();
	ui.packageTable->setRowCount(0);

	for (unsigned int i=0; i<packagelist->size(); i++)
	{
		for (unsigned int t=0; t<packagelist->get_package(i)->get_tags()->size(); t++)
		{
			if (packagelist->get_package(i)->get_tags()->get_tag(t)->get_name() == tag.toStdString())
			{
				insertPackageIntoTable(i);
			}
		}
		setBarValue(ui.progressBar, i);
	}
}

void MainWindow::loadData()
{
	emit loadPackageDatabase();

}

void MainWindow::initProgressBar(unsigned int stepCount)
{
	ui.progressBar->setValue(0);
	ui.progressBar->setMaximum(stepCount);
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

