/***************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.cpp,v 1.3 2007/02/18 03:10:34 i27249 Exp $
 * ************************************************************************/

#include "preferencesbox.h"

PreferencesBox::PreferencesBox(QWidget *parent)
{
	ui.setupUi(this);
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

void PreferencesBox::loadData(mpkg *mDb)
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

void PreferencesBox::applyConfig(mpkg *mDb)
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