/***************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.cpp,v 1.15 2007/05/04 13:40:44 i27249 Exp $
 * ************************************************************************/

#include "preferencesbox.h"
#include <QFileDialog>
//#include "mainwindow.h"
PreferencesBox::PreferencesBox(mpkg *mDB)
{
	mDb=mDB;
	ui.setupUi(this);
	ui.addModifyRepositoryFrame->hide();
	QObject::connect(ui.urlEdit, SIGNAL(textEdited(const QString &)), this, SLOT(changeRepositoryType()));
	QObject::connect(ui.repTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeRepositoryType()));
	QObject::connect(ui.authCheckBox, SIGNAL(stateChanged(int)), this, SLOT(switchAuthMode()));
	QObject::connect(ui.applyRepChangesButton, SIGNAL(clicked()), this, SLOT(applyRepositoryChanges()));
	QObject::connect(ui.cancelRepChangesButton, SIGNAL(clicked()), this, SLOT(cancelRepositoryEdit()));
	QObject::connect(ui.applyButton, SIGNAL(clicked()), this, SLOT(applyConfig()));
	QObject::connect(ui.addRepositoryButton, SIGNAL(clicked()), this, SLOT(addRepositoryShow()));
	QObject::connect(ui.editRepositoryButton, SIGNAL(clicked()), this, SLOT(editRepositoryShow()));
	QObject::connect(ui.delRepositoryButton, SIGNAL(clicked()), this, SLOT(delRepository()));
	QObject::connect(ui.okButton, SIGNAL(clicked()), this, SLOT(okProcess()));
	QObject::connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelProcess()));	
	QObject::connect(ui.urlHelpSearch, SIGNAL(clicked()), this, SLOT(urlHelpSearchShow()));
	QObject::connect(ui.sysrootHelpBtn, SIGNAL(clicked()), this, SLOT(sysRootHelpShow()));
	QObject::connect(ui.syscacheHelpBtn, SIGNAL(clicked()), this, SLOT(sysCacheHelpShow()));
	QObject::connect(ui.dbfileHelpBtn, SIGNAL(clicked()), this, SLOT(sysDBHelpShow()));
	QObject::connect(ui.scriptsfolderHelpBtn, SIGNAL(clicked()), this, SLOT(sysScriptsHelpShow()));
	QObject::connect(ui.volNameDetectButton, SIGNAL(clicked()), this, SLOT(detectCdromVolnameCall()));
}

