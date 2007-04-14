/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.45 2007/04/14 19:15:08 i27249 Exp $
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
	ui.statusbar->showMessage("Loading database...");
}

void MainWindow::sqlQueryEnd()
{
	ui.statusbar->showMessage("Data loaded, rendering visuals");
}

void MainWindow::loadingStarted()
{
	ui.statusbar->showMessage("Loading started");
}

void MainWindow::loadingFinished()
{
	ui.statusbar->showMessage("Loading finished");
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
	QString nameMask;
	bool nameOk = false;
	bool statusOk = false;
	bool categoryOk = false;
	TAG_LIST tmpTagList;
	string tagvalue;
	int packageStatus;

	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		nameOk = false;
		statusOk = false;
		categoryOk = false;
		nameMask = nameMask.fromStdString(packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_name());
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
			switch(newStatus[i])
			{
				case PKGSTATUS_INSTALLED:
					if (ui.actionShow_installed->isChecked())
					{
						statusOk = true;
					}
					else statusOk = false;
					break;
				case PKGSTATUS_INSTALL:
					if (ui.actionShow_install->isChecked()) statusOk = true;
					else statusOk = false;
					break;
				case PKGSTATUS_AVAILABLE:
					if (ui.actionShow_available->isChecked()) statusOk = true;
					else statusOk = false;

					break;
				case PKGSTATUS_PURGE:
					if (ui.actionShow_purge->isChecked()) statusOk = true;
					else statusOk = false;

					break;

				case PKGSTATUS_UNAVAILABLE:
					if (ui.actionShow_unavailable->isChecked()) statusOk = true;
					else statusOk = false;

					break;
				case PKGSTATUS_REMOVED_AVAILABLE:
					if (ui.actionShow_available->isChecked()) statusOk = true;
					else statusOk = false;
					break;
				case PKGSTATUS_REMOVED_UNAVAILABLE:
					if (ui.actionShow_unavailable->isChecked()) statusOk = true;
					else statusOk = false;	
					break;
				default:
					statusOk = false;
			} // switch(status)

		} // if(nameOk)
		if (statusOk)
		{
			tagvalue = (string) _categories.getChildNode("group", currentCategoryID).getAttribute("tag");
			if (tagvalue == "_all_")
			{
				categoryOk = true;
			}
			else
			{
				categoryOk = false;
				tmpTagList = *packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_tags();
				for (int t = 0; t < tmpTagList.size(); t++)
				{
					if (tmpTagList.get_tag(t)->get_name() == tagvalue)
					{
						categoryOk = true;
					}
				} // for (... tmpTagList ...)
				if (tmpTagList.size() == 0)
				{
					categoryOk = false;
				}
			} // else (tagvalue)
		} // if (statusOk)

		if (nameOk && statusOk && categoryOk)
		{
			ui.packageTable->setRowHidden(i, false);
		}
		else ui.packageTable->setRowHidden(i, true);
	} // for (...)	
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
	consoleMode=false; // Setting event tracking to GUI mode
	currentCategoryID=1;
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
		tmp = tmp.fromStdString(packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_name());
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
	long id = ui.packageTable->item(ui.packageTable->currentRow(), PT_ID)->text().toLong();
	PACKAGE *pkg = packagelist->get_package(id);
	string info = "<html><h1>"+pkg->get_name()+" "+pkg->get_version()+"</h1><p><b>Architecture:</b> "+pkg->get_arch()+"<br><b>Build:</b> "+pkg->get_build();
	info += "<br><b>Description: </b><br>"+pkg->get_description()+"</p></html>";

	string extendedInfo = (string) "<html>" \
			       + (string) "<h2>" + pkg->get_name() + (string) "</h2>" \
			       + (string) "<br><b>Version: </b>" + pkg->get_version() \
			       + (string) "<br><b>Arch: </b>"+pkg->get_arch() \
			       + (string) "<br><b>Build: </b>"+pkg->get_build() \
			       + (string) "<br><b>Package size: </b>" + pkg->get_compressed_size() \
			       + (string) "<br><b>Installed size: </b>" + pkg->get_installed_size() \
			       + (string) "<br><b>Filename: </b>" + pkg->get_filename() \
			       + (string) "<br><b>MD5 sum: </b>"+pkg->get_md5() \
			       + (string) "<br><b>Packager: </b>"+pkg->get_packager() + (string) " &lt" + pkg->get_packager_email() + (string)"&lt" \
			       + (string) "<br><b>Status: </b>" + pkg->get_vstatus() \
			       + (string) "</html>";

	ui.detailedEdit->setHtml(extendedInfo.c_str());
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
    ui.packageTable->setColumnWidth(PT_NAME, ui.packageTable->frameSize().width()-80);

    QTableWidgetItem *__colItem7 = new QTableWidgetItem();
    __colItem7->setText(QApplication::translate("MainWindow", "ID", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_ID, __colItem7);
    ui.packageTable->setColumnHidden(PT_ID, true);

}


