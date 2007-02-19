/*******************************************************************
 * MOPSLinux packaging system
 * Package builder
 * $Id: mainwindow.cpp,v 1.12 2007/02/19 04:58:09 i27249 Exp $
 * ***************************************************************/

#include <QTextCodec>
#include <QtGui>
#include "mainwindow.h"
#include <QDir>
#include <mpkg/libmpkg.h>
#include <QFileDialog>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>

Form::Form(QWidget *parent)
{
	modified=false;
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	short_description.resize(2);
	description.resize(2);
	ui.setupUi(this);
}

void Form::loadData()
{
	XMLNode node;
	PACKAGE pkg;
	string tag_tmp;
	xmlFilename = QFileDialog::getOpenFileName(this, "Choose package index (data.xml):", ".", "Package index (data.xml)");

	QXmlInputSource xmlsrc;
	xmlsrc.setData((QString) ReadFile(xmlFilename.toStdString()).c_str());

	QXmlSimpleReader *xmlReader = new QXmlSimpleReader;
	if (xmlReader->parse(xmlsrc))
	{
		PackageConfig p(xmlFilename.toStdString().c_str());
		xml2package(p.getXMLNode(), &pkg);
	

		// Filling data 
		ui.NameEdit->setText(pkg.get_name().c_str());
		ui.VersionEdit->setText(pkg.get_version().c_str());
		ui.ArchComboBox->setCurrentIndex(ui.ArchComboBox->findText(pkg.get_arch().c_str()));
		ui.BuildEdit->setText(pkg.get_build().c_str());
		ui.ShortDescriptionEdit->setText(pkg.get_short_description().c_str());
		
		for (int i=0; i<pkg.get_descriptions()->size(); i++)
		{
			printf("id=%d, lang = %s\n",i,pkg.get_descriptions()->get_description(i)->get_language().c_str());
			if (pkg.get_descriptions()->get_description(i)->get_language()=="en")
			{
				short_description[0]=pkg.get_descriptions()->get_description(i)->get_shorttext().c_str();
				description[0]=pkg.get_descriptions()->get_description(i)->get_text().c_str();
			}
			if (pkg.get_descriptions()->get_description(i)->get_language()=="ru")
			{
				short_description[1]=pkg.get_descriptions()->get_description(i)->get_shorttext().c_str();
				description[1]=pkg.get_descriptions()->get_description(i)->get_text().c_str();
			}
		}
		if (!short_description[1].isEmpty() || !description[1].isEmpty())
		{
			ui.ShortDescriptionEdit->setText(short_description[1]);
			ui.DescriptionEdit->setText(description[1]);
			printf("ru default\n");
		}
		else if (!short_description[0].isEmpty() || !description[0].isEmpty())
		{
			ui.DescriptionLanguageComboBox->setCurrentIndex(0);
			ui.ShortDescriptionEdit->setText(short_description[1]);
			ui.DescriptionEdit->setText(description[1]);
			printf("en default\n");
			
		}
		else
		{
			ui.DescriptionEdit->setText(pkg.get_description().c_str());
			printf("no default\n");
		}

		ui.ChangelogEdit->setText(pkg.get_changelog().c_str());
		ui.MaintainerNameEdit->setText(pkg.get_packager().c_str());
		ui.MaintainerMailEdit->setText(pkg.get_packager_email().c_str());
	

		for (int i=0; i<pkg.get_dependencies()->size(); i++)
		{
			ui.DepTableWidget->insertRow(0);
			ui.DepTableWidget->setItem(0,3, new QTableWidgetItem(pkg.get_dependencies()->get_dependency(i)->get_type().c_str()));
			ui.DepTableWidget->setItem(0,0, new QTableWidgetItem(pkg.get_dependencies()->get_dependency(i)->get_package_name().c_str()));
			ui.DepTableWidget->setItem(0,1, new QTableWidgetItem(pkg.get_dependencies()->get_dependency(i)->get_vcondition().c_str()));
			ui.DepTableWidget->setItem(0,2, new QTableWidgetItem(pkg.get_dependencies()->get_dependency(i)->get_package_version().c_str()));
		}

		for (int i=0; i<pkg.get_tags()->size(); i++)
		{
			tag_tmp=pkg.get_tags()->get_tag(i)->get_name();
			ui.TagListWidget->addItem(tag_tmp.c_str());
			tag_tmp.clear();
		}
	}
	ui.DepTableWidget->resizeRowsToContents();
	ui.DepTableWidget->resizeColumnsToContents();
	modified=false;
}

void Form::saveData()
{
	if (xmlFilename.isEmpty())
	{
		xmlFilename = QFileDialog::getSaveFileName(this, "Choose where to save package index (data.xml):", ".", "Package index (data.xml)");
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
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("condition");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("condition").addText(hcondition2xml(ui.DepTableWidget->item(i,1)->text().toStdString()).c_str());
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("version");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("version").addText(ui.DepTableWidget->item(i,2)->text().toStdString().c_str());
			dcurr++;
		}
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
	
	(new QDir())->mkdir("install");
	node.writeToFile(xmlFilename.toStdString().c_str());
	setWindowTitle(windowTitle()+" (saved)");
	modified=false;
}

void Form::addTag(){
	if (!ui.TagEdit->text().isEmpty()) {
	       ui.TagListWidget->addItem(ui.TagEdit->text());
	       ui.TagEdit->clear();
	}
}
void Form::addDependency(){

	if (!ui.DepNameEdit->text().isEmpty() && !ui.DepVersionEdit->text().isEmpty())
	{	ui.DepTableWidget->insertRow(0);
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

void Form::deleteTag()
{
	int i=ui.TagListWidget->currentRow();
	ui.TagListWidget->takeItem(i);
}

void Form::deleteDependency()
{
	int i=ui.DepTableWidget->currentRow();
	ui.DepTableWidget->removeRow(i);
}
void Form::changeHeader()
{
	printf("headerChange\n");
	modified=true;

	QString FLabel="MOPSLinux package builder";

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
	printf("headerChange\n");
	modified=true;
	QString FLabel="MOPSLinux package builder";

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
	if (ui.DescriptionLanguageComboBox->currentText()=="ru")
	{
		i=0;
		i2=1;
	}
	else 
	{
		i=1;
		i2=0;
	}

	short_description[i]=ui.ShortDescriptionEdit->text();
	description[i]=ui.DescriptionEdit->toPlainText();
	ui.ShortDescriptionEdit->setText(short_description[i2]);
	ui.DescriptionEdit->setPlainText(description[i2]);
}

void Form::storeCurrentDescription()
{
	printf("stored\n");
	int i;
	if (ui.DescriptionLanguageComboBox->currentText()=="ru")
	{
		i=1;
	}
	else 
	{
		i=0;
	}

	short_description[i]=ui.ShortDescriptionEdit->text();
	description[i]=ui.DescriptionEdit->toPlainText();

}

void Form::quitApp()
{
	int ret;
	if (modified)
	{
		printf("modified\n");
		ret = QMessageBox::warning(this, tr("MOPSLinux package builder"),
                   tr("The document has been modified.\n"
                      "Do you want to save your changes?"),
                   QMessageBox::Save | QMessageBox::Discard
                   | QMessageBox::Cancel,
                   QMessageBox::Save);
		printf("ret = %d\n", ret);
		switch(ret)
		{
			case QMessageBox::Save: saveData();
				qApp->quit();
				break;
			case QMessageBox::Discard:
				qApp->quit();
				break;
			case QMessageBox::Cancel:
				break;
		}
	}
	else qApp->quit();
}

