/***************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.cpp,v 1.8 2007/03/12 15:02:59 i27249 Exp $
 * ************************************************************************/

#include "preferencesbox.h"

PreferencesBox::PreferencesBox(mpkg *mDb, QWidget *parent)
{

	ui.setupUi(this);
	QObject::connect(ui.applyButton, SIGNAL(clicked()), this, SLOT(applyConfig()));
	QObject::connect(ui.addRepositoryButton, SIGNAL(clicked()), this, SLOT(addRepository()));
	QObject::connect(ui.editRepositoryButton, SIGNAL(clicked()), this, SLOT(editRepository()));
	QObject::connect(ui.repositoryList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(editRepository()));
	QObject::connect(ui.delRepositoryButton, SIGNAL(clicked()), this, SLOT(delRepository()));
	QObject::connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okProcess()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelProcess()));	
}

void PreferencesBox::delRepository()
{
	ui.repositoryList->takeItem(ui.repositoryList->currentRow());
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
	ui.repositoryList->addItem(ui.repositoryEdit->text());
	ui.repositoryEdit->clear();
}

void PreferencesBox::editRepository()
{
	ui.repositoryEdit->setText(ui.repositoryList->takeItem(ui.repositoryList->currentRow())->text());
	
}

void PreferencesBox::openAccounts()
{
	show();
}

void PreferencesBox::openRepositories()
{
	show();
}

void PreferencesBox::openInterface()
{
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
	ui.scriptsfolderEdit->setText(mDb->get_scriptsdir().c_str());
	// Load accounts
	// Load repository
	vector<string> rList = mDb->get_repositorylist();
	for (int i=0; i < rList.size(); i++)
	{
		ui.repositoryList->addItem(rList[i].c_str());
	}
	
	// Load updates
}

void PreferencesBox::openCore()
{
	show();
}

void PreferencesBox::openUpdates()
{
	show();
}

void PreferencesBox::applyConfig()
{
	mDb->set_sysroot(ui.sysrootEdit->text().toStdString());
	mDb->set_syscache(ui.syscacheEdit->text().toStdString());
	mDb->set_dburl(ui.dbfileEdit->text().toStdString());
	mDb->set_scriptsdir(ui.scriptsfolderEdit->text().toStdString());
	mDb->set_runscripts(ui.runscriptsCheckBox->checkState());
	vector<string>rList;
	for (int i=0; i < ui.repositoryList->count(); i++)
	{
		rList.push_back(ui.repositoryList->item(i)->text().toStdString());
	}
	mDb->set_repositorylist(rList);
	// TODO: Apply config
}