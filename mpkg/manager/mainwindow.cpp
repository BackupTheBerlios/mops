/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.61 2007/04/25 14:52:23 i27249 Exp $
 *
 * TODO: Interface improvements
 * 
 *
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
//#include "packageitem.h"
void MainWindow::errorLoadingDatabase()
{
	QMessageBox::critical(this, tr("MOPSLinux package manager"),
				tr("Database initialization error!\nCheck your settings"),
				QMessageBox::Ok, QMessageBox::Ok);

}

void MainWindow::showErrorMessage(QString headerText, QString bodyText, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
	emit sendUserReply(QMessageBox::warning(this, headerText, bodyText, buttons, defaultButton));
}
void MainWindow::sqlQueryBegin()
{
	currentStatus="Loading database...";
}

void MainWindow::sqlQueryEnd()
{
	currentStatus="Data loaded, rendering visuals";
}

void MainWindow::loadingStarted()
{
	ui.loadingLabel->setText("<html><img src=\"splash.png\"></img></html>");
	ui.splashFrame->show();
	ui.packageTable->hide();
	ui.selectAllButton->hide();
	ui.deselectAllButton->hide();
	ui.quickPackageSearchEdit->hide();
	ui.clearSearchButton->hide();

}

void MainWindow::filterCloneItems()
{

	}

		
		

void MainWindow::loadingFinished()
{
	hideEntireTable();
	ui.splashFrame->hide();
	ui.quickPackageSearchEdit->show();
	ui.clearSearchButton->show();
	ui.packageTable->show();
	setTableSize();
	currentStatus = "Idle";
}

void MainWindow::enableProgressBar()
{
	ui.progressBar->show();
	ui.currentLabel->show();
}

void MainWindow::disableProgressBar()
{
	ui.progressBar->hide();
	ui.currentLabel->hide();
}

void MainWindow::setProgressBarValue(unsigned int value)
{
	ui.progressBar->setValue(value);
}

void MainWindow::enableProgressBar2()
{
	ui.progressBar2->show();
	ui.totalLabel->show();
}

void MainWindow::disableProgressBar2()
{
	ui.progressBar2->hide();
	ui.totalLabel->hide();
}

void MainWindow::setProgressBarValue2(unsigned int value)
{
	ui.progressBar2->setValue(value);
}

void MainWindow::applyPackageFilter ()
{
	string pkgBoxLabel = "Packages";
	QString nameMask;
	bool nameOk = false;
	bool statusOk = false;
	bool categoryOk = false;
	bool cloneOk = true;
	TAG_LIST tmpTagList;
	string tagvalue;
	int action;
	bool available;
	bool installed;
	bool configexist;

	pkgBoxLabel += " - " + ui.listWidget->item(currentCategoryID)->text().toStdString();
	pkgBoxLabel += " ";
	unsigned int pkgCount = 0;
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		nameOk = false;
		statusOk = false;
		categoryOk = false;
		cloneOk = true;

		nameMask = nameMask.fromStdString(packagelist->get_package(i)->get_name());
		if (nameMask.contains(ui.quickPackageSearchEdit->text(), Qt::CaseInsensitive))
		{
			nameOk = true;
		}
		else
		{
			nameOk = false;
		}

		if (nameOk)
		{
			action = packagelist->get_package(i)->action();
			available = packagelist->get_package(i)->available();
			installed = packagelist->get_package(i)->installed();
			configexist = packagelist->get_package(i)->configexist();

			statusOk=false;

			if (ui.actionShow_installed->isChecked() && installed)
			{
				ui.packageTable->setRowHidden(i, false);
				statusOk = true;
			}
			if (ui.actionShow_available->isChecked() && available && !installed)
			{
				ui.packageTable->setRowHidden(i, false);
				statusOk = true;
			}
			if (ui.actionShow_configexist->isChecked() && !installed && configexist)
			{
				ui.packageTable->setRowHidden(i, false);
				statusOk = true;
			}
			if (ui.actionShow_queue->isChecked() && action!=ST_NONE)
			{
				ui.packageTable->setRowHidden(i, false);
				statusOk = true;
			}
			if (ui.actionShow_unavailable->isChecked())
			{
				ui.packageTable->setRowHidden(i,false);
				statusOk=true;
			}
		} // if(nameOk)
		if (statusOk)
		{

			tagvalue = (string) _categories.getChildNode("group", currentCategoryID).getAttribute("tag");
			if (tagvalue == "_updates_")
			{
				if (packagelist->get_package(i)->isMaxVersion) categoryOk = true;
				else categoryOk = false;
			}
			else
			{
				if (tagvalue == "_all_")
				{
					categoryOk = true;
				}
				else
					{
					categoryOk = false;
					//tmpTagList = *packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_tags();
					tmpTagList = *packagelist->get_package(i)->get_tags();
					for (int t = 0; t < tmpTagList.size(); t++)
					{
						if (tmpTagList.get_tag(t)->get_name() == tagvalue)
						{
							categoryOk = true;
						}
					} // for (... tmpTagList ...)
					if (tmpTagList.size() == 0 && tagvalue != "_misc_")
					{
						categoryOk = false;
					}
					else
					{
						if (tmpTagList.size() == 0 && tagvalue == "_misc_") categoryOk = true;
					}
				} // else (tagvalue)
			} // if (statusOk)
		}
		if (nameOk && statusOk && categoryOk && cloneOk)
		{
			pkgCount++;
			ui.packageTable->setRowHidden(i, false);
		}
		else ui.packageTable->setRowHidden(i, true);

	} // for (...)	
	pkgBoxLabel += "\t\t("+IntToStr(pkgCount)+"/"+IntToStr(ui.packageTable->rowCount())+" packages)";
	ui.packagesBox->setTitle(pkgBoxLabel.c_str());
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

