/***************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.cpp,v 1.12 2007/04/19 18:14:13 i27249 Exp $
 * ************************************************************************/

#include "preferencesbox.h"
//#include "mainwindow.h"
PreferencesBox::PreferencesBox(mpkg *mDb, QWidget *parent)
{

	ui.setupUi(this);
	ui.addModifyRepositoryFrame->hide();
	QObject::connect(ui.applyButton, SIGNAL(clicked()), this, SLOT(applyConfig()));
	QObject::connect(ui.addRepositoryButton, SIGNAL(clicked()), this, SLOT(addRepository()));
	QObject::connect(ui.editRepositoryButton, SIGNAL(clicked()), this, SLOT(editRepository()));
	//QObject::connect(ui.repositoryList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(editRepository()));
	QObject::connect(ui.delRepositoryButton, SIGNAL(clicked()), this, SLOT(delRepository()));
	QObject::connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okProcess()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelProcess()));	
}

void PreferencesBox::delRepository()
{
	ui.repositoryTable->removeRow(ui.repositoryTable->currentRow());
}

void PreferencesBox::okProcess()
{
	applyConfig();
	this->close();
}

void PreferencesBox::cancelProcess()
{
	this->close();
}

void PreferencesBox::addRepository()
{
	ui.addModifyRepositoryFrame->show();
	//ui.repositoryTable->addItem(ui.repositoryEdit->text());
	//ui.repositoryEdit->clear();
}

void PreferencesBox::editRepository()
{
	ui.addModifyRepositoryFrame->show();
	//ui.repositoryEdit->setText(ui.repositoryList->takeItem(ui.repositoryList->currentRow())->text());
	
}

void PreferencesBox::openAccounts()
{
	loadData();
	show();
}

void PreferencesBox::openRepositories()
{
	loadData();
	ui.tabWidget->setCurrentIndex(2);
	show();
}

void PreferencesBox::openInterface()
{
	loadData();
	ui.tabWidget->setCurrentIndex(0);
	show();
}

void PreferencesBox::loadData()
{
	// Load interface
	// Load core
	ui.sysrootEdit->setText(mDb->get_sysroot().c_str());
	ui.syscacheEdit->setText(mDb->get_syscache().c_str());
	ui.dbfileEdit->setText(mDb->get_dburl().c_str());
	ui.runscriptsCheckBox->setChecked(mDb->get_runscripts());
	ui.cdromDeviceEdit->setText(mDb->get_cdromdevice().c_str());
	ui.mountPointEdit->setText(mDb->get_cdrommountpoint().c_str());
	switch(mDb->get_checkFiles())
	{
		case CHECKFILES_PREINSTALL:
			ui.fcheckInstallation->setChecked(true);
			ui.fcheckRemove->setChecked(false);
			ui.fcheckDisable->setChecked(false);

			break;
		case CHECKFILES_POSTINSTALL:
			ui.fcheckInstallation->setChecked(false);
			ui.fcheckRemove->setChecked(true);
			ui.fcheckDisable->setChecked(false);

			break;
		case CHECKFILES_DISABLE:
			ui.fcheckDisable->setChecked(true);
			ui.fcheckInstallation->setChecked(false);
			ui.fcheckRemove->setChecked(false);

			break;
	}
	ui.scriptsfolderEdit->setText(mDb->get_scriptsdir().c_str());
	// Load accounts
	// Load repository
	vector<string> rList = mDb->get_repositorylist();
	vector<string> drList = mDb->get_disabled_repositorylist();

	ui.repositoryTable->clearContents();
	ui.repositoryTable->setRowCount(rList.size()+1+drList.size()+1);
	for (int i=0; i < rList.size(); i++)
	{
		RCheckBox *rCheckBox = new RCheckBox;
		rCheckBox->setCheckState(Qt::Checked);
		rCheckBox->row = i;
		ui.repositoryTable->setCellWidget(i,0,rCheckBox);
		ui.repositoryTable->setItem(i, 1, new QTableWidgetItem(rList[i].c_str()));
	}
	for (int i=0; i < drList.size(); i++)
	{
		RCheckBox *rCheckBox = new RCheckBox;
		rCheckBox->setCheckState(Qt::Unchecked);
		rCheckBox->row = i+rList.size();
		ui.repositoryTable->setCellWidget(i+rList.size(),0,rCheckBox);
		ui.repositoryTable->setItem(i+rList.size(), 1, new QTableWidgetItem(drList[i].c_str()));
	}
	ui.repositoryTable->setColumnWidth(0, 15);
 	ui.repositoryTable->setColumnWidth(1, 400);


	
	// Load updates
}

void PreferencesBox::openCore()
{
	loadData();
	ui.tabWidget->setCurrentIndex(1);
	show();
}

void PreferencesBox::openUpdates()
{
	loadData();
	ui.tabWidget->setCurrentIndex(4);
	show();
}

void PreferencesBox::applyConfig()
{
	mDb->set_sysroot(ui.sysrootEdit->text().toStdString());
	mDb->set_syscache(ui.syscacheEdit->text().toStdString());
	mDb->set_dburl(ui.dbfileEdit->text().toStdString());
	mDb->set_scriptsdir(ui.scriptsfolderEdit->text().toStdString());
	mDb->set_runscripts(ui.runscriptsCheckBox->checkState());
	mDb->set_cdromdevice(ui.cdromDeviceEdit->text().toStdString());
	mDb->set_cdrommountpoint(ui.mountPointEdit->text().toStdString());
	unsigned int fcheck;
	if (ui.fcheckInstallation->isChecked()) fcheck = CHECKFILES_PREINSTALL;
	if (ui.fcheckRemove->isChecked()) fcheck = CHECKFILES_POSTINSTALL;
	if (ui.fcheckDisable->isChecked()) fcheck = CHECKFILES_DISABLE;
	mDb->set_checkFiles(fcheck);
	vector<string>rList;
	for (int i=0; i < ui.repositoryTable->rowCount(); i++)
	{
		rList.push_back(ui.repositoryTable->item(i, 1)->text().toStdString());
	}
	mDb->set_repositorylist(rList);
	// TODO: Apply config
}

RCheckBox::RCheckBox()
{
}
