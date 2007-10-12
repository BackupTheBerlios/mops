/*******************************************************************
 * MOPSLinux packaging system
 * Package builder
 * $Id: mainwindow.cpp,v 1.29 2007/10/12 15:54:37 i27249 Exp $
 * ***************************************************************/

#include <QTextCodec>
#include <QtGui>
#include "mainwindow.h"
#include <QDir>
#include <mpkg/libmpkg.h>
#include <QFileDialog>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>

Form::Form(QWidget *parent, TargetType type, string arg)
{
	_type = type;
	_arg = arg;
	modified=false;
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	short_description.resize(2);
	description.resize(2);
	if (parent==0) ui.setupUi(this);
	else ui.setupUi(parent);
	ui.DepTableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	ui.DepTableWidget->horizontalHeader()->hide();
//	ui.DescriptionLanguageComboBox->hide();
	connect(ui.saveAndQuitButton, SIGNAL(clicked()), this, SLOT(saveAndExit()));
	connect(ui.addDepFromFilesBtn, SIGNAL(clicked()), this, SLOT(addDepsFromFiles()));
	xmlExists=true;
	this->show();
	loadData();
}



string cleanDescr(string str)
{
	string ret;
	if (str.find_first_of("\n")==std::string::npos) return str;
	while (str.find_first_of("\n")!=std::string::npos)
	{
		ret += str.substr(0,str.find_first_of("\n"));
		if (str.find_first_of("\n")<str.length()-1) str=str.substr(str.find_first_of("\n")+1);
		else {
			ret+=str;
			str.clear();
		}
	}
	ret+=str;
	return ret;
}
void Form::loadData()
{
	//XMLNode node;
	PACKAGE pkg;
	string tag_tmp;
	switch(_type)
	{
		case TYPE_NONE:
			if (FileExists("install/data.xml")) xmlFilename = "install/data.xml";
			else
			{
				xmlFilename = QFileDialog::getOpenFileName(this, tr("Choose package index") + " (data.xml):", "./", "");
			}
			break;
		case TYPE_XML:
			if (FileExists(_arg)) xmlFilename = _arg.c_str();
			else _type = TYPE_NONE;
			break;
		case TYPE_TGZ:
			if (FileExists(_arg))
			{
				// Extract package to somewhere and open the file
				// Create a temporary directory
				_tmpdir = get_tmp_file();
				say("Creating temp directory in %s\n", _tmpdir.c_str());
				unlink(_tmpdir.c_str());
				system("mkdir -p " + _tmpdir);
				say("Extracting\n");
				system("tar zxf " + _arg + " -C " + _tmpdir);
				if (FileExists(_tmpdir+"/install/data.xml")) 
				{
					xmlExists = true;
				}
				else {
					ui.ChangelogEdit->setText("Description converted from " + (QString) _arg.c_str());
					xmlExists = false;
				}	
				xmlFilename = _tmpdir.c_str() + (QString) "/install/data.xml";
			}
	}
//	XMLResults xmlErr;

//	xmlDocPtr doc; 
	//XMLNode *tmp = new XMLNode;
	if (!xmlFilename.isEmpty() && xmlExists)
	{
		//*tmp = XMLNode::parseFile(xmlFilename.toStdString().c_str(), "package", &xmlErr);
	//	doc = = xmlReadFile(xmlFilename.toStdString().c_str(),"UTF-8", NULL);

		PackageConfig p(xmlFilename.toStdString().c_str());
		if (p.parseOk)
		{
			//*tmp = p.getXMLNode();
			
			xml2package(p.getXMLNode(), &pkg);
			//delete tmp;
			pkg.sync();
	
			// Filling data 
			ui.NameEdit->setText(pkg.get_name()->c_str());
			ui.VersionEdit->setText(pkg.get_version()->c_str());
			ui.ArchComboBox->setCurrentIndex(ui.ArchComboBox->findText(pkg.get_arch()->c_str()));
			ui.BuildEdit->setText(pkg.get_build()->c_str());
			ui.ShortDescriptionEdit->setText(pkg.get_short_description()->c_str());
		
			//{
			short_description[0]=pkg.get_short_description()->c_str();
			description[0]=cleanDescr(*pkg.get_description()).c_str();
			//}
			ui.ShortDescriptionEdit->setText(short_description[0]);
			ui.DescriptionEdit->setText(description[0]);

			ui.ChangelogEdit->setText(pkg.get_changelog()->c_str());
			ui.MaintainerNameEdit->setText(pkg.get_packager()->c_str());
			ui.MaintainerMailEdit->setText(pkg.get_packager_email()->c_str());

			for (unsigned int i=0; i<pkg.get_dependencies()->size(); i++)
			{
				ui.DepTableWidget->insertRow(0);
				ui.DepTableWidget->setItem(0,3, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_type()->c_str()));
				ui.DepTableWidget->setItem(0,0, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_package_name()->c_str()));
				ui.DepTableWidget->setItem(0,1, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_vcondition().c_str()));
				ui.DepTableWidget->setItem(0,2, new QTableWidgetItem(pkg.get_dependencies()->at(i).get_package_version()->c_str()));
			}
		
			for (unsigned int i=0; i<pkg.get_tags()->size(); i++)
			{
				tag_tmp=pkg.get_tags()->at(i);
				ui.TagListWidget->addItem(tag_tmp.c_str());
				tag_tmp.clear();
			}
			for (unsigned int i=0; i<pkg.get_config_files()->size(); i++)
			{
				QListWidgetItem *__item = new QListWidgetItem(ui.configFilesListWidget);
				__item->setText(pkg.get_config_files()->at(i).get_name()->c_str());
			}
		}
		else
		{
			QMessageBox::warning(this, tr("Bad XML!"), \
				tr("Error opening ") + xmlFilename + tr(": invalid XML structure"), \
				QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	ui.DepTableWidget->resizeRowsToContents();
	ui.DepTableWidget->resizeColumnsToContents();
	modified=false;
	if (!xmlFilename.isEmpty() && xmlFilename.length()>strlen("install/data.xml"))
	{
		pkgRoot = xmlFilename.toStdString().substr(0, xmlFilename.length()-strlen("install/data.xml")).c_str();
	}
}

void Form::saveData()
{
	QString currentWindowTitle = windowTitle();
	setWindowTitle(windowTitle()+tr(": saving, please wait..."));
	modified=false;

	string slack_desc;
	string slack_required;
	string slack_suggests;
	string desc_chunk;
	QString xmlDir;
	if (xmlFilename.isEmpty())
	{
		xmlDir = QFileDialog::getExistingDirectory(this, tr("Choose directory where to save package index files")+" (data.xml, slack-desc, ...):", "."/*, "Package index (data.xml)"*/);
		xmlFilename = xmlDir+"/data.xml";
	}
	else
	{
		string namepart = xmlFilename.toStdString().substr(0,xmlFilename.length()-strlen("data.xml"));
		xmlDir = xmlDir.fromStdString(namepart);
	}

	if (xmlFilename.isEmpty())
	{
		return;
	}


	XMLNode node;
	node = XMLNode::createXMLTopNode("package");
	node.addChild("name");
	node.getChildNode("name").addText(ui.NameEdit->text().toStdString().c_str());
	node.addChild("version");
	node.getChildNode("version").addText(ui.VersionEdit->text().toStdString().c_str());
	node.addChild("arch");
	node.getChildNode("arch").addText(ui.ArchComboBox->currentText().toStdString().c_str());
	node.addChild("build");
	node.getChildNode("build").addText(ui.BuildEdit->text().toStdString().c_str());

	storeCurrentDescription();

	node.addChild("description");
	node.getChildNode("description",0).addAttribute("lang", "en");
	node.getChildNode("description",0).addText(description[0].toStdString().c_str());
	unsigned int spacePosition=0;
	slack_desc = ui.NameEdit->text().toStdString() + ": " + short_description[0].toStdString() + "\n" + ui.NameEdit->text().toStdString() + ": \n";
	for (unsigned int i=0; i<description[0].toStdString().length(); i++)
	{
		if (description[0].toStdString()[i]==' ') spacePosition = i;
		if (i >=70 || i == description[0].toStdString().length()-1)
		{
			desc_chunk = ui.NameEdit->text().toStdString() + ": " + description[0].toStdString().substr(0, spacePosition);
			if (i < description[0].toStdString().length()-1) i=0;
			if (spacePosition < description[0].toStdString().length()) description[0] = description[0].toStdString().substr(spacePosition).c_str();
			slack_desc+=desc_chunk + "\n";
		}
	}
	slack_desc = slack_desc.substr(0, slack_desc.length()-1) + description[0].toStdString();
	WriteFile(xmlDir.toStdString()+"/slack-desc", slack_desc);

	node.addChild("description");
	node.getChildNode("description",1).addAttribute("lang", "ru");
	node.getChildNode("description",1).addText(description[1].toStdString().c_str());

	node.addChild("short_description");
	node.getChildNode("short_description",0).addAttribute("lang", "en");
	node.getChildNode("short_description",0).addText(short_description[0].toStdString().c_str());
	node.addChild("short_description");
	node.getChildNode("short_description",1).addAttribute("lang", "ru");
	node.getChildNode("short_description",1).addText(short_description[1].toStdString().c_str());

	node.addChild("dependencies");

	node.addChild("suggests");
	int dcurr=0;
	int scurr=0;
	for (int i=0; i<ui.DepTableWidget->rowCount(); i++)
	{
		if (ui.DepTableWidget->item(i,3)->text().toUpper()== "DEPENDENCY")
		{
			node.getChildNode("dependencies").addChild("dep");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("name");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("name").addText(ui.DepTableWidget->item(i,0)->text().toStdString().c_str());
			slack_required+=ui.DepTableWidget->item(i,0)->text().toStdString();
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("condition");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("condition").addText(hcondition2xml(ui.DepTableWidget->item(i,1)->text().toStdString()).c_str());
			if (ui.DepTableWidget->item(i,1)->text().toStdString()!="any")
			{
				if (ui.DepTableWidget->item(i,1)->text().toStdString() == "==")
				{
					slack_required += "=";
				}
				else
				{
					slack_required+=ui.DepTableWidget->item(i,1)->text().toStdString();
				}
			}
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("version");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("version").addText(ui.DepTableWidget->item(i,2)->text().toStdString().c_str());
			if (ui.DepTableWidget->item(i,1)->text().toStdString()!="any")
			{
				slack_required += ui.DepTableWidget->item(i,2)->text().toStdString() + "\n";
			}
			dcurr++;
		}
		if (!slack_required.empty()) WriteFile(xmlDir.toStdString()+"/slack-required", slack_required);
		if (ui.DepTableWidget->item(i,3)->text().toUpper()=="SUGGESTION" ||
				ui.DepTableWidget->item(i,3)->text().toUpper()=="SUGGEST")
		{
			node.getChildNode("suggests").addChild("suggest");
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("name");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("name").addText(ui.DepTableWidget->item(i,0)->text().toStdString().c_str());
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("condition");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("condition").addText(hcondition2xml(ui.DepTableWidget->item(i,1)->text().toStdString()).c_str());
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("version");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("version").addText(ui.DepTableWidget->item(i,2)->text().toStdString().c_str());
			scurr++;
		}

	}
	node.addChild("tags");
	node.addChild("changelog");

	for (int i=0; i<ui.TagListWidget->count(); i++)
	{
		node.getChildNode("tags").addChild("tag");
		node.getChildNode("tags").getChildNode("tag",i).addText(ui.TagListWidget->item(i)->text().toStdString().c_str());
	}

	node.getChildNode("changelog").addText(ui.ChangelogEdit->toPlainText().toStdString().c_str());
	if (!ui.MaintainerNameEdit->text().isEmpty())
	{
		node.addChild("maintainer");
		node.getChildNode("maintainer").addChild("name");
		node.getChildNode("maintainer").getChildNode("name").addText(ui.MaintainerNameEdit->text().toStdString().c_str());
		if (!ui.MaintainerMailEdit->text().isEmpty())
		{
			node.getChildNode("maintainer").addChild("email");
			node.getChildNode("maintainer").getChildNode("email").addText(ui.MaintainerMailEdit->text().toStdString().c_str());
		}
	}
	node.addChild("configfiles");
	for (int i=0; i<ui.configFilesListWidget->count(); i++)
	{
		node.getChildNode("configfiles").addChild("conffile");
		node.getChildNode("configfiles").getChildNode("conffile",i).addText(ui.configFilesListWidget->item(i)->text().toStdString().c_str());
	}
	//(new QDir())->mkdir("install");
	node.writeToFile(xmlFilename.toStdString().c_str());
	setWindowTitle(currentWindowTitle+tr(" (saved)"));
	modified=false;
	if (_type==TYPE_TGZ)
	{
		string cdir = get_current_dir_name();
		// Pack the file back
		string arg_dir;
		if (_arg[0]!='/') 
		{

			arg_dir=cdir;
		}
		if (_arg.find_last_of("/")!=std::string::npos) 
		{
			arg_dir += _arg.substr(0,_arg.find_last_of("/"));
		}
		string cmd = "cd " + _tmpdir + "; buildpkg; rm " +arg_dir+"/"+ _arg+"; mv *.tgz " +arg_dir+"/";
		printf("cmd = [%s]\n",cmd.c_str());
		system(cmd);
	}
}

void Form::addTag(){
	if (!ui.tagBox->currentText().isEmpty())
	{
		ui.TagListWidget->addItem(ui.tagBox->currentText());
	}
}

void Form::saveAndExit()
{
	saveData();
	quitApp();
}
void Form::addDepsFromFiles()
{
	QStringList files;
	QFileDialog *dialog = new QFileDialog;
	dialog->setFileMode(QFileDialog::ExistingFiles);
	if (dialog->exec()) files = dialog->selectedFiles();
	PackageConfig *p;
	string tmp_xml = get_tmp_file();
	for (unsigned int i=0; i<files.size(); i++)
	{
		if (extractFromTgz(files.at(i).toStdString(), "install/data.xml", tmp_xml)==0)
		{
			p = new PackageConfig(tmp_xml);
			ui.DepNameEdit->setText(p->getName().c_str());
			ui.DepConditionComboBox->setCurrentIndex(1);
			ui.DepVersionEdit->setText(p->getVersion().c_str());
			addDependency();
			delete p;
			unlink(tmp_xml.c_str());
		}
	}
}

void Form::addDependency(){

	if (!ui.DepNameEdit->text().isEmpty())
	{
		if (ui.DepConditionComboBox->currentText()=="any" || !ui.DepVersionEdit->text().isEmpty())
		{
			ui.DepTableWidget->insertRow(0);
			ui.DepTableWidget->setItem(0,3, new QTableWidgetItem(ui.DepSuggestComboBox->currentText()));
			ui.DepTableWidget->setItem(0,0, new QTableWidgetItem(ui.DepNameEdit->text()));
			ui.DepTableWidget->setItem(0,1, new QTableWidgetItem(ui.DepConditionComboBox->currentText()));
			ui.DepTableWidget->setItem(0,2, new QTableWidgetItem(ui.DepVersionEdit->text()));
			ui.DepNameEdit->clear();
			ui.DepVersionEdit->clear();
			ui.DepSuggestComboBox->setCurrentIndex(0);
			ui.DepConditionComboBox->setCurrentIndex(0);
		}
	}
}
void Form::addConfigFile()
{
	if (!ui.confFileAddEdit->text().isEmpty())
	{
		QListWidgetItem *__item = new QListWidgetItem(ui.configFilesListWidget);
		__item->setText(ui.confFileAddEdit->text());
	}
}

void Form::deleteConfigFile()
{
	int i=ui.configFilesListWidget->currentRow();
	ui.configFilesListWidget->takeItem(i);
}
void Form::deleteTag()
{
	int i=ui.TagListWidget->currentRow();
	ui.TagListWidget->takeItem(i);
}
void Form::searchConfigFile()
{
	if (pkgRoot.isEmpty())
	{
		pkgRoot = QFileDialog::getExistingDirectory(this, tr("Choose package root"), "./");
	}
	if (!pkgRoot.isEmpty()) 
	{
		QString fname = QFileDialog::getOpenFileName(this, tr("Choose a config file"), pkgRoot, "");
		if (fname.length()>pkgRoot.length())
		ui.confFileAddEdit->setText(fname.toStdString().substr(pkgRoot.length()).c_str());
	}
}

void Form::deleteDependency()
{
	int i=ui.DepTableWidget->currentRow();
	ui.DepTableWidget->removeRow(i);
}
void Form::changeHeader()
{
	//printf("headerChange\n");
	modified=true;

	QString FLabel=tr("MOPSLinux package builder");

	if (!ui.NameEdit->text().isEmpty())
	{
		FLabel+=": "+ui.NameEdit->text();
		if (!ui.VersionEdit->text().isEmpty())
		{
			FLabel+="-"+ui.VersionEdit->text()+"-"+ui.ArchComboBox->currentText();
			if (!ui.BuildEdit->text().isEmpty())
			{
				FLabel+="-"+ui.BuildEdit->text();
			}
		}
	}
	setWindowTitle(FLabel);
}

void Form::changeHeader(const QString & text)
{
	/*if (!text.isEmpty())
	{
		setWindowTitle(text);
		return;
	}*/
	//printf("headerChange\n");
	modified=true;
	QString FLabel=tr("MOPSLinux package builder");

	if (!ui.NameEdit->text().isEmpty())
	{
		FLabel+=": "+ui.NameEdit->text();
		if (!ui.VersionEdit->text().isEmpty())
		{
			FLabel+="-"+ui.VersionEdit->text()+"-"+ui.ArchComboBox->currentText();
			if (!ui.BuildEdit->text().isEmpty())
			{
				FLabel+="-"+ui.BuildEdit->text();
			}
		}
	}
	setWindowTitle(FLabel);
}

void Form::swapLanguage()
{
	
	int i;
	int i2;
	/*if (ui.DescriptionLanguageComboBox->currentText()=="ru")
	{
		i=0;
		i2=1;
	}
	else 
	{*/
		i=1;
		i2=0;
	//}

	short_description[i]=ui.ShortDescriptionEdit->text();
	description[i]=ui.DescriptionEdit->toPlainText();
	ui.ShortDescriptionEdit->setText(short_description[i2]);
	ui.DescriptionEdit->setPlainText(description[i2]);
}

void Form::storeCurrentDescription()
{
	//printf("stored\n");
	int i;
	/*if (ui.DescriptionLanguageComboBox->currentText()=="ru")
	{
		i=1;
	}
	else 
	{*/
		i=0;
	//}

	short_description[i]=ui.ShortDescriptionEdit->text();
	description[i]=ui.DescriptionEdit->toPlainText();

}

void Form::quitApp()
{
	int ret;
	if (modified)
	{
		//printf("modified\n");
		ret = QMessageBox::warning(this, tr("MOPSLinux package builder"),
                   tr("The document has been modified.\n"
                      "Do you want to save your changes?"),
                   QMessageBox::Save | QMessageBox::Discard
                   | QMessageBox::Cancel,
                   QMessageBox::Save);
		//printf("ret = %d\n", ret);
		switch(ret)
		{
			case QMessageBox::Save: 
				saveData();
				if (_type==TYPE_TGZ) system("rm -rf " + _tmpdir);
				qApp->quit();
				break;
			case QMessageBox::Discard:
				if (_type==TYPE_TGZ) system("rm -rf " + _tmpdir);
				qApp->quit();
				break;
			case QMessageBox::Cancel:
				break;
		}
	}
	else qApp->quit();
}

