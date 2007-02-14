#ifndef MV_H
#define MV_H
#include "ui_package_builder.h"

class Form: public QWidget
{
	Q_OBJECT
	public:
		Form (QWidget *parent = 0);
	public slots:
		void loadData();
		void saveData();
		void addTag();
		void addDependency();
		void deleteTag();
	public:
		Ui::Form ui;
};
#endif
