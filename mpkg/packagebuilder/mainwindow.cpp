/*******************************************************************
 * MOPSLinux packaging system
 * Package builder
 * $Id: mainwindow.cpp,v 1.3 2007/02/15 08:38:20 i27249 Exp $
 * ***************************************************************/


#include <QtGui>
#include "mainwindow.h"
#include <mpkg/libmpkg.h>
Form::Form(QWidget *parent)
{
	ui.setupUi(this);
}

void Form::loadData()
{
	XMLNode node;
	PACKAGE pkg;
	string tag_tmp;

	if (FileNotEmpty("install/data.xml"))
	{
		PackageConfig p("install/data.xml");
		xml2package(p.getXMLNode(), &pkg);
	

		// Filling data 
		ui.NameEdit->setText(pkg.get_name().c_str());
		ui.VersionEdit->setText(pkg.get_version().c_str());
		ui.ArchComboBox->setCurrentIndex(ui.ArchComboBox->findText(pkg.get_arch().c_str()));
		ui.BuildEdit->setText(pkg.get_build().c_str());
		ui.ShortDescriptionEdit->setText(pkg.get_short_description().c_str());
		ui.DescriptionEdit->setText(pkg.get_description().c_str());
		ui.ChangelogEdit->setText(pkg.get_changelog().c_str());
	

		//QTableWidgetItem tmp;
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
}

void Form::saveData()
{
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

	node.addChild("description");
	node.getChildNode("description").addText(ui.DescriptionEdit->toPlainText().toStdString().c_str());
	node.addChild("short_description");
	node.getChildNode("short_description").addText(ui.ShortDescriptionEdit->text().toStdString().c_str());
	node.addChild("dependencies");

	int dcurr=0;
	int scurr=0;
	for (int i=0; i<ui.DepTableWidget->rowCount(); i++)
	{
		if (ui.DepTableWidget->item(i,3)->text()== "Dependency")
		{
			node.getChildNode("dependencies").addChild("dep");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("name");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("name").addText(ui.DepTableWidget->item(i,0)->text().toStdString().c_str());
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("condition");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("condition").addText(ui.DepTableWidget->item(i,1)->text().toStdString().c_str());
			node.getChildNode("dependencies").getChildNode("dep", dcurr).addChild("version");
			node.getChildNode("dependencies").getChildNode("dep", dcurr).getChildNode("version").addText(ui.DepTableWidget->item(i,2)->text().toStdString().c_str());
			dcurr++;
		}
		if (ui.DepTableWidget->item(i,3)->text()=="Suggestion")
		{
			node.getChildNode("suggests").addChild("suggest");
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("name");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("name").addText(ui.DepTableWidget->item(i,0)->text().toStdString().c_str());
			node.getChildNode("suggests").getChildNode("suggest", scurr).addChild("condition");
			node.getChildNode("suggests").getChildNode("suggest", scurr).getChildNode("condition").addText(ui.DepTableWidget->item(i,1)->text().toStdString().c_str());
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
	node.addChild("maintainer");
	node.getChildNode("maintainer").addChild("name");
	node.getChildNode("maintainer").addChild("email");
	node.getChildNode("maintainer").getChildNode("name").addText(ui.MaintainerNameEdit->text().toStdString().c_str());
	node.getChildNode("maintainer").getChildNode("email").addText(ui.MaintainerMailEdit->text().toStdString().c_str());
	node.writeToFile("install/data.xml");
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