void PreferencesBox::delRepository()
{
	int remIndex = ui.repositoryTable->currentRow();
	if (remIndex>=0)
	{
		ui.repositoryTable->removeRow(remIndex);
		vector<bool> tmpRepStatus;
		for (unsigned int i=0; i<repStatus.size(); i++)
		{
			if (i == (unsigned int) remIndex) i++;
			tmpRepStatus.push_back(repStatus[i]);
		}
		repStatus = tmpRepStatus;
	}
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

void PreferencesBox::addRepositoryShow()
{
	editMode = REP_ADDMODE;

	ui.applyRepChangesButton->setText("Add");
	ui.addRepositoryButton->setEnabled(false);
	ui.delRepositoryButton->setEnabled(false);
	ui.editRepositoryButton->setEnabled(false);
	ui.repTypeComboBox->setCurrentIndex(0);
	changeRepositoryType();
	//ui.urlLabel->setText("URL:");
	ui.urlEdit->clear();
	ui.loginEdit->clear();
	ui.passwdEdit->clear();
	ui.volNameEdit->clear();
	ui.authCheckBox->setCheckState(Qt::Unchecked);
	//ui.loginLabel->hide();
	//ui.loginEdit->hide();
	//ui.passwdlabel->hide();
	//ui.passwdEdit->hide();
	//ui.urlHelpSearch->hide();
	ui.addModifyRepositoryFrame->show();
}

void PreferencesBox::switchAuthMode()
{
	if (ui.authCheckBox->checkState()==Qt::Unchecked)
	{
		ui.loginLabel->hide();
		ui.loginEdit->hide();
		ui.passwdlabel->hide();
		ui.passwdEdit->hide();
	}
	else
	{
		ui.loginLabel->show();
		ui.loginEdit->show();
		ui.passwdlabel->show();
		ui.passwdEdit->show();
	}
}


void PreferencesBox::changeRepositoryType()
{
	switch(ui.repTypeComboBox->currentIndex())
	{
		case 0:
			ui.urlLabel->setText("URL:");
			ui.urlHelpSearch->hide();
			ui.authCheckBox->show();
			ui.volNameEdit->hide();
			ui.volNameLabel->hide();
			ui.volNameDetectButton->hide();
			switchAuthMode();
			break;
		case 1:
			ui.urlLabel->setText("Path:");
			ui.urlHelpSearch->show();
			ui.authCheckBox->hide();
			ui.loginLabel->hide();
			ui.loginEdit->hide();
			ui.passwdlabel->hide();
			ui.passwdEdit->hide();
			ui.volNameEdit->hide();
			ui.volNameLabel->hide();
			ui.volNameDetectButton->hide();

			break;
		case 2:
			ui.urlLabel->setText("Relative path:");
			ui.urlHelpSearch->show();
			ui.authCheckBox->hide();
			ui.loginLabel->hide();
			ui.loginEdit->hide();
			ui.passwdlabel->hide();
			ui.passwdEdit->hide();
			ui.volNameLabel->show();
			ui.volNameEdit->show();
			ui.volNameDetectButton->show();
			break;
	}
}		

void PreferencesBox::detectCdromVolnameCall()
{
	//mpkgErrorReturn errRet;
	ui.volNameDetectButton->setEnabled(false);
	ui.volNameDetectButton->setText("Detecting...");
	emit getCdromName();
}

void PreferencesBox::recvCdromVolname(string volname)
{
	printf("recv volname = [%s]\n", volname.c_str());
	if (volname.empty())
	{
		printf("no volname\n");
		ui.volNameDetectButton->setText("Detect");
		ui.volNameDetectButton->setEnabled(true);
		return;
	}
	ui.volNameDetectButton->setText("Detect");
	ui.volNameDetectButton->setEnabled(true);
	ui.volNameEdit->setText(volname.c_str());
}
void PreferencesBox::urlHelpSearchShow()
{
	QString url = QFileDialog::getExistingDirectory(this, "Select a repository folder", ui.urlEdit->text());
	if (!url.isEmpty()) ui.urlEdit->setText(url);
}

void PreferencesBox::sysCacheHelpShow()
{
	string current = ui.syscacheEdit->text().toStdString();
	if (current[current.length()-1] == '/') current = current.substr(0,current.length()-1);
	QString sroot = QFileDialog::getExistingDirectory(this, "Select package cache folder", current.c_str());
	if (!sroot.isEmpty()) ui.syscacheEdit->setText(sroot);
}

void PreferencesBox::sysDBHelpShow()
{
	string current = ui.dbfileEdit->text().toStdString();
	current = current.substr(current.find("://")+3);

	QString sroot = QFileDialog::getOpenFileName(this, "Select a database file", current.c_str());
	if (!sroot.isEmpty()) ui.dbfileEdit->setText(sroot);
}

void PreferencesBox::sysScriptsHelpShow()
{
	string current = ui.scriptsfolderEdit->text().toStdString();
	if (current[current.length()-1] == '/') current = current.substr(0,current.length()-1);

	QString sroot = QFileDialog::getExistingDirectory(this, "Select script storage folder", current.c_str());
	if (!sroot.isEmpty()) ui.scriptsfolderEdit->setText(sroot);
}

void PreferencesBox::sysRootHelpShow()
{
	string current = ui.sysrootEdit->text().toStdString();
	if (current[current.length()-1] == '/') current = current.substr(0,current.length()-1);

	QString sroot = QFileDialog::getExistingDirectory(this, "Select system root folder", current.c_str());
	if (!sroot.isEmpty()) ui.sysrootEdit->setText(sroot);
}




void PreferencesBox::editRepositoryShow()
{
	ui.addModifyRepositoryFrame->show();
	ui.addRepositoryButton->setEnabled(false);
	ui.delRepositoryButton->setEnabled(false);
	ui.editRepositoryButton->setEnabled(false);

	string url = ui.repositoryTable->item(ui.repositoryTable->currentRow(), 1)->text().toStdString();
	editingRepository = ui.repositoryTable->currentRow();
	editMode = REP_EDITMODE;
	
	ui.applyRepChangesButton->setText("Apply");
	//string url_type;
	string login;
	string password;
	string urlbody;
	string volname;
	string url_type = url.substr(0,url.find("://")+3);
	if (url_type == "file://")
	{
		ui.repTypeComboBox->setCurrentIndex(1);
		changeRepositoryType();
		ui.urlEdit->setText(url.substr(url.find("://")+3).c_str());
	}
	else if (url_type == "cdrom://")
	{
		ui.repTypeComboBox->setCurrentIndex(2);
		changeRepositoryType();
		urlbody = url.substr(url.find("://")+3);
		volname = urlbody.substr(0, urlbody.find("/"));
		ui.urlEdit->setText(urlbody.substr(urlbody.find("/")-1).c_str());
		ui.volNameEdit->setText(volname.c_str());
	}
	else
	{
		ui.repTypeComboBox->setCurrentIndex(0);
		changeRepositoryType();
		if (url.find("@")!=std::string::npos)
		{
			ui.authCheckBox->setCheckState(Qt::Checked);
			switchAuthMode();
			//url_type = url.substr(0,url.find("://")+3);
			url = url.substr(url.find("://")+3);
			if (url.find(":")!=std::string::npos)
			{
				login=url.substr(0,url.find(":"));
				url = url.substr(url.find(":")+1);

				password = url.substr(0,url.find("@"));
				url = url.substr(url.find("@")+1);
			}
			else
			{
			       login = url.substr(0,url.find("@"));
			       url = url.substr(url.find("@")+1);
			}
			url = url_type + url;
			ui.loginEdit->setText(login.c_str());
			ui.passwdEdit->setText(password.c_str());
		}
		ui.urlEdit->setText(url.c_str());
	}
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
	printf("Loading core data\n");
	ui.sysrootEdit->setText(mDb->get_sysroot().c_str());
	ui.syscacheEdit->setText(mDb->get_syscache().c_str());
	ui.dbfileEdit->setText(mDb->get_dburl().c_str());
	ui.runscriptsCheckBox->setChecked(mDb->get_runscripts());
	ui.cdromDeviceEdit->setText(mDb->get_cdromdevice().c_str());
	ui.mountPointEdit->setText(mDb->get_cdrommountpoint().c_str());
	ui.volNameLabel->setTextFormat(Qt::RichText);
	ui.urlLabel->setTextFormat(Qt::RichText);
	printf("Loading checkfiles configuration\n");
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
	printf("Loading repository vectors\n");
	vector<string> rList;
        rList = mDb->get_repositorylist();
	vector<string> drList;
        drList = mDb->get_disabled_repositorylist();
	printf("Filling repository table\n");
	ui.repositoryTable->clearContents();
	ui.repositoryTable->setRowCount(rList.size()+drList.size());
	for (unsigned int i=0; i < rList.size(); i++)
	{
		RCheckBox *rCheckBox = new RCheckBox(this);
		rCheckBox->setCheckState(Qt::Checked);
		rCheckBox->setRow(i);
		ui.repositoryTable->setCellWidget(i,0,rCheckBox);
		ui.repositoryTable->setItem(i, 1, new QTableWidgetItem(rList[i].c_str()));
	}
	printf("Filling disabled repository table\n");
	for (unsigned int i=0; i < drList.size(); i++)
	{
		RCheckBox *rCheckBox = new RCheckBox(this);
		rCheckBox->setCheckState(Qt::Unchecked);
		rCheckBox->setRow(i+rList.size());
		ui.repositoryTable->setCellWidget(i+rList.size(),0,rCheckBox);
		ui.repositoryTable->setItem(i+rList.size(), 1, new QTableWidgetItem(drList[i].c_str()));
	}
	printf("Setting table sizes\n");
	ui.repositoryTable->setColumnWidth(0, 32);
 	ui.repositoryTable->setColumnWidth(1, 900);
	printf("Data loaded\n");
}

void PreferencesBox::applyRepositoryChanges()
{
	string urlString, tmp_url;
	unsigned int body_pos;

	switch(ui.repTypeComboBox->currentIndex())
	{
		case 0:
			// Checking validity of URL
			tmp_url = ui.urlEdit->text().toStdString();
			body_pos = tmp_url.find("://");
			if (body_pos == std::string::npos)
			{
				ui.urlLabel->setText("<html><font color = \"#FF0000\">URL:</color></html>");
				return;
			}
			else body_pos = body_pos + 3;
			if (ui.authCheckBox->checkState()==Qt::Checked && ui.loginEdit->text().length()>0)
			{
				urlString = tmp_url.substr(0, body_pos) + ui.loginEdit->text().toStdString();
				if (ui.passwdEdit->text().length()>0)
				{
					urlString += ":" + ui.passwdEdit->text().toStdString();
				}
				urlString += "@"+tmp_url.substr(body_pos);
			}
			else
				urlString = tmp_url;
			break;
		case 1:
			urlString = "file://";
			if (ui.urlEdit->text().length()==0)
			{
				ui.urlLabel->setText("<font color=\"#FF0000\">Path:</color>");
				return;
			}

			urlString += ui.urlEdit->text().toStdString();
			break;
		case 2:
			urlString = "cdrom://";
			if (ui.urlEdit->text().length()==0)
			{
				ui.urlLabel->setText("<font color=\"#FF0000\">Relative path:</color>");
				return;
			}

			if (ui.volNameEdit->text().length()==0)
			{
				ui.volNameLabel->setText("<font color=\"#FF0000\">Volume name:</color>");
				return;
			}
			urlString += ui.volNameEdit->text().toStdString() + "/" + ui.urlEdit->text().toStdString();
			break;
	}


	int insert_pos=ui.repositoryTable->rowCount();
	if (editMode==REP_ADDMODE)
	{
		RCheckBox *rCheckBox = new RCheckBox(this);
		rCheckBox->setCheckState(Qt::Checked);
		insert_pos = ui.repositoryTable->rowCount();
		ui.repositoryTable->setRowCount(insert_pos+1);
		rCheckBox->setRow(insert_pos);
		//repStatus.push_back(true);
		ui.repositoryTable->setCellWidget(rCheckBox->getRow(),0,rCheckBox);
	}
	if (editMode == REP_EDITMODE)
	{
		insert_pos = editingRepository;
	}
	ui.repositoryTable->setItem(insert_pos, 1, new QTableWidgetItem(urlString.c_str()));
	ui.addModifyRepositoryFrame->hide();
	ui.addRepositoryButton->setEnabled(true);
	ui.editRepositoryButton->setEnabled(true);
	ui.delRepositoryButton->setEnabled(true);

}

void PreferencesBox::cancelRepositoryEdit()
{
	ui.addRepositoryButton->setEnabled(true);
	ui.editRepositoryButton->setEnabled(true);
	ui.delRepositoryButton->setEnabled(true);
	ui.addModifyRepositoryFrame->hide();
}

void PreferencesBox::openCore()
{
	printf("loading data...\n");
	loadData();
	printf("Enabling window...\n");
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
	unsigned int fcheck=CHECKFILES_DISABLE;
	if (ui.fcheckInstallation->isChecked()) fcheck = CHECKFILES_PREINSTALL;
	if (ui.fcheckRemove->isChecked()) fcheck = CHECKFILES_POSTINSTALL;
	if (ui.fcheckDisable->isChecked()) fcheck = CHECKFILES_DISABLE;
	mDb->set_checkFiles(fcheck);
	vector<string>rList;
	vector<string>drList;
	for (int i=0; i < ui.repositoryTable->rowCount(); i++)
	{
		if (repStatus[i])
		{
			printf("enabled\n");
			rList.push_back(ui.repositoryTable->item(i, 1)->text().toStdString());
		}
		else
		{
			printf("disabled\n");
			drList.push_back(ui.repositoryTable->item(i,1)->text().toStdString());
		}
	}
	mDb->set_repositorylist(rList, drList);
	// TODO: Apply config
}

RCheckBox::RCheckBox(PreferencesBox *parent)
{
	mw = parent;
	if (mw->repStatus.size()<(unsigned int) row+1) mw->repStatus.resize(row+1);

	QObject::connect(this, SIGNAL(stateChanged(int)), this, SLOT(markChanges()));

}

void RCheckBox::markChanges()
{
	if (mw->repStatus.size()<(unsigned int) row+1) mw->repStatus.resize(row+1);
	if (checkState() == Qt::Checked) mw->repStatus[row] = true;
	else mw->repStatus[row] = false;
}
void RCheckBox::setRow(int rowNum)
{
	row = rowNum;
	if (mw->repStatus.size()<(unsigned int)row+1) mw->repStatus.resize(row+1);
	markChanges();
}
int RCheckBox::getRow()
{
	return row;
}
