/**************************************************************************
 * MOPSLinux packaging system - package manager - preferences
 * $Id: preferencesbox.h,v 1.8 2007/04/19 18:14:13 i27249 Exp $
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

class RCheckBox: public QCheckBox
{
	Q_OBJECT
	public:
		RCheckBox();
	public slots:
	//	void markChanges();
	public:
	int row;
	
};


#endif