void MainWindow::showPreferences()
{
	PreferencesBox *prefbox = new PreferencesBox(mDb);
	prefbox->loadData();
	prefbox->openInterface();
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
	setStatus("exiting...");
	thread->callQuit();
	qApp->quit();

}
void MainWindow::showCoreSettings()
{
}
void MainWindow::commitChanges()
{
	emit commit(newStatus);
}
//void MainWindow::resetChanges(){}
//void MainWindow::saveQueue(){}
void MainWindow::showAddRemoveRepositories(){
	PreferencesBox *prefBox = new PreferencesBox(mDb);
	prefBox->openRepositories();
}
//void MainWindow::showCustomFilter(){}
void MainWindow::setInstalledFilter()
{
	int tmp;
	bool showIt;
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		showIt = false;
		tmp = packagelist->get_package(ui.packageTable->item(i, PT_ID)->text().toLong())->get_status();
		if (ui.actionShow_installed->isChecked() && tmp == PKGSTATUS_INSTALLED)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}
		if (ui.actionShow_available->isChecked() && tmp == PKGSTATUS_AVAILABLE)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}
		if (ui.actionShow_available->isChecked() && ui.actionShow_removed->isChecked() && tmp == PKGSTATUS_REMOVED_AVAILABLE)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}
		if (ui.actionShow_removed->isChecked() && tmp == PKGSTATUS_REMOVED_UNAVAILABLE)
		{
			ui.packageTable->setRowHidden(i, false);
			showIt = true;
		}
		if (ui.actionShow_unavailable->isChecked() && tmp == PKGSTATUS_UNAVAILABLE)
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
//void MainWindow::setAvailableFilter(bool showThis){}
//void MainWindow::setBrokenFilter(bool showThis){}
//void MainWindow::setUnavailableFilter(bool showThis){}
//void MainWindow::setRemovedFilter(bool showThis){}
//void MainWindow::showHelpTopics(){}
//void MainWindow::showFaq(){}

void MainWindow::markChanges(int x, Qt::CheckState state)
{
		unsigned long i = ui.packageTable->item(x, PT_ID)->text().toLong();
		if (i >= newStatus.size())
		{
			printf("i is out of range: i=%ld, max = %d\n", i, packagelist->size());
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
					_z->row = x;
					ui.packageTable->setCellWidget(x, PT_NAME, _z);

				}
				else
				{
					newStatus[i]=PKGSTATUS_AVAILABLE;
					ui.statusbar->showMessage("Package removed from install queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/available.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					_z->row = x;

					ui.packageTable->setCellWidget(x, PT_NAME, _z);


				}
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
					_z->row = x;
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
					_z->row = x;
					ui.packageTable->setCellWidget(x, PT_NAME, _z);

				}
				break;
			case PKGSTATUS_INSTALL:
				if (state == Qt::Checked)
				{
					newStatus[i]=PKGSTATUS_INSTALL;
					ui.statusbar->showMessage("Package added to install queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/install.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					_z->row = x;
					ui.packageTable->setCellWidget(x, PT_NAME, _z);

				}
				else
				{
					newStatus[i]=PKGSTATUS_AVAILABLE;
					ui.statusbar->showMessage("Package removed from install queue");
					string pName = "<table><tbody><tr><td><img src = \"icons/available.png\"></img></td><td><b>"+ \
							_p->get_name()+"</b> "+_p->get_version() + "<br>"+_p->get_short_description()+"</td></tr></tbody></table>";
					TableLabel *_z = new TableLabel(ui.packageTable);
					_z->setTextFormat(Qt::RichText);
					_z->setText(pName.c_str());
					_z->row = x;

					ui.packageTable->setCellWidget(x, PT_NAME, _z);
				}
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
					_z->row = x;
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
					_z->row = x;
					ui.packageTable->setCellWidget(x, PT_NAME, _z);

				}
				break;
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

