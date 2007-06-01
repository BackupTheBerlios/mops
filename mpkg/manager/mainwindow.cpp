/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.105 2007/06/01 02:51:46 i27249 Exp $
 *
 ****************************************************************/

#include <QTextCodec>
#include <QtGui>
#include "mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include "aboutbox.h"
#include "preferencesbox.h"
#include "loading.h"
#include "db.h"
#include <stdio.h>
#include "tablelabel.h"
#include <unistd.h>

MainWindow::MainWindow(QMainWindow *parent)
{
	totalInstalledSize=0;
	totalAvailableSize=0;
	totalAvailableCount = 0;
	installedCount = 0;
	installQueueCount = 0;
	removeQueueCount = 0;
	willBeFreed = 0;
	willBeOccupied = 0;
	lockPackageList(false);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

	initializeOk=false;
	consoleMode=false; // Setting event tracking to GUI mode
	currentCategoryID=1;
	qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");
	qRegisterMetaType<QMessageBox::StandardButtons>("QMessageBox::StandardButtons");

	ErrorBus = new errorBus;
	QObject::connect(this, SIGNAL(startErrorBus()), ErrorBus, SLOT(Start()), Qt::DirectConnection);
	QObject::connect(this, SIGNAL(sendUserReply(QMessageBox::StandardButton)), ErrorBus, SLOT(receiveUserReply(QMessageBox::StandardButton)));
	QObject::connect(ErrorBus, SIGNAL(sendErrorMessage(QString, QString, QMessageBox::StandardButtons, QMessageBox::StandardButton)), \
				this, SLOT(showErrorMessage(QString, QString, QMessageBox::StandardButtons, QMessageBox::StandardButton)));


	emit startErrorBus();
	if (getuid()!=0)
	{
		QMessageBox::critical(this, tr("MOPSLinux package manager"),
                   tr("You need to be root to run package manager"),
                   QMessageBox::Ok,
                   QMessageBox::Ok);
		exit(0);

	}

	if (parent == 0) ui.setupUi(this);
	else ui.setupUi(parent);
	ui.applyButton->setEnabled(false);
	ui.quickPackageSearchEdit->hide();
	ui.quickSearchLabel->hide();
	ui.clearSearchButton->hide();
	ui.packageTable->hide();
	movie = new QMovie("/usr/share/mpkg/icons/indicator.mng");
	ui.indicatorLabel->setMovie(movie);
	movie->start();
	ui.progressTable->setColumnWidth(0,190);
	ui.progressTable->setColumnWidth(1,350);
	ui.progressTable->setColumnHidden(2,true);

	clearForm();
	disableProgressBar(); disableProgressBar2();
	
	tableMenu = new QMenu;
	installPackageAction = tableMenu->addAction(tr("Install"));
	removePackageAction = tableMenu->addAction(tr("Remove"));
	purgePackageAction = tableMenu->addAction(tr("Purge"));

	qRegisterMetaType<string>("string");
	qRegisterMetaType<PACKAGE_LIST>("PACKAGE_LIST");
	qRegisterMetaType< vector<int> >("vector<int>");
	qRegisterMetaType< vector<string> >("vector<string>");
	StatusThread = new statusThread;
	thread = new coreThread; // Creating core thread
	packagelist = new PACKAGE_LIST;
	ui.progressTable->hide();
	// Building thread connections
	QObject::connect(this, SIGNAL(imReady()), thread, SLOT(recvReadyFlag()));
	QObject::connect(this, SIGNAL(fillReady()), thread, SLOT(recvFillReady()));

	QObject::connect(StatusThread, SIGNAL(showProgressWindow(bool)), this, SLOT(showProgressWindow(bool)));
	QObject::connect(StatusThread, SIGNAL(setSkipButton(bool)), this, SLOT(setSkipButton(bool)));
	QObject::connect(StatusThread, SIGNAL(setIdleButtons(bool)), this, SLOT(setIdleButtons(bool)));
	QObject::connect(ui.abortButton, SIGNAL(clicked()), this, SLOT(abortActions()));
	QObject::connect(ui.skipButton, SIGNAL(clicked()), this, SLOT(skipAction()));
	QObject::connect(this, SIGNAL(redrawReady(bool)), StatusThread, SLOT(recvRedrawReady(bool)));
	QObject::connect(StatusThread, SIGNAL(loadProgressData()), this, SLOT(updateProgressData()));
	QObject::connect(ui.clearSearchButton, SIGNAL(clicked()), ui.quickPackageSearchEdit, SLOT(clear()));
	QObject::connect(thread, SIGNAL(applyFilters()), this, SLOT(applyPackageFilter()));
	QObject::connect(thread, SIGNAL(initState(bool)), this, SLOT(setInitOk(bool)));
	QObject::connect(StatusThread, SIGNAL(setStatus(QString)), this, SLOT(setStatus(QString)));
	QObject::connect(thread, SIGNAL(errorLoadingDatabase()), this, SLOT(errorLoadingDatabase()));
	QObject::connect(thread, SIGNAL(sqlQueryBegin()), this, SLOT(sqlQueryBegin()));
	QObject::connect(thread, SIGNAL(sqlQueryEnd()), this, SLOT(sqlQueryEnd()));
	QObject::connect(thread, SIGNAL(loadingStarted()), this, SLOT(loadingStarted()));
	QObject::connect(thread, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()));
	QObject::connect(thread, SIGNAL(enableProgressBar()), this, SLOT(enableProgressBar()));
	QObject::connect(thread, SIGNAL(disableProgressBar()), this, SLOT(disableProgressBar()));
	QObject::connect(thread, SIGNAL(resetProgressBar()), this, SLOT(resetProgressBar()));
	QObject::connect(thread, SIGNAL(setProgressBarValue(unsigned int)), this, SLOT(setProgressBarValue(unsigned int)));
	QObject::connect(thread, SIGNAL(clearTable()), this, SLOT(clearTable()));
	QObject::connect(thread, SIGNAL(setTableSize(unsigned int)), this, SLOT(setTableSize(unsigned int)));
	QObject::connect(thread, SIGNAL(setTableItem(unsigned int, int, bool, string)), this, SLOT(setTableItem(unsigned int, int, bool, string)));
	QObject::connect(thread, SIGNAL(setTableItemVisible(unsigned int, bool)), this, SLOT(setTableItemVisible(unsigned int, bool)));
	QObject::connect(this, SIGNAL(loadPackageDatabase()), thread, SLOT(loadPackageDatabase()));
	QObject::connect(this, SIGNAL(startThread()), thread, SLOT(start()), Qt::DirectConnection);
	QObject::connect(this, SIGNAL(startStatusThread()), StatusThread, SLOT(start()), Qt::DirectConnection);
	QObject::connect(this, SIGNAL(requestPackages(vector<bool>)), thread, SLOT(loadItems(vector<bool>)));
	QObject::connect(this, SIGNAL(syncData()), thread, SLOT(getPackageList()));
	QObject::connect(thread, SIGNAL(initProgressBar(unsigned int)), this, SLOT(initProgressBar(unsigned int)));
	QObject::connect(thread, SIGNAL(sendPackageList(PACKAGE_LIST, vector<int>)), this, SLOT(receivePackageList(PACKAGE_LIST, vector<int>)));
	QObject::connect(thread, SIGNAL(loadData()), this, SLOT(loadData()), Qt::DirectConnection);
	QObject::connect(this, SIGNAL(updateDatabase()), thread, SLOT(updatePackageDatabase()));
	QObject::connect(this, SIGNAL(quitThread()), thread, SLOT(callQuit()));
	QObject::connect(this, SIGNAL(commit(vector<int>)), thread, SLOT(commitQueue(vector<int>)));
	QObject::connect(thread, SIGNAL(setStatus(QString)), this, SLOT(setStatus(QString)));
	QObject::connect(StatusThread, SIGNAL(enableProgressBar()), this, SLOT(enableProgressBar()));
	QObject::connect(StatusThread, SIGNAL(disableProgressBar()), this, SLOT(disableProgressBar()));
	QObject::connect(StatusThread, SIGNAL(setBarValue(unsigned int)), this, SLOT(setProgressBarValue(unsigned int)));
	QObject::connect(StatusThread, SIGNAL(initProgressBar(unsigned int)), this, SLOT(initProgressBar(unsigned int)));
	QObject::connect(this, SIGNAL(getAvailableTags()), thread, SLOT(getAvailableTags()));
	QObject::connect(thread, SIGNAL(sendAvailableTags(vector<string>)), this, SLOT(receiveAvailableTags(vector<string>)), Qt::QueuedConnection);
	QObject::connect(StatusThread, SIGNAL(enableProgressBar2()), this, SLOT(enableProgressBar2()));
	QObject::connect(StatusThread, SIGNAL(disableProgressBar2()), this, SLOT(disableProgressBar2()));
	QObject::connect(StatusThread, SIGNAL(setBarValue2(unsigned int)), this, SLOT(setProgressBarValue2(unsigned int)));
	QObject::connect(StatusThread, SIGNAL(initProgressBar2(unsigned int)), this, SLOT(initProgressBar2(unsigned int)));
	QObject::connect(this, SIGNAL(callCleanCache()), thread, SLOT(cleanCache()));
	QObject::connect(thread, SIGNAL(showMessageBox(QString, QString)), this, SLOT(showMessageBox(QString, QString)));
	// Startup initialization
	emit startThread(); // Starting thread (does nothing imho....)
	emit startStatusThread();
	prefBox = new PreferencesBox(mDb);
	QObject::connect(prefBox, SIGNAL(updatePackageData()), thread, SLOT(updatePackageDatabase()));
	QObject::connect(prefBox, SIGNAL(getCdromName()), thread, SLOT(getCdromName()));
	QObject::connect(thread, SIGNAL(sendCdromName(string)), prefBox, SLOT(recvCdromVolname(string))); 

	QObject::connect(this, SIGNAL(getRequiredPackages(unsigned int)), thread, SLOT(getRequiredPackages(unsigned int)));
	QObject::connect(this, SIGNAL(getDependantPackages(unsigned int)), thread, SLOT(getDependantPackages(unsigned int)));
	QObject::connect(thread, SIGNAL(sendRequiredPackages(unsigned int, PACKAGE_LIST)), this, SLOT(receiveRequiredPackages(unsigned int, PACKAGE_LIST)));
	QObject::connect(thread, SIGNAL(sendDependantPackages(unsigned int, PACKAGE_LIST)), this, SLOT(receiveDependantPackages(unsigned int, PACKAGE_LIST)));

	this->show();
	// Wait threads to start
	while (!StatusThread->isRunning() && !thread->isRunning() && !ErrorBus->isRunning())
	{
		say("Waiting for threads to start...\n");
		usleep(1);
	}

	emit getAvailableTags();
	emit loadPackageDatabase(); // Calling loadPackageDatabase from thread
}

