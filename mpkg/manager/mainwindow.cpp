/*******************************************************************
 * MOPSLinux packaging system
 * Package manager - main code
 * $Id: mainwindow.cpp,v 1.1 2007/02/16 01:36:36 i27249 Exp $
 * ***************************************************************/

#include <QTextCodec>
#include <QtGui>
#include "mainwindow.h"
#include <QDir>
#include <mpkg/libmpkg.h>
#include <QFileDialog>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>

MainWindow::MainWindow(QMainWindow *parent)
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
	ui.setupUi(this);
}