void MainWindow::selectAll()
{
	for (int i = 0; i<ui.packageTable->rowCount(); i++)
	{
		markChanges(i, Qt::Checked);
	}
}

void MainWindow::deselectAll()
{
	for (int i = 0; i<ui.packageTable->rowCount(); i++)
	{
		markChanges(i, Qt::Unchecked);
	}
}

string bool2str(bool data)
{
	if (data) return "true";
	else return "false";
}

void MainWindow::setTableItem(unsigned int row, bool checkState, string cellItemText)
{

	CheckBox *stat = new CheckBox(this);
	if (checkState) stat->setCheckState(Qt::Checked);
	else stat->setCheckState(Qt::Unchecked);
	ui.packageTable->setCellWidget(row,PT_INSTALLCHECK, stat);
	TableLabel *pkgName = new TableLabel(ui.packageTable);
	pkgName->setTextFormat(Qt::RichText);
	pkgName->setText(cellItemText.c_str());
	pkgName->row = row;
	ui.packageTable->setCellWidget(row, PT_NAME, pkgName);
	string truefalse;
	if (packagelist->get_package(row)->isMaxVersion) truefalse="true";
	else truefalse="false";
	cellItemText+="<html><b>Installed version: </b>" + packagelist->get_package(row)->installedVersion + \
		       "<br><b>isMaxVersion: </b>" + bool2str(packagelist->get_package(row)->isMaxVersion)+\
		       "<br><b>isMasterClone: </b>" + bool2str(packagelist->get_package(row)->isMasterClone) + \
		       "<br><b>hasUpdates: </b>" + bool2str(packagelist->get_package(row)->hasUpdates) +\
		       "<br><b>masterCloneID: </b>" + IntToStr(packagelist->get_package(row)->masterCloneID) + \
		       "<br><b>hasClone: </b>" + bool2str(packagelist->get_package(row)->hasClone) + \
			"<br><br><b>Description: </b><br>" + \
		       adjustStringWide(packagelist->get_package(row)->get_description(), packagelist->get_package(row)->get_short_description().size())+ \
		       		       "</html>";
	ui.packageTable->cellWidget(row, PT_NAME)->setToolTip(cellItemText.c_str());
	stat->row = row;
	QObject::connect(stat, SIGNAL(stateChanged(int)), stat, SLOT(markChanges()));
	ui.packageTable->setRowHeight(row-1, 45);
}


void MainWindow::setTableItemVisible(unsigned int row, bool visible)
{
	ui.packageTable->setRowHidden(row, visible);
}