MainWindow::~MainWindow()
{
	ui.packageTable->clearContents();
	thread->callQuit();
	StatusThread->halt();
	ErrorBus->Stop();
	thread->wait();
	StatusThread->wait();
	ErrorBus->wait();
}


void MainWindow::setSkipButton(bool flag)
{
	ui.skipButton->setVisible(flag);
}

void MainWindow::setIdleButtons(bool flag)
{
	ui.applyButton->setVisible(flag);
	ui.quitButton->setVisible(flag);
	ui.abortButton->setVisible(!flag);
}

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

void MainWindow::showMessageBox(QString header, QString body)
{
	QMessageBox::information(this, header, body, QMessageBox::Ok, QMessageBox::Ok);
}
void MainWindow::sqlQueryBegin()
{
	currentStatus=tr("Loading database...").toStdString();
}

void MainWindow::sqlQueryEnd()
{
	currentStatus=tr("Data loaded, rendering visuals").toStdString();
}

void MainWindow::loadingStarted()
{

	ui.statLabel->setText("");
	ui.loadingLabel->setText("<html><img src=\"/usr/share/mpkg/splash.png\"></img></html>");
	ui.packageListBox->setEnabled(false);
	ui.packageInfoBox->hide();
	ui.packageTable->hide();
	ui.selectAllButton->hide();
	ui.deselectAllButton->hide();
	ui.quickPackageSearchEdit->hide();
	ui.quickSearchLabel->hide();
	ui.clearSearchButton->hide();
	ui.splashFrame->show();
}

