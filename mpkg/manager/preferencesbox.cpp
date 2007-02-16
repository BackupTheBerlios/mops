/***************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.cpp,v 1.2 2007/02/16 09:54:22 i27249 Exp $
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

