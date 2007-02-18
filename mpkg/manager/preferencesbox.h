/**************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.h,v 1.3 2007/02/18 03:10:34 i27249 Exp $
 * **********************************************************************/
#ifndef PREF_BOX_H
#define PREF_BOX_H

#include "ui_preferencesbox.h"
#include <mpkg/libmpkg.h>
class PreferencesBox: public QWidget
{
	Q_OBJECT
	public:
		PreferencesBox(QWidget *parent = 0);
		Ui::preferencesBox ui;
	public slots:
		void openAccounts();
		void openCore();
		void openUpdates();
		void openRepositories();
		void openInterface();
		void loadData(mpkg *mDb);
		void applyConfig(mpkg *mDb);
};
#endif