void MainWindow::filterCloneItems()
{

}			

void MainWindow::loadingFinished()
{
	hideEntireTable();
	ui.splashFrame->hide();
	ui.packageListBox->setEnabled(true);
	ui.progressTable->hide();
	ui.quickSearchLabel->show();
	ui.quickPackageSearchEdit->show();
	ui.clearSearchButton->show();
	ui.packageTable->show();
	ui.selectAllButton->show();
	ui.deselectAllButton->show();
	
	setTableSize();
	emit imReady();
	currentStatus = tr("Idle").toStdString();
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
void MainWindow::resetProgressBar()
{
	ui.progressBar->reset();
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

void MainWindow::generateStat()
{
	totalInstalledSize=0;
	totalAvailableSize=0;
	totalAvailableCount = 0;
	installedCount = 0;
	installQueueCount = 0;
	removeQueueCount = 0;
	updateQueueCount = 0;
	willBeFreed = 0;
	willBeOccupied = 0;
	if (!initializeOk)
	{
		return;
	}
	waitUnlock();
	for (int i=0; i<packagelist->size(); i++)
	{
		if (packagelist->get_package(i)->installed())
		{
			installedCount++;
			totalInstalledSize = totalInstalledSize + atoi(packagelist->get_package(i)->get_installed_size()->c_str());
		}
		if (packagelist->get_package(i)->available())
		{
			totalAvailableCount++;
			totalAvailableSize=totalAvailableSize + atoi(packagelist->get_package(i)->get_compressed_size()->c_str());
		}
		if (newStatus[i]==ST_INSTALL)
		{
			installQueueCount++;
			willBeOccupied = willBeOccupied + atoi(packagelist->get_package(i)->get_installed_size()->c_str());
		}
		if (newStatus[i]==ST_REMOVE)
		{
			removeQueueCount++;
			willBeFreed = willBeFreed + atoi(packagelist->get_package(i)->get_installed_size()->c_str());
		}
		if (newStatus[i] == ST_UPDATE)
		{
			updateQueueCount++;
			willBeFreed = willBeFreed + atoi(packagelist->get_package(i)->get_installed_size()->c_str());
		}

		if (newStatus[i]==ST_PURGE)
		{
			removeQueueCount++;
			if (packagelist->get_package(i)->installed()) willBeFreed=willBeFreed + atoi(packagelist->get_package(i)->get_installed_size()->c_str());
		}
	}
	installQueueCount -= updateQueueCount;
	QString countStat = tr("Installed: ") + IntToStr(installedCount).c_str() +\
			    tr(", Available: ")+IntToStr(totalAvailableCount).c_str()+\
			    tr(", To install: ")+IntToStr(installQueueCount).c_str()+\
			    tr(", To remove: ")+IntToStr(removeQueueCount).c_str()+\
			    tr(", To update: ")+IntToStr(updateQueueCount).c_str();
	if (willBeFreed>willBeOccupied) countStat += tr(", Will be freed: ") + humanizeSize(willBeFreed - willBeOccupied).c_str();
	if (willBeFreed<willBeOccupied) countStat+= tr(", Will be occupied: ") + humanizeSize(willBeOccupied - willBeFreed).c_str();

	ui.statLabel->setText(countStat);
}

void MainWindow::applyPackageFilter ()
{
	if (!initializeOk)
	{
		printf("%s: uninitialized\n", __func__);
		return;
	}
	else printf("%s: init ok\n", __func__);
	string pkgBoxLabel = tr("Packages").toStdString();
	generateStat();
	bool nameOk = false;
	bool statusOk = false;
	bool categoryOk = false;
	bool deprecatedOk = false;
	vector<string> tmpTagList;
	string tagvalue;
	int action;
	bool available;
	bool installed;
	bool configexist;
	bool deprecated;

	pkgBoxLabel += " - " + ui.listWidget->item(currentCategoryID)->text().toStdString();
	pkgBoxLabel += " ";
	unsigned int pkgCount = 0;
	for (unsigned int a=0; a<availableTags.size(); a++)
	{
		highlightMap[availableTags[a]]=false;
	}
	waitUnlock();
	for (int i=0; i<packagelist->size(); i++)
	{
		nameOk = false;
		statusOk = false;
		categoryOk = false;

		nameOk = nameComplain(i, ui.quickPackageSearchEdit->text());
	
		if (nameOk)
		{
			action = packagelist->get_package(i)->action();
			available = packagelist->get_package(i)->available();
			installed = packagelist->get_package(i)->installed();
			configexist = packagelist->get_package(i)->configexist();
			deprecated = packagelist->get_package(i)->deprecated();
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
			if (ui.actionShow_unavailable->isChecked() && !available && !installed)
			{
				ui.packageTable->setRowHidden(i,false);
				statusOk=true;
			}
		} // if(nameOk)
		if (statusOk)
		{
			if (deprecated)
			{
				if (ui.actionShow_deprecated->isChecked())
				{
					ui.packageTable->setRowHidden(i, false);
					deprecatedOk = true;
				}
				else deprecatedOk = false;
			}
			else deprecatedOk=true;

		}
		for (unsigned int a=0; a<availableTags.size(); a++)
		{
			if (isCategoryComplain(i, a) && nameOk)
			{
				highlightMap[availableTags[a]]=true;
			}
		}
		if (nameOk && statusOk && deprecatedOk && isCategoryComplain(i, currentCategoryID))
		{
			pkgCount++;
			ui.packageTable->setRowHidden(packagelist->getTableID(i), false);
		}
		else 
		{
			if (isCategoryComplain(i, currentCategoryID)) ui.packageTable->setRowHidden(packagelist->getTableID(i), true);
		}
	} // for (...)	

	pkgBoxLabel += "\t\t("+IntToStr(pkgCount)+"/"+IntToStr(ui.packageTable->rowCount())\
			+tr(" packages)").toStdString();
	ui.packagesBox->setTitle(pkgBoxLabel.c_str());
	highlightCategoryList();
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
		if (!ui.packageTable->isRowHidden(i)) markChanges(i, Qt::Checked);
	}
	applyPackageFilter();
}

void MainWindow::deselectAll()
{
	for (int i = 0; i<ui.packageTable->rowCount(); i++)
	{
		if (!ui.packageTable->isRowHidden(i)) markChanges(i, Qt::Unchecked);
	}
	applyPackageFilter();
}

string MainWindow::bool2str(bool data)
{
	if (data) return tr("true").toStdString();
	else return tr("false").toStdString();
}

void MainWindow::setTableItem(unsigned int row, int packageNum, bool checkState, string cellItemText)
{
	waitUnlock();
	if (nameComplain(packageNum, ui.quickPackageSearchEdit->text()))
	{
		ui.packageTable->setRowHidden(row, false);
	}
	packagelist->setTableID(packageNum, row);
	CheckBox *stat = new CheckBox(this);
	if (checkState) stat->setCheckState(Qt::Checked);
	else stat->setCheckState(Qt::Unchecked);
	ui.packageTable->setCellWidget(row,PT_INSTALLCHECK, stat);
	TableLabel *pkgName = new TableLabel(ui.packageTable);
	pkgName->setTextFormat(Qt::RichText);
	pkgName->setText(cellItemText.c_str());
	pkgName->row = row;
	ui.packageTable->setCellWidget(row, PT_NAME, pkgName);
	string depData;
	if (packagelist->get_package(packageNum)->get_dependencies()->size()>0)
	{
		depData = "<b> "+tr("Depends on:").toStdString()+" </b>";
		for (unsigned int i=0; i<packagelist->get_package(packageNum)->get_dependencies()->size(); i++)
		{
			depData += "<br>"+packagelist->get_package(packageNum)->get_dependencies()->at(i).getDepInfo();
		}
		depData+="<br>";
	}

	string tagList = "<br><b>" + tr("Tags:").toStdString() + "</b>";
	for (unsigned int i=0; i<packagelist->get_package(packageNum)->get_tags()->size(); i++)
	{
		tagList+= "<br>" + packagelist->get_package(packageNum)->get_tags()->at(i);
	}
	tagList += "<br>";

	cellItemText+="<html><b>"+tr("Installed version:").toStdString()+" </b>" + packagelist->get_package(packageNum)->installedVersion + \
		       "<br><b>"+tr("It is max version:").toStdString()+" </b>" + bool2str(packagelist->get_package(packageNum)->hasMaxVersion)+\
		       "<br><b>"+tr("Max version:").toStdString()+" </b>" + packagelist->get_package(packageNum)->maxVersion + \
			"<br>" + depData + tagList + \
			"<b>"+tr("Description:").toStdString()+" </b><br>" + \
		       adjustStringWide(*packagelist->get_package(packageNum)->get_description(), packagelist->get_package(packageNum)->get_short_description()->size())+ \
		       		       "</html>";
	ui.packageTable->cellWidget(row, PT_NAME)->setToolTip(cellItemText.c_str());
	stat->row = row;
	QObject::connect(stat, SIGNAL(stateChanged(int)), stat, SLOT(markChanges()));
	QObject::connect(stat, SIGNAL(stateChanged(int)), this, SLOT(applyPackageFilter()));
	ui.packageTable->setRowHeight(row, 45);
	//ui.packageTable->setRowHidden(row, true);
	//thread->recvFillReady();
}


void MainWindow::setTableItemVisible(unsigned int row, bool visible)
{
	ui.packageTable->setRowHidden(row, visible);
}



void MainWindow::abortActions()
{
	if (QMessageBox::warning(this, tr("Please confirm abort"), tr("Are you sure you want to abort current operations?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No)==QMessageBox::Yes)
	{
		actionBus.abortActions();
	}
}

void MainWindow::skipAction()
{
	actionBus.skipAction(actionBus.currentProcessingID());
}

void MainWindow::setInitOk(bool flag)
{
	initializeOk=flag;
}
void MainWindow::showProgressWindow(bool flag)
{
	if (flag)
	{
		ui.progressTable->show();
		if (ui.packageTable->isVisible())
		{
			ui.packageTable->hide();
			ui.selectAllButton->hide();
			ui.quickSearchLabel->hide();
			ui.deselectAllButton->hide();
			ui.quickPackageSearchEdit->hide();
			ui.clearSearchButton->hide();
		}
	}
	else
	{
		if (ui.progressTable->isVisible() && !ui.splashFrame->isVisible())
		{
			ui.packageTable->show();
			ui.selectAllButton->show();
			ui.deselectAllButton->show();
			ui.quickPackageSearchEdit->show();
			ui.quickSearchLabel->show();
			ui.clearSearchButton->show();
		}
		ui.progressTable->hide();
	}
}

void MainWindow::updateProgressData()
{
	ui.packagesBox->setTitle(pData.getCurrentAction().c_str());
	emit redrawReady(false);
	double dtmp=0;
	int tmp_c;
	ui.progressTable->clearContents();
	int totalCount=pData.size();
	int totalVisible=0;
	for (unsigned int i=0; i<pData.size(); i++)
	{
		if (pData.getItemState(i)!=ITEMSTATE_WAIT ) totalVisible++;
	}
	ui.progressTable->setRowCount(totalCount);

	int tablePos = 0;
	bool showIt;
	bool updateIt;
	bool keepCount;
	if (pData.size()>0)
	{
		for (unsigned int i=0; i<pData.size(); i++)
		{
			tablePos=i;
			showIt=false;
			updateIt=false;
			keepCount=false;
			if (pData.getItemState(i)==ITEMSTATE_FINISHED && totalVisible>20)
			{
				keepCount=true;
				totalVisible--;
			}
			if (pData.getItemState(i)!=ITEMSTATE_WAIT && !keepCount) showIt=true;

			if (pData.getItemState(i)==ITEMSTATE_FINISHED) pData.increaseIdleTime(i); 
			
			if (showIt)
			{
				ui.progressTable->setRowHidden(tablePos, false);
				pData.setItemUnchanged(i);
				
				QTableWidgetItem *__name = new QTableWidgetItem;
				__name->setText(pData.getItemName(i).c_str());
				
				switch (pData.getItemState(i))
				{
					case ITEMSTATE_INPROGRESS:
						__name->setIcon(QIcon(QString::fromUtf8("/opt/kde/share/icons/crystalsvg/32x32/apps/kget.png")));
						break;
					case ITEMSTATE_FINISHED:
						__name->setIcon(QIcon(QString::fromUtf8("/usr/share/mpkg/icons/installed.png")));
						break;
					case ITEMSTATE_FAILED:
						__name->setIcon(QIcon(QString::fromUtf8("/usr/share/mpkg/icons/remove.png")));
						break;
				}
				
				ui.progressTable->setItem(tablePos,0, __name);
				if (pData.getItemState(i)==ITEMSTATE_INPROGRESS)
				{
					QProgressBar *pBar = new QProgressBar;
					pBar->setFormat(pData.getItemCurrentAction(i).c_str());
					if (pData.size()>0) dtmp = 100 * (pData.getItemProgress(i)/pData.getItemProgressMaximum(i));
					tmp_c = (int) dtmp;

					pBar->setMaximum(100);
					pBar->setValue(tmp_c);
					ui.progressTable->setCellWidget(tablePos,1,pBar);
				}
				else
				{
					ui.progressTable->setItem(tablePos,1,new QTableWidgetItem(pData.getItemCurrentAction(i).c_str()));
				}
				ui.progressTable->setRowHeight(tablePos,20);
			}
			else ui.progressTable->setRowHidden(tablePos, true);

			tablePos++;
		}
	}
	emit redrawReady(true);

}

void MainWindow::cleanCache()
{
	emit callCleanCache();
}	

void MainWindow::receiveAvailableTags(vector<string> tags)
{
	lockPackageList(true);
	availableTags.clear();
	availableTags.push_back("_all_");
	availableTags.push_back("_updates_");
	for (unsigned int i=0; i<tags.size(); i++)
	{
		availableTags.push_back(tags[i]);
	}
	availableTags.push_back("_misc_");

	lockPackageList(false);
	initCategories(true);
}

void MainWindow::initCategories(bool initial)
{
	printf("%s: start\n", __func__);
	if (!FileExists("/etc/mpkg-groups.xml")) return;
	XMLResults xmlErrCode;
	_categories = XMLNode::parseFile("/etc/mpkg-groups.xml", "groups", &xmlErrCode);
	if (xmlErrCode.error != eXMLErrorNone)
	{
		return;
	}
	bool lockSta=false;
	ui.listWidget->clear();

	bool named = false;
	for (unsigned int i=0; i<availableTags.size(); i++)
	{
		highlightMap[availableTags[i]]=false;

		named=false;
		for (int t = 0; t< _categories.nChildNode("group");t++)
		{
			if (availableTags[i]==(string) _categories.getChildNode("group",t).getAttribute("tag"))
			{
				named=true;
				QListWidgetItem *__item = new QListWidgetItem(ui.listWidget);
				ListLabel *L_item = new ListLabel(ui.listWidget, i);
				L_item->setText((QString) _categories.getChildNode("group",t).getAttribute("name"));
		    		__item->setIcon(QIcon(QString::fromUtf8(_categories.getChildNode("group", t).getAttribute("icon"))));
				ui.listWidget->setItemWidget(__item, L_item);
			}
		}
		if (!named)
		{
			ListLabel *L_item = new ListLabel(ui.listWidget, i);
			QListWidgetItem *__item = new QListWidgetItem(ui.listWidget);
			L_item->setText(availableTags[i].c_str());
	    		__item->setIcon(QIcon("/usr/share/mpkg/icons/icons/taskbar.png"));
			ui.listWidget->setItemWidget(__item, L_item);


		}
	}

	QObject::connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(filterCategory(int)));
	if (initial)
	{
		ui.listWidget->setCurrentRow(1);
		ui.listWidget->scrollToItem(ui.listWidget->item(0));
	}
}

void MainWindow::hideEntireTable()
{
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		ui.packageTable->setRowHidden(i, true);
	}
}
bool MainWindow::isCategoryComplain(int package_num, int category_id)
{
	waitUnlock();

	string tagvalue;
	bool ret=false;
	if (category_id>=0 && availableTags.size()>(unsigned int) category_id)
	{
		tagvalue = availableTags[category_id];
	}
	else tagvalue = "";
	if (tagvalue == "_updates_")
	{
		if (packagelist->get_package(package_num)->isUpdate()) ret = true;
		else ret = false;
	}
	else
	{
		if (tagvalue == "_all_")
		{
			ret = true;
		}
		else
		{
			ret = false;
			for (unsigned int t = 0; t < packagelist->get_package(package_num)->get_tags()->size(); t++)
			{
				if (packagelist->get_package(package_num)->get_tags()->at(t) == tagvalue)
				{
					ret = true;
				}
			} // for (... tmpTagList ...)	
			if (packagelist->get_package(package_num)->get_tags()->empty() && tagvalue != "_misc_")
			{
				ret = false;
			}
			else
			{
				if (packagelist->get_package(package_num)->get_tags()->empty() && tagvalue == "_misc_") ret = true;
			}
		}
	}
	return ret;
}

void MainWindow::filterCategory(int category_id)
{

	printf("packagelist size = %d\n", packagelist->size());
	waitUnlock();

	if (!actionBus.idle()) actionBus.abortActions();
	currentCategoryID = category_id;
	vector<bool> request;
	vector<string> tmpTagList;
	string tagvalue;
	request.resize(packagelist->size());
	for (int i=0; i<packagelist->size(); i++)
	{
		request[i]=isCategoryComplain(i, category_id);

	}
	if (actionBus._abortActions)
	{
		while (!actionBus._abortComplete)
		{
			usleep(10);
		}
	}
	emit requestPackages(request);
	applyPackageFilter();
}

void MainWindow::setStatus(QString status)
{
	ui.statusbar->showMessage(status);
}

void MainWindow::lockPackageList(bool state)
{
	__pkgLock=state;
}

void MainWindow::waitUnlock()
{
	while (__pkgLock)
	{
		usleep(1);
	}
}


void MainWindow::receivePackageList(PACKAGE_LIST pkgList, vector<int> nStatus)
{
	printf("%s: RECEIVED package list, size = %d\n", __func__, pkgList.size());
	*packagelist=pkgList;
	newStatus = nStatus;

}

void MainWindow::quickPackageSearch()
{
	waitUnlock();

	QString tmp;
	for (int i=0; i<ui.packageTable->rowCount(); i++)
	{
		tmp = tmp.fromStdString(*packagelist->getPackageByTableID(i)->get_name());
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

void MainWindow::resetQueue()
{
	for (unsigned int i=0; i<newStatus.size(); i++)
	{
		newStatus.at(i)=ST_NONE;
	}
	commitChanges();
}

void MainWindow::showPackageInfo()
{
	waitUnlock();

	long id = ui.packageTable->currentRow();
	PACKAGE *pkg = packagelist->getPackageByTableID(id);
	string info = "<html><h1>"+*pkg->get_name()+" "+*pkg->get_version()+"</h1><p><b>"+\
		       tr("Architecture:").toStdString()+"</b> "+*pkg->get_arch()+"<br><b>" + \
		       tr("Build:").toStdString()+"</b> "+*pkg->get_build();

	info += "<br><b>" + tr("Description:").toStdString()+" </b><br>"+*pkg->get_description()+"</p></html>";
	mstring taglist;
	for (unsigned int i=0; i< pkg->get_tags()->size(); i++)
	{
		taglist+="<br>";
		taglist+=pkg->get_tags()->at(i);
	}
	string extendedInfo = (string) "<html>" \
			       + (string) "<h2>" + *pkg->get_name() + (string) "</h2>" \
			       + (string) "<br><b>"+tr("Version:").toStdString()+" </b>" + *pkg->get_version() \
			       + (string) "<br><b>"+tr("Arch:").toStdString()+" </b>"+*pkg->get_arch() \
			       + (string) "<br><b>"+tr("Build:").toStdString()+" </b>"+*pkg->get_build() \
			       + (string) "<br><b>"+tr("Package size:").toStdString()+" </b>" + humanizeSize(*pkg->get_compressed_size()) \
			       + (string) "<br><b>"+tr("Installed size:").toStdString()+" </b>" + humanizeSize(*pkg->get_installed_size()) \
			       + (string) "<br><b>"+tr("Filename:").toStdString()+" </b>" + *pkg->get_filename() \
			       + (string) "<br><b>"+tr("MD5 sum:").toStdString()+" </b>"+*pkg->get_md5() \
			       + (string) "<br><b>"+tr("Maintainer:").toStdString()+" </b>"+*pkg->get_packager() \
			       + (string) " (" + *pkg->get_packager_email() + (string)")" \
			       + (string) "<br><b>"+tr("Status:").toStdString()+" </b>" + pkg->get_vstatus() \
			       + (string) "<br><br><b>"+tr("Tags:").toStdString()+"</b> " \
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

    QTableWidgetItem *__colItem1 = new QTableWidgetItem();
    __colItem1->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    ui.packageTable->setHorizontalHeaderItem(PT_NAME, __colItem1);

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
	if (!actionBus.idle())
	{
		if (QMessageBox::warning(this, \
					tr("Some actions doesn't completed"), \
					tr("Some actions hasn't completed yet. Abort and exit?"), \
					QMessageBox::Yes | QMessageBox::No, \
					QMessageBox::No) \
				== QMessageBox::Yes)
		{
			actionBus.abortActions();
			currentStatus = tr("Aborting actions...").toStdString();
		}
		else
		{
			return;
		}
	}
	else currentStatus = tr("Exiting...").toStdString();
	thread->callQuit();
	this->hide();
	qApp->quit();

}
void MainWindow::showCoreSettings()
{
	prefBox->openCore();
}
void MainWindow::commitChanges()
{
	waitUnlock();

	if (willBeOccupied - willBeFreed - get_disk_freespace() > 0)
	{
		if (QMessageBox::warning(this, \
					tr("Insufficient disk space"),
					tr("It seems that only ") + humanizeSize(get_disk_freespace()).c_str() +\
				       	tr(" is available on /, but we need ") + humanizeSize(willBeOccupied - willBeFreed).c_str() + \
					tr(", please free additional ") + humanizeSize(willBeOccupied - willBeFreed - get_disk_freespace()).c_str() + \
					tr(" to perform all requested operations. Are you sure you want to continue anyway?"), \
					QMessageBox::Yes | QMessageBox::No, QMessageBox::No)==QMessageBox::No)
		{
			return;
		}
	}
	QString installList;
	QString removeList;
	for (unsigned int i=0; i<newStatus.size(); i++)
	{
		if (newStatus[i]==ST_INSTALL)
		{
			installList+= "\n" + (QString) packagelist->get_package(i)->get_name()->c_str() + " " +\
				      (QString) packagelist->get_package(i)->get_fullversion().c_str();
		}
		if (newStatus[i]==ST_REMOVE || newStatus[i] == ST_PURGE || newStatus[i] == ST_UPDATE)
		{
			removeList+= "\n" + (QString) packagelist->get_package(i)->get_name()->c_str() + " " +\
				      (QString) packagelist->get_package(i)->get_fullversion().c_str();
		}
	}

	QMessageBox msgBox;
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	QString details;
	
	if (installList.length()>0) details += tr("Next packages will be installed:") + installList + "\n";
	if (removeList.length()>0) details += tr("Next packages will be removed:") + removeList + "\n";
	
	msgBox.setDetailedText(details);
	
	QString text = 	tr("Will be installed: ")+IntToStr(installQueueCount).c_str() + tr(" packages\n") + \
			tr("Will be removed: ") + IntToStr(removeQueueCount).c_str() + tr(" packages\n") + \
			tr("Will be updated: ") + IntToStr(updateQueueCount).c_str() + tr(" packages\n");
	
	if (willBeOccupied > willBeFreed)
	{
		text+= tr("Disk space will be occupied: ") + humanizeSize(willBeOccupied - willBeFreed).c_str();
	}
	else
	{
		text+=tr("Disk space will be freed: ") + humanizeSize(willBeFreed - willBeOccupied).c_str();
	}

	msgBox.setWindowTitle(tr("Action summary"));
	
	msgBox.setText(text);
	
	if (msgBox.exec() == QMessageBox::Cancel) return;

	emit commit(newStatus);
}
void MainWindow::resetChanges()
{
	emit loadData();
}
void MainWindow::showAddRemoveRepositories(){
	prefBox->openRepositories();
}

void MainWindow::receiveRequiredPackages(unsigned int package_num, PACKAGE_LIST req)
{
	waitUnlock();

	if (req.size()==1) return;
	int this_id=packagelist->get_package(package_num)->get_id();
	for (int i=0; i<packagelist->size(); i++)
	{
		for (int t=0; t<req.size(); t++)
		{
			if (packagelist->get_package(i)->get_id()!=this_id && packagelist->get_package(i)->get_id()==req.get_package(t)->get_id())
			{
				markChanges(i, Qt::Checked);
			}
		}		
	}
	generateStat();
}

void MainWindow::receiveDependantPackages(unsigned int package_num, PACKAGE_LIST dep)
{
	waitUnlock();

	if (dep.size()==1) return;
	int this_id=packagelist->get_package(package_num)->get_id();
	for (int i=0; i<packagelist->size(); i++)
	{
		for (int t=0; t<dep.size(); t++)
		{
			if (packagelist->get_package(i)->get_id()!=this_id && packagelist->get_package(i)->get_id()==dep.get_package(t)->get_id())
			{
				markChanges(i, Qt::Unchecked);
			}
		}		
	}
	generateStat();

}

void MainWindow::markChanges(int x, Qt::CheckState state, int force_state)
{
waitUnlock();

	generateStat();
	if (state == Qt::Checked)
	{
		emit getRequiredPackages(x);
	}
	else
	{
		emit getDependantPackages(x);
	}
	//unsigned long i = x;
	PACKAGE *_p = packagelist->getPackageByTableID(x);
	unsigned long i = packagelist->getRealNum(x);
	if (i >= newStatus.size())
	{
		mError ("overflow");
		return;
	}


	if (force_state>=0)
	{
		if (force_state == ST_NONE)
		{
			newStatus[i]==force_state;
		}
		if (!_p->installed() && !_p->installedVersion.empty() && force_state == ST_INSTALL)
		{
			for (int t=0; t<packagelist->size(); t++)
			{
				if (*packagelist->get_package(t)->get_name()==*_p->get_name())
				{
					markChanges(t, Qt::Unchecked, ST_UPDATE);
					break;
				}
			}
		}

		if (_p->installed() && force_state == ST_UPDATE)
		{
			newStatus[i]=force_state;
			currentStatus=tr("Package queued to update").toStdString();
			state = Qt::Unchecked;
		}

		if (_p->installed() && force_state == ST_REMOVE)
		{
			newStatus[i]=force_state;
			currentStatus=tr("Package queued to remove").toStdString();
			state = Qt::Unchecked;
		}
		if (_p->configexist() && force_state == ST_PURGE)
		{
			newStatus[i] = force_state;
			currentStatus = tr("Package queued to purge").toStdString();
			state = Qt::Unchecked;
		}
		if (!_p->installed() && force_state == ST_INSTALL)
		{
			newStatus[i] = force_state;
			currentStatus = tr("Package queued to install").toStdString();
			state = Qt::Checked;
		}
	}
	else
	{
		if (_p->installed())
		{

			switch(_p->action())
			{
				case ST_NONE:
					if (state == Qt::Checked)
					{
						newStatus[i]=ST_NONE;
						currentStatus=tr("Package keeped in system").toStdString();
					}
					else
					{
						newStatus[i]=ST_REMOVE;
						currentStatus=tr("Package queued to remove").toStdString();
					}
					break;
				case ST_UPDATE:
				case ST_REMOVE:
					if (state == Qt::Checked)
					{
						newStatus[i]=ST_NONE;
						currentStatus=tr("Package removed from remove queue").toStdString();
					}
					else
					{
						newStatus[i]=ST_REMOVE;
						currentStatus=tr("Package queued to remove").toStdString();
					}
					break;
				case ST_PURGE:
					if (state == Qt::Checked)
					{
						newStatus[i]=ST_NONE;
						currentStatus=tr("Package removed from purge queue").toStdString();
					}
					else
					{
						newStatus[i]=ST_PURGE;
						currentStatus=tr("Package queued to purge").toStdString();
					}
					break;
				default:
					currentStatus=tr("Unknown condition").toStdString();
			}
		} // if(_p->installed())
		else
		{
			switch(_p->action())
			{
				case ST_INSTALL:
				case ST_NONE:
					if (!_p->installed() && !_p->installedVersion.empty())
					{
						for (int t=0; t<packagelist->size(); t++)
						{
							if (*packagelist->get_package(t)->get_name() == *_p->get_name() && \
									packagelist->get_package(t)->installed() && \
									newStatus[t]==ST_NONE)
							{
								markChanges(t, Qt::Unchecked, ST_UPDATE);
								break;
							}
						}
					}


					if (state==Qt::Checked)
					{
						newStatus[i]=ST_INSTALL;
						currentStatus=tr("Package queued to install").toStdString();
					}
					else
					{
						newStatus[i]=ST_NONE;
						for (unsigned int t=0; t<newStatus.size(); t++)
						{
							if (newStatus[t]==ST_UPDATE && *packagelist->get_package(t)->get_name() == *_p->get_name())
							{
								newStatus[t]=ST_NONE;
								markChanges(t, Qt::Checked, ST_NONE);
								break;
							}
						}
							
						currentStatus=tr("Package unqueued").toStdString();

					}
					break;
				case ST_PURGE:
					if (state==Qt::Checked)
					{
						newStatus[i]=ST_NONE;
						currentStatus=tr("Package queued to install").toStdString();
					}
					else
					{
						newStatus[i]=ST_PURGE;
						currentStatus=tr("Package queued to purge").toStdString();
					}
					break;
				default:
					currentStatus=tr("Unknown condition").toStdString();
			}
		} // else
	} // else (force_state)
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
		case ST_UPDATE:
		case ST_REMOVE:
			package_icon="remove.png";
			break;
		case ST_PURGE:
			package_icon="purge.png";
			break;
	}
	string cloneHeader;
	if (_p->isUpdate()) cloneHeader = "<b><font color=\"red\">["+tr("update").toStdString()+"]</font></b>";
	if (_p->deprecated()) package_icon = (string) "deprecated_" + package_icon;

	string pName = "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" ><tbody><tr><td><img src = \"/usr/share/mpkg/icons/"+package_icon+"\"></img></td><td><b>"+*_p->get_name()+"</b> "\
		+_p->get_fullversion()\
		+" <font color=\"green\"> \t["+humanizeSize(*_p->get_compressed_size()) + "]     </font>" + cloneHeader+\
	       	+ "<br>"+*_p->get_short_description()+"</td></tr></tbody></table>";

	setTableItem(x, i, state, pName);		
	ui.applyButton->setEnabled(true);

}

void MainWindow::loadData()
{
	ui.statLabel->setText("");
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

bool MainWindow::nameComplain(int package_num, QString text)
{
waitUnlock();

	QString nameMask;	
	nameMask = nameMask.fromStdString(*packagelist->get_package(package_num)->get_name());
	return nameMask.contains(text, Qt::CaseInsensitive);
}
void MainWindow::highlightCategoryList()
{
	bool named, hlThis=false;
	QString itemIcon;
	QString itemText;
	QString textStyle = "<b>";
	QString _textStyle = "</b>";
	bool initial=false;
	itemIcon = "/usr/share/mpkg/icons/void.png";
	if (highlightState.size()!=availableTags.size())
	{
		initial=true;
		highlightState.clear();
		highlightState.resize(availableTags.size());
		for (unsigned int i=0; i<highlightState.size(); i++)
		{
			highlightState[i]=false;
		}
	}
	for (unsigned int i=0; i<availableTags.size(); i++)
	{
		if (!ui.quickPackageSearchEdit->text().isEmpty() && highlightMap[availableTags[i]])
		{
			if (!highlightState[i])
			{
				hlThis=true;
				highlightState[i]=true;
			}
			textStyle = "<b>";
			_textStyle = "</b>";
		}
		else
		{
			if (highlightState[i])
			{
				hlThis=true;
				highlightState[i]=false;
			}
			textStyle.clear();
			_textStyle.clear();
		}

		if (initial) printf("initial highlighting\n");
		named=false;
		if (initial || hlThis)
		{
			for (int t = 0; t< _categories.nChildNode("group");t++)
			{
				if (availableTags[i]==(string) _categories.getChildNode("group",t).getAttribute("tag"))
				{
					named=true;
					ListLabel *L_item = new ListLabel(ui.listWidget, i);
					itemText = "<table><tbody><tr><td><img src = \"" + itemIcon + "\"></img></td><td>" + \
						textStyle +\
					   	_categories.getChildNode("group",t).getAttribute("name") +\
						_textStyle + \
						"</td></tr></tbody></table>";

					L_item->setText(itemText);
					ui.listWidget->setItemWidget(ui.listWidget->item(i), L_item);
				}
			}
			if (!named)
			{
				ListLabel *L_item = new ListLabel(ui.listWidget, i);
				itemText = "<table><tbody><tr><td><img src = \"" + itemIcon + "\"></img></td><td>" + textStyle  +\
					   availableTags[i].c_str() + _textStyle + "</td></tr></tbody></table>";

				L_item->setText(itemText);
				ui.listWidget->setItemWidget(ui.listWidget->item(i), L_item);
			}
		}
	}

}

void CheckBox::markChanges()
{
	mw->markChanges(row, checkState());
}

CheckBox::CheckBox(MainWindow *parent)
{
	mw = parent;
}