MainWindow::MainWindow(QMainWindow *parent)
{
	consoleMode=false; // Setting event tracking to GUI mode
	currentCategoryID=2;
	qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");
	qRegisterMetaType<QMessageBox::StandardButtons>("QMessageBox::StandardButtons");

	ErrorBus = new errorBus;
	QObject::connect(this, SIGNAL(startErrorBus()), ErrorBus, SLOT(Start()), Qt::DirectConnection);
	QObject::connect(this, SIGNAL(sendUserReply(QMessageBox::StandardButton)), ErrorBus, SLOT(receiveUserReply(QMessageBox::StandardButton)), Qt::QueuedConnection);
	QObject::connect(ErrorBus, SIGNAL(sendErrorMessage(QString, QString, QMessageBox::StandardButtons, QMessageBox::StandardButton)), \
				this, SLOT(showErrorMessage(QString, QString, QMessageBox::StandardButtons, QMessageBox::StandardButton)), Qt::QueuedConnection);


	emit startErrorBus();
	if (getuid()!=0)
	{
		QMessageBox::critical(this, tr("MOPSLinux package manager"),
                   tr("You need to be root to run package manager"),
                   QMessageBox::Ok,
                   QMessageBox::Ok);
		exit(0);

	}

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	if (parent == 0) ui.setupUi(this);
	else ui.setupUi(parent);
	initCategories();
	ui.selectAllButton->hide();
	ui.deselectAllButton->hide();
	ui.quickPackageSearchEdit->hide();
	ui.clearSearchButton->hide();
	ui.packageTable->hide();
	movie = new QMovie("icons/indicator.mng");
	ui.indicatorLabel->setMovie(movie);
	movie->start();

	this->show();
	clearForm();
	disableProgressBar(); disableProgressBar2();
	tableMenu = new QMenu;
	installPackageAction = tableMenu->addAction(tr("Install"));
	removePackageAction = tableMenu->addAction(tr("Remove"));
	purgePackageAction = tableMenu->addAction(tr("Purge"));
	upgradePackageAction = tableMenu->addAction(tr("Upgrade"));
	qRegisterMetaType<string>("string");
	qRegisterMetaType<PACKAGE_LIST>("PACKAGE_LIST");
	qRegisterMetaType< vector<int> >("vector<int>");
	QObject::connect(installPackageAction, SIGNAL(triggered()), this, SLOT(markToInstall()));
	QObject::connect(ui.packageTable, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(execMenu()));
	StatusThread = new statusThread;
	thread = new coreThread; // Creating core thread
	packagelist = new PACKAGE_LIST;
	// Building thread connections
	QObject::connect(ui.clearSearchButton, SIGNAL(clicked()), ui.quickPackageSearchEdit, SLOT(clear()));
	QObject::connect(thread, SIGNAL(applyFilters()), this, SLOT(applyPackageFilter()), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(setStatus(QString)), this, SLOT(setStatus(QString)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(errorLoadingDatabase()), this, SLOT(errorLoadingDatabase()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(sqlQueryBegin()), this, SLOT(sqlQueryBegin()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(sqlQueryEnd()), this, SLOT(sqlQueryEnd()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(loadingStarted()), this, SLOT(loadingStarted()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(enableProgressBar()), this, SLOT(enableProgressBar()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(disableProgressBar()), this, SLOT(disableProgressBar()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setProgressBarValue(unsigned int)), this, SLOT(setProgressBarValue(unsigned int)), Qt::QueuedConnection);
	//QObject::connect(thread, SIGNAL(fitTable()), this, SLOT(fitTable()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(clearTable()), this, SLOT(clearTable()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setTableSize(unsigned int)), this, SLOT(setTableSize(unsigned int)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setTableItem(unsigned int, bool, string)), this, SLOT(setTableItem(unsigned int, bool, string)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setTableItemVisible(unsigned int, bool)), this, SLOT(setTableItemVisible(unsigned int, bool)), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(loadPackageDatabase()), thread, SLOT(loadPackageDatabase()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(startThread()), thread, SLOT(start()), Qt::DirectConnection);
	QObject::connect(this, SIGNAL(startStatusThread()), StatusThread, SLOT(start()), Qt::DirectConnection);

	QObject::connect(this, SIGNAL(syncData()), thread, SLOT(getPackageList()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(initProgressBar(unsigned int)), this, SLOT(initProgressBar(unsigned int)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(sendPackageList(PACKAGE_LIST, vector<int>)), this, SLOT(receivePackageList(PACKAGE_LIST, vector<int>)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(loadData()), this, SLOT(loadData()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(updateDatabase()), thread, SLOT(updatePackageDatabase()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(quitThread()), thread, SLOT(callQuit()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(commit(vector<int>)), thread, SLOT(commitQueue(vector<int>)), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(setStatus(QString)), this, SLOT(setStatus(QString)), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(enableProgressBar()), this, SLOT(enableProgressBar()), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(disableProgressBar()), this, SLOT(disableProgressBar()), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(setBarValue(unsigned int)), this, SLOT(setProgressBarValue(unsigned int)), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(initProgressBar(unsigned int)), this, SLOT(initProgressBar(unsigned int)), Qt::QueuedConnection);

	QObject::connect(StatusThread, SIGNAL(enableProgressBar2()), this, SLOT(enableProgressBar2()), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(disableProgressBar2()), this, SLOT(disableProgressBar2()), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(setBarValue2(unsigned int)), this, SLOT(setProgressBarValue2(unsigned int)), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(initProgressBar2(unsigned int)), this, SLOT(initProgressBar2(unsigned int)), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(callCleanCache()), thread, SLOT(cleanCache()), Qt::QueuedConnection);
	// Startup initialization
	emit startThread(); // Starting thread (does nothing imho....)
	emit startStatusThread();
	prefBox = new PreferencesBox(mDb);
		//ui.statusbar->addWidget(indicator);

	QObject::connect(prefBox, SIGNAL(getCdromName()), thread, SLOT(getCdromName()), Qt::QueuedConnection);
	QObject::connect(thread, SIGNAL(sendCdromName(string)), prefBox, SLOT(recvCdromVolname(string)),Qt::QueuedConnection); 

	emit loadPackageDatabase(); // Calling loadPackageDatabase from thread
}

void MainWindow::cleanCache()
{
	emit callCleanCache();
}	

void MainWindow::initCategories()
{
	if (!FileExists("/etc/mpkg-groups.xml")) return;

	//printf("File exists\n");
	XMLResults xmlErrCode;
	_categories = XMLNode::parseFile("/etc/mpkg-groups.xml", "groups", &xmlErrCode);
	if (xmlErrCode.error != eXMLErrorNone)
	{
		// Init defaults here...
	//	printf("Should initialize defaults for groups, doing nothing for now\n");
		return;
	}
	ui.listWidget->clear();
	for (unsigned int i = 0; i< _categories.nChildNode("group"); i++)
	{
		QListWidgetItem *__item = new QListWidgetItem(ui.listWidget);
    		__item->setText(QApplication::translate("MainWindow", _categories.getChildNode("group",i).getAttribute("name"), 0, QApplication::UnicodeUTF8));
    		__item->setIcon(QIcon(QString::fromUtf8(_categories.getChildNode("group", i).getAttribute("icon"))));
	}

	QObject::connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(filterCategory(int)));
	ui.listWidget->setCurrentRow(2);
}

void MainWindow::hideEntireTable()
{
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		ui.packageTable->setRowHidden(i, true);
	}
}

void MainWindow::filterCategory(int category_id)
{
	currentCategoryID = category_id;
	applyPackageFilter();
	/*TAG_LIST tmp;
	bool showIt = false;
	string tagvalue = (string) _categories.getChildNode("group", category_id).getAttribute("tag");
	printf("tagvalue = %s\n", tagvalue.c_str());
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		if (tagvalue == "_all_") showIt = true;
		else
		{
			showIt = false;
			tmp = *packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_tags();
		
			printf("tag size = %d\n", packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_tags()->size());
			for (int t = 0; t < tmp.size(); t++)
			{
				printf("scan tag %d %s\n",t,tmp.get_tag(t)->get_name().c_str());
				if (tmp.get_tag(t)->get_name() == tagvalue)
				{
					printf("all?\n");
					showIt = true;
					break;
				}
			}
		}
		if (!showIt) ui.packageTable->setRowHidden(i, true);
		else ui.packageTable->setRowHidden(i, false);
	}
	// Apply filters
	//setInstalledFilter();
	*/
}

void MainWindow::setStatus(QString status)
{
	ui.statusbar->showMessage(status);
}


MainWindow::~MainWindow()
{
	//printf("Closing threads...\n");
	thread->callQuit();
}

void MainWindow::receivePackageList(PACKAGE_LIST pkgList, vector<int> nStatus)
{
	*packagelist=pkgList;
	newStatus = nStatus;
}

void MainWindow::quickPackageSearch()
{
	QString tmp;
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		tmp = tmp.fromStdString(packagelist->get_package(i)->get_name());
		//tmp = tmp.fromStdString(packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_name());
		if (!tmp.contains(ui.quickPackageSearchEdit->text(), Qt::CaseInsensitive))
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
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		ui.packageTable->setRowHidden(i,false);
	}
}
void MainWindow::resetQueue()
{
	loadData();
}

void MainWindow::showPackageInfo()
{
	//long id = ui.packageTable->item(ui.packageTable->currentRow(), PT_ID)->text().toLong();
	long id = ui.packageTable->currentRow();
	PACKAGE *pkg = packagelist->get_package(id);
	string info = "<html><h1>"+pkg->get_name()+" "+pkg->get_version()+"</h1><p><b>Architecture:</b> "+pkg->get_arch()+"<br><b>Build:</b> "+pkg->get_build();
	info += "<br><b>Description: </b><br>"+pkg->get_description()+"</p></html>";
	mstring taglist;
	for (int i=0; i< pkg->get_tags()->size(); i++)
	{
		taglist+="<br>";
		taglist+=pkg->get_tags()->get_tag(i)->get_name();
	}
	string extendedInfo = (string) "<html>" \
			       + (string) "<h2>" + pkg->get_name() + (string) "</h2>" \
			       + (string) "<br><b>Version: </b>" + pkg->get_version() \
			       + (string) "<br><b>Arch: </b>"+pkg->get_arch() \
			       + (string) "<br><b>Build: </b>"+pkg->get_build() \
			       + (string) "<br><b>Package size: </b>" + humanizeSize(pkg->get_compressed_size()) \
			       + (string) "<br><b>Installed size: </b>" + humanizeSize(pkg->get_installed_size()) \
			       + (string) "<br><b>Filename: </b>" + pkg->get_filename() \
			       + (string) "<br><b>MD5 sum: </b>"+pkg->get_md5() \
			       + (string) "<br><b>Maintainer: </b>"+pkg->get_packager() \
			       + (string) " (" + pkg->get_packager_email() + (string)")" \
			       + (string) "<br><b>Status: </b>" + pkg->get_vstatus() \
			       + (string) "<br><br><b>Tags: </b> " \
			       + taglist.s_str() \
			       + (string) "</html>";

	ui.detailedEdit->setHtml(extendedInfo.c_str());
	ui.overviewEdit->setHtml(info.c_str());
}

void MainWindow::execMenu()
{
	
	tableMenu->exec(QCursor::pos());
}
void MainWindow::setTableSize()
{
	ui.packageTable->setColumnWidth(PT_INSTALLCHECK, 15);
 	ui.packageTable->setColumnWidth(PT_NAME, ui.packageTable->frameSize().width()-80);
}
void MainWindow::initPackageTable()
{

    if (ui.packageTable->columnCount() < 2)
    	ui.packageTable->setColumnCount(2);
    QTableWidgetItem *__colItem0 = new QTableWidgetItem();
    __colItem0->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_INSTALLCHECK, __colItem0);
   
    //QTableWidgetItem *__colItem = new QTableWidgetItem();
    //__colItem->setText(QApplication::translate("MainWindow", "Status", 0, QApplication::UnicodeUTF8));
    //ui.packageTable->setHorizontalHeaderItem(PT_STATUS, __colItem);
    //ui.packageTable->setColumnHidden(PT_STATUS, true);

    QTableWidgetItem *__colItem1 = new QTableWidgetItem();
    __colItem1->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_NAME, __colItem1);

    //QTableWidgetItem *__colItem7 = new QTableWidgetItem();
    //__colItem7->setText(QApplication::translate("MainWindow", "ID", 0, QApplication::UnicodeUTF8));
    //ui.packageTable->setHorizontalHeaderItem(PT_ID, __colItem7);
    //ui.packageTable->setColumnHidden(PT_ID, true);
	setTableSize();
}


void MainWindow::showPreferences()
{
	prefBox->loadData();
	prefBox->openInterface();
}

void MainWindow::showAbout()
{
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
	currentStatus="exiting...";
	thread->callQuit();
	qApp->quit();

}
void MainWindow::showCoreSettings()
{
	prefBox->openCore();
}
void MainWindow::commitChanges()
{
	emit commit(newStatus);
}
//void MainWindow::resetChanges(){}
//void MainWindow::saveQueue(){}
void MainWindow::showAddRemoveRepositories(){
	prefBox->openRepositories();
}
//void MainWindow::showCustomFilter(){}
void MainWindow::setInstalledFilter()
{
	int action=0;
	bool available=0;
	bool installed=0;
	bool configexist=0;
	bool showIt;
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		showIt = false;
		//action = packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->action();
		//available = packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->available();
		//installed = packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->installed();
		//configexist = packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->configexist();
		action = packagelist->get_package(i)->action();
		available = packagelist->get_package(i)->available();
		installed = packagelist->get_package(i)->installed();
		configexist = packagelist->get_package(i)->configexist();


		if (ui.actionShow_installed->isChecked() && installed)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}
		if (ui.actionShow_available->isChecked() && available)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}
		if (ui.actionShow_configexist->isChecked() && !installed && configexist)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}
		if (ui.actionShow_queue->isChecked() && action!=ST_NONE)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}

		if (!showIt)
		{
			ui.packageTable->setRowHidden(i, true);
		}
	}
}

void MainWindow::markChanges(int x, Qt::CheckState state)
{
		//unsigned long i = ui.packageTable->item(x, PT_ID)->text().toLong();
		unsigned long i = x;
		if (i >= newStatus.size())
		{
			printf("i is out of range: i=%ld, max = %d\n", i, packagelist->size());
			return;
		}
		PACKAGE *_p = packagelist->get_package(i);
		if (_p->installed())
		{
			switch(_p->action())
			{
			case ST_NONE:
				if (state == Qt::Checked)
				{
					newStatus[i]=ST_NONE;
					currentStatus="Package keeped in system";
				}
				else
				{
					newStatus[i]=ST_REMOVE;
					currentStatus="Package queued to remove";
				}
				break;
			case ST_REMOVE:
				if (state == Qt::Checked)
				{
					newStatus[i]=ST_NONE;
					currentStatus="Package removed from remove queue";
				}
				else
				{
					newStatus[i]=ST_REMOVE;
					currentStatus="Package queued to remove";
				}
				break;
			case ST_PURGE:
				if (state == Qt::Checked)
				{
					newStatus[i]=ST_NONE;
					currentStatus="Package removed from purge queue";
				}
				else
				{
					newStatus[i]=ST_PURGE;
					currentStatus="Package queued to purge";
				}
				break;
			default:
				currentStatus="Unknown condition";
			}
		} // if(_p->installed())
		else
		{
			switch(_p->action())
			{
				case ST_INSTALL:
				case ST_NONE:
					if (state==Qt::Checked)
					{
						newStatus[i]=ST_INSTALL;
						currentStatus="Package queued to install";
					}
					else
					{
						newStatus[i]=ST_NONE;
						currentStatus="Package unqueued";
					}
					break;
				case ST_PURGE:
					if (state==Qt::Checked)
					{
						newStatus[i]=ST_INSTALL;
						currentStatus="Package queued to install";
					}
					else
					{
						newStatus[i]=ST_PURGE;
						currentStatus="Package queued to purge";
					}
					break;
				default:
					currentStatus="Unknown condition";
			}
		} // else
		string package_icon;
		switch (newStatus[i])
		{
		case ST_NONE:
			if (_p->installed()) package_icon = "installed.png";
			else
			{
				if (_p->available()){
				       package_icon="available.png";
				       if (_p->configexist()) package_icon="removed_available.png";
				}
				else package_icon="unknown.png";
			}
			break;
		case ST_INSTALL:
			package_icon="install.png";
			break;
		case ST_REMOVE:
			package_icon="remove.png";
			break;
		case ST_PURGE:
			package_icon="purge.png";
			break;
		}
		
		string pName = "<table><tbody><tr><td><img src = \"icons/"+package_icon+"\"></img></td><td><b>"+ \
			_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+\
			"</td></tr></tbody></table>";
		setTableItem(x, state, pName);
		//TableLabel *_z = new TableLabel(ui.packageTable);
		//_z->setTextFormat(Qt::RichText);
		//_z->setText(pName.c_str());
		//_z->row = x;
		//ui.packageTable->setCellWidget(x, PT_NAME, _z);
		
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

void MainWindow::initProgressBar2(unsigned int stepCount)
{
	ui.progressBar2->setValue(0);
	ui.progressBar2->setMaximum(stepCount);
}



void MainWindow::setBarValue(QProgressBar *Bar, int stepValue)
{
	Bar->setValue(stepValue);
}

void MainWindow::markToInstall()
{

}

void CheckBox::markChanges()
{
	mw->markChanges(row, checkState());
}

CheckBox::CheckBox(MainWindow *parent)
{
	mw = parent;
}

