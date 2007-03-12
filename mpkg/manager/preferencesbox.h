/**************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.h,v 1.7 2007/03/12 15:02:59 i27249 Exp $
 * **********************************************************************/
#ifndef PREF_BOX_H
#define PREF_BOX_H

#include "ui_preferencesbox.h"
#include <mpkg/libmpkg.h>
class PreferencesBox: public QWidget
{
	Q_OBJECT
	public:
		PreferencesBox(mpkg *mDb, QWidget *parent = 0);
		Ui::preferencesBox ui;
	public slots:
		void openAccounts();
		void openCore();
		void openUpdates();
		void openRepositories();
		void openInterface();
		void loadData();
		void applyConfig();
		
		void addRepository();
		void editRepository();
		void delRepository();
		void okProcess();
		void cancelProcess();
		
	private:
		mpkg *mDb;
		unsigned int editingRepository;
};
#endif
